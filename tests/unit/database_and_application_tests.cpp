#include "tusho/application/crawler_application.hpp"
#include "tusho/application/run_configuration.hpp"
#include "tusho/database/file_system_catalog_repository.hpp"
#include "tusho/database/sqlite_database_connection.hpp"
#include "tusho/filesystem/file_system_entry_record.hpp"

#include "../support/test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
std::int64_t fetch_count_from_query(sqlite3* native_database_handle, const std::string& sql_statement_text)
{
  sqlite3_stmt* prepared_statement = nullptr;
  const int prepare_result =
      sqlite3_prepare_v2(native_database_handle, sql_statement_text.c_str(), -1, &prepared_statement, nullptr);

  if (prepare_result != SQLITE_OK)
  {
    throw std::runtime_error("Unable to prepare a verification query.");
  }

  const int step_result = sqlite3_step(prepared_statement);

  if (step_result != SQLITE_ROW)
  {
    sqlite3_finalize(prepared_statement);
    throw std::runtime_error("A verification query did not return a row.");
  }

  const std::int64_t query_result = sqlite3_column_int64(prepared_statement, 0);
  sqlite3_finalize(prepared_statement);
  return query_result;
}

void verify_sqlite_database_connection_supports_move_operations()
{
  const std::filesystem::path test_directory_path =
      ::tusho::tests::create_temporary_test_directory_path("sqlite-database-connection");
  const std::filesystem::path database_file_path = test_directory_path / "connection.sqlite3";

  tusho::SqliteDatabaseConnection original_database_connection(database_file_path);
  original_database_connection.execute_sql_statement("CREATE TABLE sample_rows (identifier INTEGER PRIMARY KEY);");

  tusho::SqliteDatabaseConnection move_constructed_database_connection(std::move(original_database_connection));
  TUSHO_EXPECT(move_constructed_database_connection.native_handle() != nullptr,
               "Move construction should transfer the native database handle.");

  tusho::SqliteDatabaseConnection move_assigned_database_connection(database_file_path);
  move_assigned_database_connection = std::move(move_constructed_database_connection);

  TUSHO_EXPECT(move_assigned_database_connection.native_handle() != nullptr,
               "Move assignment should transfer the native database handle.");

  move_assigned_database_connection.execute_sql_statement("INSERT INTO sample_rows DEFAULT VALUES;");

  const std::int64_t inserted_row_count =
      fetch_count_from_query(move_assigned_database_connection.native_handle(), "SELECT COUNT(*) FROM sample_rows;");

  TUSHO_EXPECT(inserted_row_count == 1, "The moved database connection should remain usable.");

  std::filesystem::remove_all(test_directory_path);
}

void verify_repository_can_clear_catalog_rows()
{
  const std::filesystem::path test_directory_path =
      ::tusho::tests::create_temporary_test_directory_path("catalog-repository");
  const std::filesystem::path database_file_path = test_directory_path / "catalog.sqlite3";

  tusho::SqliteDatabaseConnection sqlite_database_connection(database_file_path);
  tusho::FileSystemCatalogRepository file_system_catalog_repository(sqlite_database_connection);

  file_system_catalog_repository.initialize_schema();
  const std::int64_t scan_run_identifier = file_system_catalog_repository.begin_scan_run("/tmp/example");

  tusho::FileSystemEntryRecord file_system_entry_record{};
  file_system_entry_record.scan_run_identifier = scan_run_identifier;
  file_system_entry_record.absolute_path = "/tmp/example/sample.txt";
  file_system_entry_record.parent_directory_path = "/tmp/example";
  file_system_entry_record.entry_name = "sample.txt";
  file_system_entry_record.entry_type = "file";
  file_system_entry_record.permissions_octal_text = "0644";
  file_system_entry_record.last_write_time_utc_text = "2026-01-01T00:00:00Z";
  file_system_entry_record.last_status_change_time_utc_text = "2026-01-01T00:00:00Z";
  file_system_entry_record.was_accessible = true;

  file_system_catalog_repository.record_file_system_entry(file_system_entry_record);
  file_system_catalog_repository.finish_scan_run(scan_run_identifier, 1, 0);
  file_system_catalog_repository.clear_catalog();

  const std::int64_t entry_count =
      fetch_count_from_query(sqlite_database_connection.native_handle(), "SELECT COUNT(*) FROM file_system_entries;");
  const std::int64_t scan_run_count =
      fetch_count_from_query(sqlite_database_connection.native_handle(), "SELECT COUNT(*) FROM scan_runs;");

  TUSHO_EXPECT(entry_count == 0, "Clearing the catalog should remove file system entries.");
  TUSHO_EXPECT(scan_run_count == 0, "Clearing the catalog should remove scan runs.");

  std::filesystem::remove_all(test_directory_path);
}

