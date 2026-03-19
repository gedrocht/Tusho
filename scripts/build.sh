#!/usr/bin/env bash

set -euo pipefail

# This script gives the repository a beginner-friendly Linux build entry point.
# It wraps the CMake commands that the project already supports so a new user
# does not have to remember every option before getting a successful build.

script_directory_path="$(
  cd "$(dirname "${BASH_SOURCE[0]}")"
  pwd
)"
project_root_directory_path="$(
  cd "${script_directory_path}/.."
  pwd
)"

selected_build_directory_path="${project_root_directory_path}/build"
selected_build_configuration="Debug"
selected_cmake_generator="Ninja"
selected_c_compiler=""
selected_cxx_compiler=""
should_clean_before_configure="false"
should_build_tests="true"
should_run_tests_after_build="false"
should_enable_sanitizers="false"
should_enable_coverage="false"

print_usage() {
  cat <<'USAGE_TEXT'
Tusho build helper

Usage:
  ./scripts/build.sh [options]

Options:
  --build-directory <path>  Override the CMake build directory. Default: ./build
  --debug                   Build with Debug configuration. This is the default.
  --release                 Build with Release configuration.
  --gcc                     Build with GCC.
  --clang                   Build with Clang.
  --clean                   Remove the selected build directory before configuring.
  --no-tests                Configure the project without compiling the C++ test suite.
  --run-tests               Run CTest after a successful build.
  --sanitizers              Enable the sanitizer build flags supported by the project.
  --coverage                Enable the coverage build flags supported by the project.
  --help                    Show this help text.

Examples:
  ./scripts/build.sh
  ./scripts/build.sh --clang --run-tests
  ./scripts/build.sh --release --build-directory ./build-release
USAGE_TEXT
}

fail_with_message() {
  local failure_message_text="$1"
  printf 'Build script error: %s\n' "${failure_message_text}" >&2
  exit 1
}

require_command() {
  local required_command_name="$1"

  if ! command -v "${required_command_name}" >/dev/null 2>&1; then
    fail_with_message "Required command not found: ${required_command_name}"
  fi
}

convert_boolean_to_cmake_option() {
  local boolean_text="$1"

  if [[ "${boolean_text}" == "true" ]]; then
    printf 'ON'
  else
    printf 'OFF'
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-directory)
      [[ $# -ge 2 ]] || fail_with_message "The --build-directory option requires a value."
      selected_build_directory_path="$2"
      shift 2
      ;;
    --debug)
      selected_build_configuration="Debug"
      shift
      ;;
    --release)
      selected_build_configuration="Release"
      shift
      ;;
    --gcc)
      selected_c_compiler="gcc"
      selected_cxx_compiler="g++"
      shift
      ;;
    --clang)
      selected_c_compiler="clang"
      selected_cxx_compiler="clang++"
      shift
      ;;
    --clean)
      should_clean_before_configure="true"
      shift
      ;;
    --no-tests)
      should_build_tests="false"
      shift
      ;;
    --run-tests)
      should_run_tests_after_build="true"
      shift
      ;;
    --sanitizers)
      should_enable_sanitizers="true"
      shift
      ;;
    --coverage)
      should_enable_coverage="true"
      shift
      ;;
    --help)
      print_usage
      exit 0
      ;;
    *)
      fail_with_message "Unknown option: $1"
      ;;
  esac
done

require_command cmake

if [[ "${selected_cmake_generator}" == "Ninja" ]]; then
  require_command ninja
fi

if [[ -n "${selected_c_compiler}" ]]; then
  require_command "${selected_c_compiler}"
fi

if [[ -n "${selected_cxx_compiler}" ]]; then
  require_command "${selected_cxx_compiler}"
fi

if [[ "${should_run_tests_after_build}" == "true" ]]; then
  require_command ctest
fi

if [[ "${should_clean_before_configure}" == "true" ]]; then
  printf 'Removing previous build directory: %s\n' "${selected_build_directory_path}"
  rm -rf "${selected_build_directory_path}"
fi

mkdir -p "${selected_build_directory_path}"

cmake_configuration_arguments=(
  -S "${project_root_directory_path}"
  -B "${selected_build_directory_path}"
  -G "${selected_cmake_generator}"
  -DCMAKE_BUILD_TYPE="${selected_build_configuration}"
  -DTUSHO_BUILD_TESTS="$(convert_boolean_to_cmake_option "${should_build_tests}")"
  -DTUSHO_ENABLE_SANITIZERS="$(convert_boolean_to_cmake_option "${should_enable_sanitizers}")"
  -DTUSHO_ENABLE_COVERAGE="$(convert_boolean_to_cmake_option "${should_enable_coverage}")"
)

printf 'Configuring Tusho in %s\n' "${selected_build_directory_path}"

if [[ -n "${selected_c_compiler}" && -n "${selected_cxx_compiler}" ]]; then
  CC="${selected_c_compiler}" CXX="${selected_cxx_compiler}" cmake "${cmake_configuration_arguments[@]}"
else
  cmake "${cmake_configuration_arguments[@]}"
fi

printf 'Building Tusho with configuration %s\n' "${selected_build_configuration}"
cmake --build "${selected_build_directory_path}"

if [[ "${should_run_tests_after_build}" == "true" ]]; then
  if [[ "${should_build_tests}" != "true" ]]; then
    fail_with_message "The --run-tests option cannot be combined with --no-tests."
  fi

  printf 'Running the C++ test suite\n'
  ctest --test-dir "${selected_build_directory_path}" --output-on-failure
fi

printf '\nBuild completed successfully.\n'
printf 'Crawler executable: %s/tusho_crawler\n' "${selected_build_directory_path}"
