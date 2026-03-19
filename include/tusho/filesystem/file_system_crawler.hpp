#ifndef TUSHO_FILESYSTEM_FILE_SYSTEM_CRAWLER_HPP
#define TUSHO_FILESYSTEM_FILE_SYSTEM_CRAWLER_HPP

#include "tusho/application/run_configuration.hpp"
#include "tusho/database/file_system_catalog_repository.hpp"
#include "tusho/filesystem/file_system_entry_record.hpp"
#include "tusho/filesystem/scan_statistics.hpp"
#include "tusho/logging/structured_logger.hpp"

#include <filesystem>
#include <sys/stat.h>

namespace tusho
{
/**
 * @brief Walks a Linux file system tree and stores discovered entries.
 */
class FileSystemCrawler
{
public:
  FileSystemCrawler(FileSystemCatalogRepository &file_system_catalog_repository, StructuredLogger &structured_logger);

  ScanStatistics crawl(const RunConfiguration &run_configuration);

private:
  FileSystemCatalogRepository &file_system_catalog_repository_;
  StructuredLogger &structured_logger_;

  static bool entry_name_is_hidden(const std::filesystem::path &entry_path);
  static std::string create_entry_type_text(mode_t status_mode_bits);
  static std::string create_permissions_octal_text(mode_t status_mode_bits);
  static std::string create_utc_timestamp_text(std::int64_t seconds_since_epoch);
  static std::string create_symbolic_link_target_path_text(const std::filesystem::path &entry_path);

  FileSystemEntryRecord create_file_system_entry_record(
    std::int64_t scan_run_identifier,
    const std::filesystem::path &entry_path,
    bool should_follow_symbolic_links) const;
};
} // namespace tusho

#endif
