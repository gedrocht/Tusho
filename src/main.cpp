#include "tusho/application/crawler_application.hpp"
#include "tusho/application/run_configuration.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
void write_help_text()
{
  std::cout
    << "Tusho Crawler\n"
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

tusho::RunConfiguration parse_command_line_arguments(const int argument_count, char **argument_values)
{
  tusho::RunConfiguration run_configuration{};
  run_configuration.crawl_root_directory_path = "/";
  run_configuration.log_file_path = std::filesystem::path("logs") / "tusho-crawler.jsonl";

  // The parser stays intentionally manual and small so beginners can trace
  // exactly how command-line arguments are interpreted.
  for (int current_argument_index = 1; current_argument_index < argument_count; ++current_argument_index)
  {
    const std::string_view current_argument(argument_values[current_argument_index]);

    if (current_argument == "--help")
    {
      write_help_text();
      std::exit(0);
    }

    if (current_argument == "--rebuild-database")
    {
      run_configuration.should_rebuild_database = true;
      continue;
    }

    if (current_argument == "--follow-symbolic-links")
    {
      run_configuration.should_follow_symbolic_links = true;
      continue;
    }

    if (current_argument == "--include-hidden-entries")
    {
      run_configuration.should_include_hidden_entries = true;
      continue;
    }

    if (current_argument == "--crawl-root" || current_argument == "--database-path" ||
        current_argument == "--log-file-path")
    {
      if (current_argument_index + 1 >= argument_count)
      {
        throw std::runtime_error("A command-line option that expects a value did not receive one.");
      }

      const std::filesystem::path option_value(argument_values[current_argument_index + 1]);
      ++current_argument_index;

      if (current_argument == "--crawl-root")
      {
        run_configuration.crawl_root_directory_path = option_value;
      }
      else if (current_argument == "--database-path")
      {
        run_configuration.database_file_path = option_value;
      }
      else
      {
        run_configuration.log_file_path = option_value;
      }

      continue;
    }

    throw std::runtime_error("Unknown command-line option: " + std::string(current_argument));
  }

  if (run_configuration.database_file_path.empty())
  {
    throw std::runtime_error("The --database-path option is required.");
  }

  return run_configuration;
}
} // namespace

int main(const int argument_count, char **argument_values)
{
  try
  {
    const tusho::RunConfiguration run_configuration = parse_command_line_arguments(argument_count, argument_values);
    const tusho::CrawlerApplication crawler_application;
    return crawler_application.run(run_configuration);
  }
  catch (const std::exception &exception)
  {
    std::cerr << "Tusho crawler failed: " << exception.what() << '\n';
    return 1;
  }
}
