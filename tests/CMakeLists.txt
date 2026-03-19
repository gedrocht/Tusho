add_executable(
  tusho_cpp_unit_tests
  support/test_support.hpp
  unit/structured_logger_tests.cpp
)

target_link_libraries(tusho_cpp_unit_tests PRIVATE Tusho::Core)
tusho_apply_project_warnings(tusho_cpp_unit_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_unit_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_unit_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_unit_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_unit_tests COMMAND tusho_cpp_unit_tests)

add_executable(
  tusho_cpp_integration_tests
  support/test_support.hpp
  integration/file_system_crawler_integration_tests.cpp
)

target_link_libraries(tusho_cpp_integration_tests PRIVATE Tusho::Core SQLite::SQLite3)
tusho_apply_project_warnings(tusho_cpp_integration_tests ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_cpp_integration_tests)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_cpp_integration_tests PRIVATE --coverage -O0 -g)
  target_link_options(tusho_cpp_integration_tests PRIVATE --coverage)
endif()

add_test(NAME tusho_cpp_integration_tests COMMAND tusho_cpp_integration_tests)
