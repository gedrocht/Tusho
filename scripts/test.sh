#!/usr/bin/env bash

set -euo pipefail

# This script runs the local checks a beginner is most likely to care about:
# build the crawler, run the C++ tests, install PHP dependencies when needed,
# then run the PHP quality and test commands.

script_directory_path="$(
  cd "$(dirname "${BASH_SOURCE[0]}")"
  pwd
)"
project_root_directory_path="$(
  cd "${script_directory_path}/.."
  pwd
)"

print_usage() {
  cat <<'USAGE_TEXT'
Tusho local test helper

Usage:
  ./scripts/test.sh

This script will:
  1. build the C++ crawler
  2. run the C++ CTest suite
  3. install Composer dependencies when needed
  4. run PHP coding standards
  5. run PHP static analysis
  6. run PHPUnit
USAGE_TEXT
}

fail_with_message() {
  local failure_message_text="$1"
  printf 'Test script error: %s\n' "${failure_message_text}" >&2
  exit 1
}

require_command() {
  local required_command_name="$1"

  if ! command -v "${required_command_name}" >/dev/null 2>&1; then
    fail_with_message "Required command not found: ${required_command_name}"
  fi
}

if [[ "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

require_command composer

printf 'Running the C++ build and test workflow\n'
"${project_root_directory_path}/scripts/build.sh" --run-tests

if [[ ! -f "${project_root_directory_path}/vendor/autoload.php" ]]; then
  printf 'Composer dependencies are missing. Installing them now.\n'
fi

(
  cd "${project_root_directory_path}"
  COMPOSER_ALLOW_SUPERUSER=1 composer install --no-interaction --prefer-dist
  printf 'Running PHP coding standards\n'
  COMPOSER_ALLOW_SUPERUSER=1 composer run lint:php
  printf 'Running PHP static analysis\n'
  COMPOSER_ALLOW_SUPERUSER=1 composer run analyse:php
  printf 'Running PHPUnit\n'
  COMPOSER_ALLOW_SUPERUSER=1 composer run test:php
)

printf '\nAll local tests completed successfully.\n'
