#include "tusho/filesystem/file_system_crawler.hpp"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace tusho
{
namespace
{
struct FileStatusReadResult
{
  struct stat native_file_status
  {
  };
  bool read_succeeded = false;
};

FileStatusReadResult read_file_status(const std::filesystem::path& entry_path, const bool should_follow_symbolic_links)
{
  FileStatusReadResult file_status_read_result{};
  const std::string entry_path_text = entry_path.string();

  const int status_result = should_follow_symbolic_links
                                ? stat(entry_path_text.c_str(), &file_status_read_result.native_file_status)
                                : lstat(entry_path_text.c_str(), &file_status_read_result.native_file_status);

  file_status_read_result.read_succeeded = status_result == 0;
  return file_status_read_result;
}

std::string create_parent_directory_path_text(const std::filesystem::path& entry_path)
{
  const std::filesystem::path parent_directory_path = entry_path.parent_path();

  if (parent_directory_path.empty())
  {
    return "/";
  }

  return parent_directory_path.string();
}
} // namespace

FileSystemCrawler::FileSystemCrawler(FileSystemCatalogRepository& file_system_catalog_repository,
                                     StructuredLogger& structured_logger)
    : file_system_catalog_repository_(file_system_catalog_repository), structured_logger_(structured_logger)
{
}

ScanStatistics FileSystemCrawler::crawl(const RunConfiguration& run_configuration)
{
  // Creating the schema at the start of every run keeps first-run setup simple
  // and also ensures older database files are upgraded before use.
  file_system_catalog_repository_.initialize_schema();

  if (run_configuration.should_rebuild_database)
  {
    structured_logger_.log_information(
        "The catalog rebuild option is enabled, so existing rows will be removed before scanning.",
        {{"database_file_path", run_configuration.database_file_path.string()}});
    file_system_catalog_repository_.clear_catalog();
  }

  const std::filesystem::path normalized_crawl_root_directory_path =
      std::filesystem::absolute(run_configuration.crawl_root_directory_path);

  ScanStatistics scan_statistics{};
  scan_statistics.scan_run_identifier =
      file_system_catalog_repository_.begin_scan_run(normalized_crawl_root_directory_path.string());

  structured_logger_.log_information(
      "The crawler is starting a recursive scan.",
      {
          {"crawl_root_directory_path", normalized_crawl_root_directory_path.string()},
          {"database_file_path", run_configuration.database_file_path.string()},
          {"follow_symbolic_links", run_configuration.should_follow_symbolic_links ? "true" : "false"},
          {"include_hidden_entries", run_configuration.should_include_hidden_entries ? "true" : "false"},
      });

  try
  {
    // Recording the root directory separately guarantees that the catalog always
    // contains the exact starting point of the crawl.
    file_system_catalog_repository_.record_file_system_entry(
        create_file_system_entry_record(scan_statistics.scan_run_identifier, normalized_crawl_root_directory_path,
                                        run_configuration.should_follow_symbolic_links));
    ++scan_statistics.accessible_entry_count;
  }
  catch (const std::exception& exception)
  {
    ++scan_statistics.inaccessible_entry_count;
    structured_logger_.log_warning("The crawler could not read the root directory metadata.",
                                   {
                                       {"crawl_root_directory_path", normalized_crawl_root_directory_path.string()},
                                       {"reason", exception.what()},
                                   });
  }

  std::error_code iterator_construction_error_code;
  const auto iterator_options = run_configuration.should_follow_symbolic_links
                                    ? std::filesystem::directory_options::follow_directory_symlink |
                                          std::filesystem::directory_options::skip_permission_denied
                                    : std::filesystem::directory_options::skip_permission_denied;

  std::filesystem::recursive_directory_iterator current_entry_iterator(
      normalized_crawl_root_directory_path, iterator_options, iterator_construction_error_code);

  if (iterator_construction_error_code)
  {
    throw std::runtime_error("Unable to begin crawling the requested root directory: " +
                             iterator_construction_error_code.message());
  }

  const std::filesystem::recursive_directory_iterator end_iterator;

  while (current_entry_iterator != end_iterator)
  {
    const std::filesystem::directory_entry& current_directory_entry = *current_entry_iterator;
    const std::filesystem::path current_entry_path = current_directory_entry.path();

    if (!run_configuration.should_include_hidden_entries && entry_name_is_hidden(current_entry_path))
    {
      if (current_directory_entry.is_directory())
      {
        // Hidden directories are skipped recursively so the crawler does not
        // descend into a subtree that the user explicitly asked to ignore.
        current_entry_iterator.disable_recursion_pending();
      }
    }
    else
    {
      try
      {
        // Each successfully read entry becomes one database row. Any failure is
        // logged and counted, but it does not stop the rest of the scan.
        file_system_catalog_repository_.record_file_system_entry(create_file_system_entry_record(
            scan_statistics.scan_run_identifier, current_entry_path, run_configuration.should_follow_symbolic_links));
        ++scan_statistics.accessible_entry_count;
      }
      catch (const std::exception& exception)
      {
        ++scan_statistics.inaccessible_entry_count;
        structured_logger_.log_warning("The crawler discovered an entry but could not fully read it.",
                                       {
                                           {"absolute_path", current_entry_path.string()},
                                           {"reason", exception.what()},
                                       });
      }
    }

    std::error_code iterator_increment_error_code;
    current_entry_iterator.increment(iterator_increment_error_code);

    if (iterator_increment_error_code)
    {
      ++scan_statistics.inaccessible_entry_count;
      structured_logger_.log_warning("The crawler could not advance to the next entry cleanly.",
                                     {
                                         {"reason", iterator_increment_error_code.message()},
                                     });
    }
  }

  file_system_catalog_repository_.finish_scan_run(scan_statistics.scan_run_identifier,
                                                  scan_statistics.accessible_entry_count,
                                                  scan_statistics.inaccessible_entry_count);

  structured_logger_.log_information(
      "The crawler completed the recursive scan.",
      {
          {"scan_run_identifier", std::to_string(scan_statistics.scan_run_identifier)},
          {"accessible_entry_count", std::to_string(scan_statistics.accessible_entry_count)},
          {"inaccessible_entry_count", std::to_string(scan_statistics.inaccessible_entry_count)},
      });

  return scan_statistics;
}

bool FileSystemCrawler::entry_name_is_hidden(const std::filesystem::path& entry_path)
{
  const std::string entry_name = entry_path.filename().string();
  return !entry_name.empty() && entry_name.front() == '.';
}

std::string FileSystemCrawler::create_entry_type_text(const mode_t status_mode_bits)
{
  if (S_ISREG(status_mode_bits))
  {
    return "file";
  }

  if (S_ISDIR(status_mode_bits))
  {
    return "directory";
  }

  if (S_ISLNK(status_mode_bits))
  {
    return "symbolic_link";
  }

  if (S_ISCHR(status_mode_bits))
  {
    return "character_device";
  }

  if (S_ISBLK(status_mode_bits))
  {
    return "block_device";
  }

  if (S_ISFIFO(status_mode_bits))
  {
    return "named_pipe";
  }

  if (S_ISSOCK(status_mode_bits))
  {
    return "socket";
  }

  return "unknown";
}

std::string FileSystemCrawler::create_permissions_octal_text(const mode_t status_mode_bits)
{
  std::ostringstream permissions_text_stream;
  permissions_text_stream << '0' << std::oct << std::setw(3) << std::setfill('0') << (status_mode_bits & 0777U);
  return permissions_text_stream.str();
}

std::string FileSystemCrawler::create_utc_timestamp_text(const std::int64_t seconds_since_epoch)
{
  const auto timestamp_value = static_cast<std::time_t>(seconds_since_epoch);
  std::tm utc_time_components{};
  gmtime_r(&timestamp_value, &utc_time_components);

  std::ostringstream timestamp_text_stream;
  timestamp_text_stream << std::put_time(&utc_time_components, "%Y-%m-%dT%H:%M:%SZ");
  return timestamp_text_stream.str();
}

std::string FileSystemCrawler::create_symbolic_link_target_path_text(const std::filesystem::path& entry_path)
{
  std::vector<char> target_path_buffer(4096, '\0');
  const ssize_t bytes_written = readlink(entry_path.c_str(), target_path_buffer.data(), target_path_buffer.size() - 1);

  if (bytes_written <= 0)
  {
    return "";
  }

  return {target_path_buffer.data(), static_cast<std::size_t>(bytes_written)};
}

FileSystemEntryRecord FileSystemCrawler::create_file_system_entry_record(const std::int64_t scan_run_identifier,
                                                                         const std::filesystem::path& entry_path,
                                                                         const bool should_follow_symbolic_links)
{
  // Linux exposes most of the file metadata we care about through stat and
  // lstat. We choose between them based on whether symbolic links should be
  // followed for this run.
  const FileStatusReadResult file_status_read_result = read_file_status(entry_path, should_follow_symbolic_links);

  if (!file_status_read_result.read_succeeded)
  {
    const int file_status_error_number = errno;
    throw std::runtime_error("Unable to read file status: " +
                             std::generic_category().message(file_status_error_number));
  }

  const struct stat& native_file_status = file_status_read_result.native_file_status;

  FileSystemEntryRecord file_system_entry_record{};
  // The record is intentionally flattened into plain strings and integers so it
  // can be stored in SQLite and consumed directly by the PHP application.
  file_system_entry_record.scan_run_identifier = scan_run_identifier;
  file_system_entry_record.absolute_path = std::filesystem::absolute(entry_path).string();
  file_system_entry_record.parent_directory_path = create_parent_directory_path_text(entry_path);
  file_system_entry_record.entry_name =
      entry_path.filename().empty() ? entry_path.string() : entry_path.filename().string();
  file_system_entry_record.entry_type = create_entry_type_text(native_file_status.st_mode);
  file_system_entry_record.symbolic_link_target_path =
      S_ISLNK(native_file_status.st_mode) ? create_symbolic_link_target_path_text(entry_path) : "";
  file_system_entry_record.permissions_octal_text = create_permissions_octal_text(native_file_status.st_mode);
  file_system_entry_record.last_write_time_utc_text = create_utc_timestamp_text(native_file_status.st_mtim.tv_sec);
  file_system_entry_record.last_status_change_time_utc_text =
      create_utc_timestamp_text(native_file_status.st_ctim.tv_sec);
  file_system_entry_record.file_size_bytes = static_cast<std::int64_t>(native_file_status.st_size);
  file_system_entry_record.owner_user_identifier = static_cast<std::int64_t>(native_file_status.st_uid);
  file_system_entry_record.owner_group_identifier = static_cast<std::int64_t>(native_file_status.st_gid);
  file_system_entry_record.inode_number = static_cast<std::int64_t>(native_file_status.st_ino);
  file_system_entry_record.device_number = static_cast<std::int64_t>(native_file_status.st_dev);
  file_system_entry_record.hard_link_count = static_cast<std::int64_t>(native_file_status.st_nlink);
  file_system_entry_record.was_accessible = true;

  return file_system_entry_record;
}
} // namespace tusho
