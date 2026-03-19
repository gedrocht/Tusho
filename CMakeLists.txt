cmake_minimum_required(VERSION 3.28)

project(
  Tusho
  VERSION 0.1.0
  DESCRIPTION "Linux file system catalog crawler with a PHP and XML search interface"
  LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TUSHO_BUILD_TESTS "Build the automated test suite." ON)
option(TUSHO_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as build-breaking errors." ON)
option(TUSHO_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer for supported toolchains." OFF)
option(TUSHO_ENABLE_COVERAGE "Enable compiler instrumentation for line coverage reporting." OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectWarnings)
include(Sanitizers)

find_package(SQLite3 REQUIRED)

set(
  tusho_core_source_files
  src/application/crawler_application.cpp
  src/database/file_system_catalog_repository.cpp
  src/database/sqlite_database_connection.cpp
  src/filesystem/file_system_crawler.cpp
  src/logging/structured_logger.cpp
)

set(
  tusho_core_header_files
  include/tusho/application/crawler_application.hpp
  include/tusho/application/run_configuration.hpp
  include/tusho/database/file_system_catalog_repository.hpp
  include/tusho/database/sqlite_database_connection.hpp
  include/tusho/filesystem/file_system_crawler.hpp
  include/tusho/filesystem/file_system_entry_record.hpp
  include/tusho/filesystem/scan_statistics.hpp
  include/tusho/logging/structured_logger.hpp
)

add_library(tusho_core STATIC ${tusho_core_source_files} ${tusho_core_header_files})
add_library(Tusho::Core ALIAS tusho_core)

target_include_directories(
  tusho_core
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(tusho_core PUBLIC SQLite::SQLite3)
target_compile_features(tusho_core PUBLIC cxx_std_20)

tusho_apply_project_warnings(tusho_core ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_core)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_core PRIVATE --coverage -O0 -g)
  target_link_options(tusho_core PRIVATE --coverage)
endif()

add_executable(tusho_crawler src/main.cpp)
target_link_libraries(tusho_crawler PRIVATE Tusho::Core)
tusho_apply_project_warnings(tusho_crawler ${TUSHO_ENABLE_WARNINGS_AS_ERRORS})

if(TUSHO_ENABLE_SANITIZERS)
  tusho_enable_sanitizers(tusho_crawler)
endif()

if(TUSHO_ENABLE_COVERAGE)
  target_compile_options(tusho_crawler PRIVATE --coverage -O0 -g)
  target_link_options(tusho_crawler PRIVATE --coverage)
endif()

install(TARGETS tusho_crawler RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(TUSHO_BUILD_TESTS)
  add_subdirectory(tests)
endif()
