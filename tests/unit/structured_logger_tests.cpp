#include "tusho/logging/structured_logger.hpp"

#include "../support/test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
  try
  {
    const std::filesystem::path test_directory_path =
        ::tusho::tests::create_temporary_test_directory_path("structured-logger");

    const std::filesystem::path log_file_path = test_directory_path / "structured-log.jsonl";

    tusho::StructuredLogger structured_logger(log_file_path);
    structured_logger.log_information(
        "A message that contains a quote: \"example\" and special characters \\ \b \f \r \t " +
            std::string(1, static_cast<char>(1)),
        {{"path", "/tmp/example"}, {"note", "line-one\nline-two"}});
    structured_logger.log_warning("A warning message");
    structured_logger.log_error("An error message");

    std::ifstream log_file_stream(log_file_path);
    std::string first_log_entry_line;
    std::getline(log_file_stream, first_log_entry_line);
    std::string second_log_entry_line;
    std::getline(log_file_stream, second_log_entry_line);
    std::string third_log_entry_line;
    std::getline(log_file_stream, third_log_entry_line);

    TUSHO_EXPECT(!first_log_entry_line.empty(), "The structured logger should write at least one line to the file.");
    TUSHO_EXPECT(first_log_entry_line.find("\"severity\":\"information\"") != std::string::npos,
                 "The severity field should be present in the serialized log entry.");
    TUSHO_EXPECT(first_log_entry_line.find("\\\"example\\\"") != std::string::npos,
                 "Quote characters should be escaped so the output remains valid JSON.");
    TUSHO_EXPECT(first_log_entry_line.find("\\\\") != std::string::npos,
                 "Backslash characters should be escaped in JSON output.");
    TUSHO_EXPECT(first_log_entry_line.find("\\b") != std::string::npos,
                 "Backspace characters should be escaped in JSON output.");
    TUSHO_EXPECT(first_log_entry_line.find("\\f") != std::string::npos,
                 "Form-feed characters should be escaped in JSON output.");
    TUSHO_EXPECT(first_log_entry_line.find("\\r") != std::string::npos,
                 "Carriage-return characters should be escaped in JSON output.");
    TUSHO_EXPECT(first_log_entry_line.find("\\t") != std::string::npos,
                 "Tab characters should be escaped in JSON output.");
    TUSHO_EXPECT(first_log_entry_line.find("\\u0001") != std::string::npos,
                 "Other control characters should be rendered as unicode escapes.");
    TUSHO_EXPECT(second_log_entry_line.find("\"severity\":\"warning\"") != std::string::npos,
                 "Warning messages should be emitted with warning severity.");
    TUSHO_EXPECT(third_log_entry_line.find("\"severity\":\"error\"") != std::string::npos,
                 "Error messages should be emitted with error severity.");

    std::filesystem::remove_all(test_directory_path);
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Structured logger tests failed: " << exception.what() << '\n';
    return 1;
  }
}
