#include "tusho/application/run_configuration.hpp"
#include "tusho/database/file_system_catalog_repository.hpp"
#include "tusho/database/sqlite_database_connection.hpp"
#include "tusho/filesystem/file_system_crawler.hpp"
#include "tusho/logging/structured_logger.hpp"

#include "../support/test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

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
} // namespace

int main()
{
  try
  {
    const std::filesystem::path test_directory_path =
        ::tusho::tests::create_temporary_test_directory_path("file-system-crawler");

    std::filesystem::create_directories(test_directory_path / "nested-directory");

    {
      std::ofstream visible_file_stream(test_directory_path / "visible-file.txt");
      visible_file_stream << "example data";
    }

    {
      std::ofstream hidden_file_stream(test_directory_path / ".hidden-file.txt");
      hidden_file_stream << "secret";
    }

    const std::filesystem::path symbolic_link_path = test_directory_path / "sample-symbolic-link.txt";
    std::error_code symbolic_link_error_code;
    std::filesystem::create_symlink(test_directory_path / "visible-file.txt", symbolic_link_path,
                                    symbolic_link_error_code);

    const std::filesystem::path database_file_path = test_directory_path / "catalog.sqlite3";
    const std::filesystem::path log_file_path = test_directory_path / "crawler.jsonl";

    tusho::StructuredLogger structured_logger(log_file_path);
    tusho::SqliteDatabaseConnection sqlite_database_connection(database_file_path);
    tusho::FileSystemCatalogRepository file_system_catalog_repository(sqlite_database_connection);
    tusho::FileSystemCrawler file_system_crawler(file_system_catalog_repository, structured_logger);

    tusho::RunConfiguration run_configuration{};
    run_configuration.database_file_path = database_file_path;
    run_configuration.crawl_root_directory_path = test_directory_path;
    run_configuration.log_file_path = log_file_path;
    run_configuration.should_rebuild_database = true;
    run_configuration.should_include_hidden_entries = false;

    const tusho::ScanStatistics scan_statistics = file_system_crawler.crawl(run_configuration);

    TUSHO_EXPECT(scan_statistics.accessible_entry_count >= 3,
                 "The crawler should index the root, a directory, and a file.");
    TUSHO_EXPECT(scan_statistics.inaccessible_entry_count == 0,
                 "The controlled integration test should not hit permission errors.");

    const std::int64_t indexed_entry_count =
        fetch_count_from_query(sqlite_database_connection.native_handle(), "SELECT COUNT(*) FROM file_system_entries;");

    const std::int64_t hidden_entry_count =
        fetch_count_from_query(sqlite_database_connection.native_handle(),
                               "SELECT COUNT(*) FROM file_system_entries WHERE entry_name = '.hidden-file.txt';");

    TUSHO_EXPECT(indexed_entry_count >= 3, "The database should contain multiple indexed entries.");
    TUSHO_EXPECT(hidden_entry_count == 0, "Hidden entries should be omitted when the option is disabled.");

    run_configuration.should_include_hidden_entries = true;
    run_configuration.should_rebuild_database = false;

    const tusho::ScanStatistics second_scan_statistics = file_system_crawler.crawl(run_configuration);

    TUSHO_EXPECT(second_scan_statistics.accessible_entry_count >= scan_statistics.accessible_entry_count,
                 "Including hidden entries should not reduce the number of indexed entries.");

    const std::int64_t included_hidden_entry_count =
        fetch_count_from_query(sqlite_database_connection.native_handle(),
                               "SELECT COUNT(*) FROM file_system_entries WHERE entry_name = '.hidden-file.txt';");

    TUSHO_EXPECT(included_hidden_entry_count == 1,
                 "Hidden entries should be indexed when the include-hidden option is enabled.");

    if (!symbolic_link_error_code)
    {
      const std::int64_t symbolic_link_entry_count = fetch_count_from_query(
          sqlite_database_connection.native_handle(),
          "SELECT COUNT(*) FROM file_system_entries WHERE entry_name = 'sample-symbolic-link.txt';");

      TUSHO_EXPECT(symbolic_link_entry_count == 1,
                   "A successfully created symbolic link should also be recorded in the catalog.");
    }

    std::filesystem::remove_all(test_directory_path);
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Crawler integration test failed: " << exception.what() << '\n';
    return 1;
  }
}
