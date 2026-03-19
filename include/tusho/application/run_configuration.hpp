#ifndef TUSHO_APPLICATION_RUN_CONFIGURATION_HPP
#define TUSHO_APPLICATION_RUN_CONFIGURATION_HPP

#include <filesystem>

namespace tusho
{
/**
 * @brief Stores the command-line choices that control a crawler run.
 *
 * The crawler is intentionally configured with a small number of plainly named
 * options because this project is meant to be understandable to beginners.
 * Each field maps directly to a user intent that is described in the README and
 * beginner tutorials.
 */
struct RunConfiguration
{
  /**
   * @brief The SQLite database file that will receive the file catalog.
   */
  std::filesystem::path database_file_path;

  /**
   * @brief The root directory from which recursive crawling should begin.
   */
  std::filesystem::path crawl_root_directory_path;

  /**
   * @brief A file path for the structured JSON log output.
   */
  std::filesystem::path log_file_path;

  /**
   * @brief Whether the crawler should remove prior catalog data before scanning.
   */
  bool should_rebuild_database = false;

  /**
   * @brief Whether symbolic links should be followed when traversing directories.
   */
  bool should_follow_symbolic_links = false;

  /**
   * @brief Whether entries whose names start with "." should be indexed.
   */
  bool should_include_hidden_entries = false;
};
} // namespace tusho

#endif
