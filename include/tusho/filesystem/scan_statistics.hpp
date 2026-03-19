#ifndef TUSHO_FILESYSTEM_SCAN_STATISTICS_HPP
#define TUSHO_FILESYSTEM_SCAN_STATISTICS_HPP

#include <cstdint>

namespace tusho
{
/**
 * @brief Summarizes what happened during a crawler scan.
 */
struct ScanStatistics
{
  /**
   * @brief The database identifier of the recorded scan run.
   */
  std::int64_t scan_run_identifier = 0;

  /**
   * @brief The number of file system entries that were read successfully.
   */
  std::uint64_t accessible_entry_count = 0;

  /**
   * @brief The number of file system entries that were discovered but not read successfully.
   */
  std::uint64_t inaccessible_entry_count = 0;
};
} // namespace tusho

#endif
