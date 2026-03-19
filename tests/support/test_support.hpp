#ifndef TUSHO_TESTS_SUPPORT_TEST_SUPPORT_HPP
#define TUSHO_TESTS_SUPPORT_TEST_SUPPORT_HPP

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace tusho::tests
{
inline void expect_condition(
  const bool condition_is_true,
  const std::string &failure_message_text,
  const std::string &source_file_path_text,
  const int source_line_number)
{
  if (!condition_is_true)
  {
    throw std::runtime_error(
      "Expectation failed at " + source_file_path_text + ":" + std::to_string(source_line_number) + " - " +
      failure_message_text);
  }
}

inline std::filesystem::path create_temporary_test_directory_path(const std::string &directory_name_suffix)
{
  const auto current_time_value = std::chrono::steady_clock::now().time_since_epoch().count();
  const std::filesystem::path test_directory_path =
    std::filesystem::temp_directory_path() /
    ("tusho-" + directory_name_suffix + "-" + std::to_string(current_time_value));

  std::filesystem::remove_all(test_directory_path);
  std::filesystem::create_directories(test_directory_path);
  return test_directory_path;
}
} // namespace tusho::tests

#define TUSHO_EXPECT(condition_expression, failure_message_text) \
  ::tusho::tests::expect_condition((condition_expression), (failure_message_text), __FILE__, __LINE__)

#endif