void verify_crawler_application_can_run_end_to_end()
{
  const std::filesystem::path test_directory_path =
      ::tusho::tests::create_temporary_test_directory_path("crawler-application");
  const std::filesystem::path crawl_root_directory_path = test_directory_path / "scan-root";
  const std::filesystem::path database_file_path = test_directory_path / "catalog.sqlite3";
  const std::filesystem::path log_file_path = test_directory_path / "application.jsonl";

  std::filesystem::create_directories(crawl_root_directory_path);

  {
    std::ofstream sample_file_stream(crawl_root_directory_path / "sample.txt");
    sample_file_stream << "sample-data";
  }

  tusho::RunConfiguration run_configuration{};
  run_configuration.database_file_path = database_file_path;
  run_configuration.crawl_root_directory_path = crawl_root_directory_path;
  run_configuration.log_file_path = log_file_path;
  run_configuration.should_rebuild_database = true;

  const int exit_code = tusho::CrawlerApplication::run(run_configuration);

  TUSHO_EXPECT(exit_code == 0, "A successful crawler application run should return zero.");
  TUSHO_EXPECT(std::filesystem::exists(database_file_path),
               "The crawler application should create the SQLite database.");
  TUSHO_EXPECT(std::filesystem::exists(log_file_path),
               "The crawler application should create the structured log file.");

  const tusho::SqliteDatabaseConnection sqlite_database_connection(database_file_path);
  const std::int64_t entry_count =
      fetch_count_from_query(sqlite_database_connection.native_handle(), "SELECT COUNT(*) FROM file_system_entries;");

  TUSHO_EXPECT(entry_count >= 2, "The application should catalog the root directory and the sample file.");

  std::filesystem::remove_all(test_directory_path);
}

void verify_invalid_sql_statements_raise_exceptions()
{
  const std::filesystem::path test_directory_path =
      ::tusho::tests::create_temporary_test_directory_path("sqlite-error-paths");
  const std::filesystem::path database_file_path = test_directory_path / "errors.sqlite3";

  const tusho::SqliteDatabaseConnection sqlite_database_connection(database_file_path);

  bool execute_sql_statement_threw = false;

  try
  {
    sqlite_database_connection.execute_sql_statement("THIS IS NOT VALID SQL;");
  }
  catch (const std::exception&)
  {
    execute_sql_statement_threw = true;
  }

  bool prepare_statement_threw = false;

  try
  {
    sqlite_database_connection.prepare_statement("SELECT * FROM");
  }
  catch (const std::exception&)
  {
    prepare_statement_threw = true;
  }

  TUSHO_EXPECT(execute_sql_statement_threw, "Invalid SQL execution should raise an exception.");
  TUSHO_EXPECT(prepare_statement_threw, "Preparing malformed SQL should raise an exception.");

  std::filesystem::remove_all(test_directory_path);
}
} // namespace

int main()
{
  try
  {
    verify_sqlite_database_connection_supports_move_operations();
    verify_repository_can_clear_catalog_rows();
    verify_crawler_application_can_run_end_to_end();
    verify_invalid_sql_statements_raise_exceptions();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Database and application tests failed: " << exception.what() << '\n';
    return 1;
  }
}
