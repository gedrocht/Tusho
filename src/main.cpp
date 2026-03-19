#include "tusho/application/crawler_application.hpp"
#include "tusho/application/run_configuration.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
void write_help_text()
{
  std::cout << "Tusho Crawler\n"
            << "Usage:\n"
            << "  tusho_crawler --database-path <path> [options]\n\n"
            << "Options:\n"
            << "  --crawl-root <path>            Root directory to scan. Default: /\n"
            << "  --database-path <path>         SQLite database destination. Required.\n"
            << "  --log-file-path <path>         Structured log destination. Default: ./logs/tusho-crawler.jsonl\n"
            << "  --rebuild-database             Remove previous catalog rows before scanning.\n"
            << "  --follow-symbolic-links        Traverse into linked directories.\n"
            << "  --include-hidden-entries       Index entries whose names begin with a period.\n"
            << "  --help                         Show this help text.\n";
}

struct CommandLineParseResult
{
  tusho::RunConfiguration run_configuration{};
  bool should_exit_after_showing_help = false;
};

CommandLineParseResult parse_command_line_arguments(const int argument_count, char** argument_values)
{
  CommandLineParseResult command_line_parse_result{};
  command_line_parse_result.run_configuration.crawl_root_directory_path = "/";
  command_line_parse_result.run_configuration.log_file_path = std::filesystem::path("logs") / "tusho-crawler.jsonl";
  const std::span<char*> argument_value_span(argument_values, static_cast<std::size_t>(argument_count));

  // The parser stays intentionally manual and small so beginners can trace
  // exactly how command-line arguments are interpreted.
  for (int current_argument_index = 1; current_argument_index < argument_count; ++current_argument_index)
  {
    const std::string_view current_argument(argument_value_span[static_cast<std::size_t>(current_argument_index)]);

    if (current_argument == "--help")
    {
      write_help_text();
      command_line_parse_result.should_exit_after_showing_help = true;
      return command_line_parse_result;
    }

    if (current_argument == "--rebuild-database")
    {
      command_line_parse_result.run_configuration.should_rebuild_database = true;
      continue;
    }

    if (current_argument == "--follow-symbolic-links")
    {
      command_line_parse_result.run_configuration.should_follow_symbolic_links = true;
      continue;
    }

    if (current_argument == "--include-hidden-entries")
    {
      command_line_parse_result.run_configuration.should_include_hidden_entries = true;
      continue;
    }

    if (current_argument == "--crawl-root" || current_argument == "--database-path" ||
        current_argument == "--log-file-path")
    {
      if (current_argument_index + 1 >= argument_count)
      {
        throw std::runtime_error("A command-line option that expects a value did not receive one.");
      }

      const std::size_t option_value_argument_index = static_cast<std::size_t>(current_argument_index) + 1U;
      const std::filesystem::path option_value(argument_value_span[option_value_argument_index]);
      ++current_argument_index;

      if (current_argument == "--crawl-root")
      {
        command_line_parse_result.run_configuration.crawl_root_directory_path = option_value;
      }
      else if (current_argument == "--database-path")
      {
        command_line_parse_result.run_configuration.database_file_path = option_value;
      }
      else
      {
        command_line_parse_result.run_configuration.log_file_path = option_value;
      }

      continue;
    }

    throw std::runtime_error("Unknown command-line option: " + std::string(current_argument));
  }

  if (command_line_parse_result.run_configuration.database_file_path.empty())
  {
    throw std::runtime_error("The --database-path option is required.");
  }

  return command_line_parse_result;
}
} // namespace

int main(const int argument_count, char** argument_values)
{
  try
  {
    const CommandLineParseResult command_line_parse_result =
        parse_command_line_arguments(argument_count, argument_values);

    if (command_line_parse_result.should_exit_after_showing_help)
    {
      return 0;
    }

    return tusho::CrawlerApplication::run(command_line_parse_result.run_configuration);
  }
  catch (const std::exception& exception)
  {
    std::cerr << "Tusho crawler failed: " << exception.what() << '\n';
    return 1;
  }
}
