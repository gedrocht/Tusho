#ifndef TUSHO_FILESYSTEM_FILE_SYSTEM_ENTRY_RECORD_HPP
#define TUSHO_FILESYSTEM_FILE_SYSTEM_ENTRY_RECORD_HPP

#include <cstdint>
#include <string>

namespace tusho
{
/**
 * @brief Represents one row in the file catalog database.
 *
 * Every field is stored in a beginner-friendly and database-friendly form so
 * that the PHP application can query the data directly with little translation.
 */
struct FileSystemEntryRecord
{
  std::int64_t scan_run_identifier = 0;
  std::string absolute_path;
  std::string parent_directory_path;
  std::string entry_name;
  std::string entry_type;
  std::string symbolic_link_target_path;
  std::string permissions_octal_text;
  std::string last_write_time_utc_text;
  std::string last_status_change_time_utc_text;
  std::int64_t file_size_bytes = 0;
  std::int64_t owner_user_identifier = -1;
  std::int64_t owner_group_identifier = -1;
  std::int64_t inode_number = -1;
  std::int64_t device_number = -1;
  std::int64_t hard_link_count = -1;
  bool was_accessible = false;
};
} // namespace tusho

#endif
