#ifndef TUSHO_DATABASE_FILE_SYSTEM_CATALOG_REPOSITORY_HPP
#define TUSHO_DATABASE_FILE_SYSTEM_CATALOG_REPOSITORY_HPP

#include "tusho/database/sqlite_database_connection.hpp"
#include "tusho/filesystem/file_system_entry_record.hpp"

#include <cstdint>
#include <string>

namespace tusho
{
/**
 * @brief Owns the database schema and all persistence logic for crawl results.
 */
class FileSystemCatalogRepository
{
public:
  explicit FileSystemCatalogRepository(SqliteDatabaseConnection &database_connection);
  ~FileSystemCatalogRepository();

  FileSystemCatalogRepository(const FileSystemCatalogRepository &) = delete;
  FileSystemCatalogRepository &operator=(const FileSystemCatalogRepository &) = delete;

  void initialize_schema();
  void clear_catalog();
  std::int64_t begin_scan_run(const std::string &crawl_root_directory_path_text);
  void record_file_system_entry(const FileSystemEntryRecord &file_system_entry_record);
  void finish_scan_run(
    std::int64_t scan_run_identifier,
    std::uint64_t accessible_entry_count,
    std::uint64_t inaccessible_entry_count);

private:
  SqliteDatabaseConnection &database_connection_;
  sqlite3_stmt *insert_or_update_file_system_entry_statement_ = nullptr;
};
} // namespace tusho

#endif
