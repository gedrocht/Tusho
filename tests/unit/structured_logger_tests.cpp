#include "tusho/logging/structured_logger.hpp"

#include "../support/test_support.hpp"

#include <filesystem>
#include <fstream>
#include <string>

int main()
{
  const std::filesystem::path test_directory_path =
    ::tusho::tests::create_temporary_test_directory_path("structured-logger");

  const std::filesystem::path log_file_path = test_directory_path / "structured-log.jsonl";

  tusho::StructuredLogger structured_logger(log_file_path);
  structured_logger.log_information(
    "A message that contains a quote: \"example\"",
    {{"path", "/tmp/example"}, {"note", "line-one\nline-two"}});

  std::ifstream log_file_stream(log_file_path);
  std::string first_log_entry_line;
  std::getline(log_file_stream, first_log_entry_line);

  TUSHO_EXPECT(!first_log_entry_line.empty(), "The structured logger should write at least one line to the file.");
  TUSHO_EXPECT(
    first_log_entry_line.find("\"severity\":\"information\"") != std::string::npos,
    "The severity field should be present in the serialized log entry.");
  TUSHO_EXPECT(
    first_log_entry_line.find("\\\"example\\\"") != std::string::npos,
    "Quote characters should be escaped so the output remains valid JSON.");

  std::filesystem::remove_all(test_directory_path);
  return 0;
}
