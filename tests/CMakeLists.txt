add_executable(
  tusho_cpp_structured_logger_tests
  support/test_support.hpp
  unit/structured_logger_tests.cpp
)

target_link_libraries(tusho_cpp_structured_logger_tests PRIVATE Tusho::Core)
tusho_apply_project_warnings(tusho_cpp_structured_logger_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_structured_logger_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_structured_logger_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_structured_logger_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_structured_logger_tests COMMAND tusho_cpp_structured_logger_tests)

add_executable(
  tusho_cpp_database_and_application_tests
  support/test_support.hpp
  unit/database_and_application_tests.cpp
)

target_link_libraries(tusho_cpp_database_and_application_tests PRIVATE Tusho::Core SQLite::SQLite3)
tusho_apply_project_warnings(tusho_cpp_database_and_application_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_database_and_application_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_database_and_application_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_database_and_application_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_database_and_application_tests COMMAND tusho_cpp_database_and_application_tests)

add_executable(
  tusho_cpp_file_system_crawler_integration_tests
  support/test_support.hpp
  integration/file_system_crawler_integration_tests.cpp
)

target_link_libraries(tusho_cpp_file_system_crawler_integration_tests PRIVATE Tusho::Core SQLite::SQLite3)
tusho_apply_project_warnings(tusho_cpp_file_system_crawler_integration_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_file_system_crawler_integration_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_file_system_crawler_integration_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_file_system_crawler_integration_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_file_system_crawler_integration_tests COMMAND tusho_cpp_file_system_crawler_integration_tests)

add_executable(
  tusho_cpp_cli_integration_tests
  support/test_support.hpp
  integration/crawler_command_line_integration_tests.cpp
)

target_link_libraries(tusho_cpp_cli_integration_tests PRIVATE Tusho::Core SQLite::SQLite3)
tusho_apply_project_warnings(tusho_cpp_cli_integration_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})
target_compile_definitions(
  tusho_cpp_cli_integration_tests
  PRIVATE
    TUSHO_CRAWLER_EXECUTABLE_PATH="$<TARGET_FILE:tusho_crawler>"
)

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_cli_integration_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_cli_integration_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_cli_integration_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_cli_integration_tests COMMAND tusho_cpp_cli_integration_tests)
add_test(NAME tusho_crawler_help_smoke_test COMMAND tusho_crawler --help)
add_test(NAME tusho_crawler_missing_database_option_test COMMAND tusho_crawler)
add_test(NAME tusho_crawler_unknown_option_test COMMAND tusho_crawler --database-path test.sqlite3 --unknown-option)
set_tests_properties(tusho_crawler_missing_database_option_test PROPERTIES WILL_FAIL TRUE)
set_tests_properties(tusho_crawler_unknown_option_test PROPERTIES WILL_FAIL TRUE)
