#include "tusho/database/file_system_catalog_repository.hpp"

#include <stdexcept>

namespace tusho
{
namespace
{
void bind_text(sqlite3_stmt* prepared_statement, const int parameter_index, const std::string& parameter_value)
{
  const int bind_result =
      sqlite3_bind_text(prepared_statement, parameter_index, parameter_value.c_str(), -1, SQLITE_TRANSIENT);

  if (bind_result != SQLITE_OK)
  {
    throw std::runtime_error("Unable to bind SQLite text parameter.");
  }
}

void bind_integer(sqlite3_stmt* prepared_statement, const int parameter_index, const std::int64_t parameter_value)
{
  const int bind_result = sqlite3_bind_int64(prepared_statement, parameter_index, parameter_value);

  if (bind_result != SQLITE_OK)
  {
    throw std::runtime_error("Unable to bind SQLite integer parameter.");
  }
}
} // namespace

FileSystemCatalogRepository::FileSystemCatalogRepository(SqliteDatabaseConnection& database_connection)
    : database_connection_(database_connection)
{
}

FileSystemCatalogRepository::~FileSystemCatalogRepository()
{
  if (insert_or_update_file_system_entry_statement_ != nullptr)
  {
    sqlite3_finalize(insert_or_update_file_system_entry_statement_);
    insert_or_update_file_system_entry_statement_ = nullptr;
  }
}

void FileSystemCatalogRepository::initialize_schema()
{
  // The schema is kept deliberately simple: one table describes scan runs and
  // one table describes the discovered file system entries.
  database_connection_.execute_sql_statement("CREATE TABLE IF NOT EXISTS scan_runs ("
                                             "  scan_run_identifier INTEGER PRIMARY KEY AUTOINCREMENT,"
                                             "  crawl_root_directory_path TEXT NOT NULL,"
                                             "  started_at_utc TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                                             "  finished_at_utc TEXT,"
                                             "  accessible_entry_count INTEGER NOT NULL DEFAULT 0,"
                                             "  inaccessible_entry_count INTEGER NOT NULL DEFAULT 0"
                                             ");");

  database_connection_.execute_sql_statement(
      "CREATE TABLE IF NOT EXISTS file_system_entries ("
      "  absolute_path TEXT PRIMARY KEY,"
      "  scan_run_identifier INTEGER NOT NULL,"
      "  parent_directory_path TEXT NOT NULL,"
      "  entry_name TEXT NOT NULL,"
      "  entry_type TEXT NOT NULL,"
      "  symbolic_link_target_path TEXT NOT NULL,"
      "  permissions_octal_text TEXT NOT NULL,"
      "  last_write_time_utc_text TEXT NOT NULL,"
      "  last_status_change_time_utc_text TEXT NOT NULL,"
      "  file_size_bytes INTEGER NOT NULL,"
      "  owner_user_identifier INTEGER NOT NULL,"
      "  owner_group_identifier INTEGER NOT NULL,"
      "  inode_number INTEGER NOT NULL,"
      "  device_number INTEGER NOT NULL,"
      "  hard_link_count INTEGER NOT NULL,"
      "  was_accessible INTEGER NOT NULL,"
      "  FOREIGN KEY (scan_run_identifier) REFERENCES scan_runs(scan_run_identifier) ON DELETE CASCADE"
      ");");

  database_connection_.execute_sql_statement(
      "CREATE INDEX IF NOT EXISTS index_file_system_entries_by_parent_directory_path "
      "ON file_system_entries(parent_directory_path);");

  database_connection_.execute_sql_statement("CREATE INDEX IF NOT EXISTS index_file_system_entries_by_entry_name "
                                             "ON file_system_entries(entry_name);");

  database_connection_.execute_sql_statement("CREATE INDEX IF NOT EXISTS index_file_system_entries_by_entry_type "
                                             "ON file_system_entries(entry_type);");

  database_connection_.execute_sql_statement(
      "CREATE INDEX IF NOT EXISTS index_file_system_entries_by_scan_run_identifier "
      "ON file_system_entries(scan_run_identifier);");

  if (insert_or_update_file_system_entry_statement_ == nullptr)
  {
    // Reusing a prepared statement avoids reparsing the same SQL for every file
    // system entry during a large crawl.
    insert_or_update_file_system_entry_statement_ = database_connection_.prepare_statement(
        "INSERT INTO file_system_entries ("
        "  absolute_path,"
        "  scan_run_identifier,"
        "  parent_directory_path,"
        "  entry_name,"
        "  entry_type,"
        "  symbolic_link_target_path,"
        "  permissions_octal_text,"
        "  last_write_time_utc_text,"
        "  last_status_change_time_utc_text,"
        "  file_size_bytes,"
        "  owner_user_identifier,"
        "  owner_group_identifier,"
        "  inode_number,"
        "  device_number,"
        "  hard_link_count,"
        "  was_accessible"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        "ON CONFLICT(absolute_path) DO UPDATE SET "
        "  scan_run_identifier = excluded.scan_run_identifier,"
        "  parent_directory_path = excluded.parent_directory_path,"
        "  entry_name = excluded.entry_name,"
        "  entry_type = excluded.entry_type,"
        "  symbolic_link_target_path = excluded.symbolic_link_target_path,"
        "  permissions_octal_text = excluded.permissions_octal_text,"
        "  last_write_time_utc_text = excluded.last_write_time_utc_text,"
        "  last_status_change_time_utc_text = excluded.last_status_change_time_utc_text,"
        "  file_size_bytes = excluded.file_size_bytes,"
        "  owner_user_identifier = excluded.owner_user_identifier,"
        "  owner_group_identifier = excluded.owner_group_identifier,"
        "  inode_number = excluded.inode_number,"
        "  device_number = excluded.device_number,"
        "  hard_link_count = excluded.hard_link_count,"
        "  was_accessible = excluded.was_accessible;");
  }
}

void FileSystemCatalogRepository::clear_catalog() const
{
  database_connection_.execute_sql_statement("DELETE FROM file_system_entries;");
  database_connection_.execute_sql_statement("DELETE FROM scan_runs;");
}

std::int64_t FileSystemCatalogRepository::begin_scan_run(const std::string& crawl_root_directory_path_text) const
{
  sqlite3_stmt* prepared_statement =
      database_connection_.prepare_statement("INSERT INTO scan_runs (crawl_root_directory_path) VALUES (?);");

  bind_text(prepared_statement, 1, crawl_root_directory_path_text);

  const int execution_result = sqlite3_step(prepared_statement);
  sqlite3_finalize(prepared_statement);

  if (execution_result != SQLITE_DONE)
  {
    throw std::runtime_error("Unable to create the scan run row.");
  }

  return database_connection_.fetch_last_inserted_row_identifier();
}

void FileSystemCatalogRepository::record_file_system_entry(const FileSystemEntryRecord& file_system_entry_record)
{
  // Resetting and rebinding lets one prepared statement be safely reused for
  // every inserted or updated row.
  sqlite3_reset(insert_or_update_file_system_entry_statement_);
  sqlite3_clear_bindings(insert_or_update_file_system_entry_statement_);

  bind_text(insert_or_update_file_system_entry_statement_, 1, file_system_entry_record.absolute_path);
  bind_integer(insert_or_update_file_system_entry_statement_, 2, file_system_entry_record.scan_run_identifier);
  bind_text(insert_or_update_file_system_entry_statement_, 3, file_system_entry_record.parent_directory_path);
  bind_text(insert_or_update_file_system_entry_statement_, 4, file_system_entry_record.entry_name);
  bind_text(insert_or_update_file_system_entry_statement_, 5, file_system_entry_record.entry_type);
  bind_text(insert_or_update_file_system_entry_statement_, 6, file_system_entry_record.symbolic_link_target_path);
  bind_text(insert_or_update_file_system_entry_statement_, 7, file_system_entry_record.permissions_octal_text);
  bind_text(insert_or_update_file_system_entry_statement_, 8, file_system_entry_record.last_write_time_utc_text);
  bind_text(insert_or_update_file_system_entry_statement_, 9,
            file_system_entry_record.last_status_change_time_utc_text);
  bind_integer(insert_or_update_file_system_entry_statement_, 10, file_system_entry_record.file_size_bytes);
  bind_integer(insert_or_update_file_system_entry_statement_, 11, file_system_entry_record.owner_user_identifier);
  bind_integer(insert_or_update_file_system_entry_statement_, 12, file_system_entry_record.owner_group_identifier);
  bind_integer(insert_or_update_file_system_entry_statement_, 13, file_system_entry_record.inode_number);
  bind_integer(insert_or_update_file_system_entry_statement_, 14, file_system_entry_record.device_number);
  bind_integer(insert_or_update_file_system_entry_statement_, 15, file_system_entry_record.hard_link_count);
  bind_integer(insert_or_update_file_system_entry_statement_, 16, file_system_entry_record.was_accessible ? 1 : 0);

  const int execution_result = sqlite3_step(insert_or_update_file_system_entry_statement_);

  if (execution_result != SQLITE_DONE)
  {
    throw std::runtime_error("Unable to insert or update a file system entry.");
  }
}

void FileSystemCatalogRepository::finish_scan_run(const std::int64_t scan_run_identifier,
                                                  const std::uint64_t accessible_entry_count,
                                                  const std::uint64_t inaccessible_entry_count) const
{
  sqlite3_stmt* prepared_statement = database_connection_.prepare_statement("UPDATE scan_runs SET "
                                                                            "  finished_at_utc = CURRENT_TIMESTAMP,"
                                                                            "  accessible_entry_count = ?,"
                                                                            "  inaccessible_entry_count = ? "
                                                                            "WHERE scan_run_identifier = ?;");

  bind_integer(prepared_statement, 1, static_cast<std::int64_t>(accessible_entry_count));
  bind_integer(prepared_statement, 2, static_cast<std::int64_t>(inaccessible_entry_count));
  bind_integer(prepared_statement, 3, scan_run_identifier);

  const int execution_result = sqlite3_step(prepared_statement);
  sqlite3_finalize(prepared_statement);

  if (execution_result != SQLITE_DONE)
  {
    throw std::runtime_error("Unable to update the scan run summary.");
  }
}
} // namespace tusho
