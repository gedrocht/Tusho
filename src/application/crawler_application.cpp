#include "tusho/application/crawler_application.hpp"

#include "tusho/database/file_system_catalog_repository.hpp"
#include "tusho/database/sqlite_database_connection.hpp"
#include "tusho/filesystem/file_system_crawler.hpp"
#include "tusho/logging/structured_logger.hpp"

#include <string>

namespace tusho
{
int CrawlerApplication::run(const RunConfiguration &run_configuration) const
{
  StructuredLogger structured_logger(run_configuration.log_file_path);
  SqliteDatabaseConnection sqlite_database_connection(run_configuration.database_file_path);
  FileSystemCatalogRepository file_system_catalog_repository(sqlite_database_connection);
  FileSystemCrawler file_system_crawler(file_system_catalog_repository, structured_logger);

  const ScanStatistics scan_statistics = file_system_crawler.crawl(run_configuration);

  structured_logger.log_information(
    "The crawler application is exiting successfully.",
    {
      {"scan_run_identifier", std::to_string(scan_statistics.scan_run_identifier)},
      {"accessible_entry_count", std::to_string(scan_statistics.accessible_entry_count)},
      {"inaccessible_entry_count", std::to_string(scan_statistics.inaccessible_entry_count)},
    });

  return 0;
}
} // namespace tusho
