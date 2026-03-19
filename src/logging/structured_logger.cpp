#include "tusho/logging/structured_logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace tusho
{
StructuredLogger::StructuredLogger(const std::filesystem::path& log_file_path)
{
  if (!log_file_path.empty())
  {
    if (log_file_path.has_parent_path())
    {
      std::filesystem::create_directories(log_file_path.parent_path());
    }

    log_file_stream_.open(log_file_path, std::ios::app);

    if (!log_file_stream_.is_open())
    {
      throw std::runtime_error("Unable to open the requested log file: " + log_file_path.string());
    }
  }
}

void StructuredLogger::log_information(const std::string& message_text,
                                       const std::vector<std::pair<std::string, std::string>>& named_fields)
{
  write_entry("information", message_text, named_fields);
}

void StructuredLogger::log_warning(const std::string& message_text,
                                   const std::vector<std::pair<std::string, std::string>>& named_fields)
{
  write_entry("warning", message_text, named_fields);
}

void StructuredLogger::log_error(const std::string& message_text,
                                 const std::vector<std::pair<std::string, std::string>>& named_fields)
{
  write_entry("error", message_text, named_fields);
}

std::string StructuredLogger::create_escaped_json_string(const std::string& unescaped_text)
{
  std::ostringstream escaped_text_stream;

  for (const char current_character : unescaped_text)
  {
    switch (current_character)
    {
    case '\\':
      escaped_text_stream << "\\\\";
      break;
    case '"':
      escaped_text_stream << "\\\"";
      break;
    case '\b':
      escaped_text_stream << "\\b";
      break;
    case '\f':
      escaped_text_stream << "\\f";
      break;
    case '\n':
      escaped_text_stream << "\\n";
      break;
    case '\r':
      escaped_text_stream << "\\r";
      break;
    case '\t':
      escaped_text_stream << "\\t";
      break;
    default:
      if (static_cast<unsigned char>(current_character) < 0x20U)
      {
        escaped_text_stream << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                            << static_cast<int>(static_cast<unsigned char>(current_character)) << std::dec
                            << std::setfill(' ');
      }
      else
      {
        escaped_text_stream << current_character;
      }
      break;
    }
  }

  return escaped_text_stream.str();
}

std::string StructuredLogger::create_current_utc_timestamp_text()
{
  const auto current_time_point = std::chrono::system_clock::now();
  const std::time_t current_time_value = std::chrono::system_clock::to_time_t(current_time_point);

  std::tm utc_time_components{};
  gmtime_r(&current_time_value, &utc_time_components);

  std::ostringstream timestamp_stream;
  timestamp_stream << std::put_time(&utc_time_components, "%Y-%m-%dT%H:%M:%SZ");
  return timestamp_stream.str();
}

void StructuredLogger::write_entry(const std::string& severity_text, const std::string& message_text,
                                   const std::vector<std::pair<std::string, std::string>>& named_fields)
{
  const std::lock_guard<std::mutex> write_lock(write_mutex_);

  std::ostringstream json_entry_stream;
  json_entry_stream << '{' << R"("timestamp":")" << create_escaped_json_string(create_current_utc_timestamp_text())
                    << R"(","severity":")" << create_escaped_json_string(severity_text) << R"(","message":")"
                    << create_escaped_json_string(message_text) << '"';

  for (const auto& [field_name, field_value] : named_fields)
  {
    json_entry_stream << ",\"" << create_escaped_json_string(field_name) << "\":\""
                      << create_escaped_json_string(field_value) << "\"";
  }

  json_entry_stream << "}";

  std::cout << json_entry_stream.str() << '\n';

  if (log_file_stream_.is_open())
  {
    log_file_stream_ << json_entry_stream.str() << '\n';
    log_file_stream_.flush();
  }
}
} // namespace tusho
