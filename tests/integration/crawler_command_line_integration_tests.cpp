#include "../support/test_support.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

#ifndef TUSHO_CRAWLER_EXECUTABLE_PATH
#define TUSHO_CRAWLER_EXECUTABLE_PATH "tusho_crawler"
#endif

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
        ::tusho::tests::create_temporary_test_directory_path("crawler-command-line");
    const std::filesystem::path crawl_root_directory_path = test_directory_path / "scan-root";
    const std::filesystem::path database_file_path = test_directory_path / "catalog.sqlite3";
    const std::filesystem::path log_file_path = test_directory_path / "crawler.jsonl";

    std::filesystem::create_directories(crawl_root_directory_path);

    {
      std::ofstream sample_file_stream(crawl_root_directory_path / "sample-file.txt");
      sample_file_stream << "sample-data";
    }

    {
      std::ofstream hidden_file_stream(crawl_root_directory_path / ".hidden-sample-file.txt");
      hidden_file_stream << "hidden-sample-data";
    }

    std::error_code symbolic_link_error_code;
    std::filesystem::create_symlink(crawl_root_directory_path / "sample-file.txt",
                                    crawl_root_directory_path / "sample-file-link.txt", symbolic_link_error_code);

    const std::string command_line_text =
        "\"" + std::string(TUSHO_CRAWLER_EXECUTABLE_PATH) + "\"" + " --database-path \"" + database_file_path.string() +
        "\"" + " --crawl-root \"" + crawl_root_directory_path.string() + "\"" + " --log-file-path \"" +
        log_file_path.string() + "\"" + " --rebuild-database --include-hidden-entries --follow-symbolic-links";

    // NOLINTNEXTLINE(cert-env33-c,concurrency-mt-unsafe)
    const int command_exit_code = std::system(command_line_text.c_str());

    TUSHO_EXPECT(command_exit_code == 0,
                 "The crawler executable should complete successfully when invoked through the command line.");
    TUSHO_EXPECT(std::filesystem::exists(database_file_path),
                 "The command-line crawler run should create the database file.");
    TUSHO_EXPECT(std::filesystem::exists(log_file_path), "The command-line crawler run should create the log file.");

    sqlite3* native_database_handle = nullptr;
    const int open_result = sqlite3_open(database_file_path.string().c_str(), &native_database_handle);

    if (open_result != SQLITE_OK || native_database_handle == nullptr)
    {
      throw std::runtime_error("The command-line integration test could not open the generated SQLite database.");
    }

    const std::int64_t entry_count =
        fetch_count_from_query(native_database_handle, "SELECT COUNT(*) FROM file_system_entries;");
    const std::int64_t hidden_entry_count = fetch_count_from_query(
        native_database_handle,
        "SELECT COUNT(*) FROM file_system_entries WHERE entry_name = '.hidden-sample-file.txt';");

    sqlite3_close(native_database_handle);

    TUSHO_EXPECT(entry_count >= 2, "The command-line crawler run should index the crawl root and the sample file.");
    TUSHO_EXPECT(hidden_entry_count == 1,
                 "The command-line crawler should index hidden entries when the matching flag is enabled.");

    if (!symbolic_link_error_code)
    {
      TUSHO_EXPECT(entry_count >= 3, "A created symbolic link should increase the catalog row count.");
    }

    std::filesystem::remove_all(test_directory_path);
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Command-line integration test failed: " << exception.what() << '\n';
    return 1;
  }
}
