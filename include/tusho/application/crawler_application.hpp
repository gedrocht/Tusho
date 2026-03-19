#ifndef TUSHO_APPLICATION_CRAWLER_APPLICATION_HPP
#define TUSHO_APPLICATION_CRAWLER_APPLICATION_HPP

#include "tusho/application/run_configuration.hpp"

namespace tusho
{
/**
 * @brief Coordinates the high-level steps of a crawler run.
 */
class CrawlerApplication
{
public:
  static int run(const RunConfiguration& run_configuration);
};
} // namespace tusho

#endif
