#ifndef TUSHO_LOGGING_STRUCTURED_LOGGER_HPP
#define TUSHO_LOGGING_STRUCTURED_LOGGER_HPP

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace tusho
{
/**
 * @brief Writes structured JSON log entries to both standard output and a file.
 *
 * The logger avoids external dependencies so that the project remains easy to
 * build on a minimal Linux machine. The output is newline-delimited JSON, which
 * works well with command-line tools, systemd journals, and many existing log
 * aggregation products.
 */
class StructuredLogger
{
public:
  explicit StructuredLogger(const std::filesystem::path &log_file_path);

  void log_information(
    const std::string &message_text,
    const std::vector<std::pair<std::string, std::string>> &named_fields = {});

  void log_warning(
    const std::string &message_text,
    const std::vector<std::pair<std::string, std::string>> &named_fields = {});

  void log_error(
    const std::string &message_text,
    const std::vector<std::pair<std::string, std::string>> &named_fields = {});

private:
  std::ofstream log_file_stream_;
  std::mutex write_mutex_;

  static std::string create_escaped_json_string(const std::string &unescaped_text);
  static std::string create_current_utc_timestamp_text();

  void write_entry(
    const std::string &severity_text,
    const std::string &message_text,
    const std::vector<std::pair<std::string, std::string>> &named_fields);
};
} // namespace tusho

#endif
