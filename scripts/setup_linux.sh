#!/usr/bin/env bash

set -euo pipefail

# This script prepares a Linux machine for local Tusho development. The goal is
# to give a complete beginner one command that installs the system tools and
# PHP packages required by the rest of the documented workflow.

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
Tusho Linux setup helper

Usage:
  ./scripts/setup_linux.sh
  ./scripts/setup_linux.sh --skip-composer-install

Options:
  --skip-composer-install  Install system prerequisites but do not run composer install.
  --help                   Show this help text.
USAGE_TEXT
}

fail_with_message() {
  local failure_message_text="$1"
  printf 'Setup script error: %s\n' "${failure_message_text}" >&2
  exit 1
}

require_command() {
  local required_command_name="$1"

  if ! command -v "${required_command_name}" >/dev/null 2>&1; then
    fail_with_message "Required command not found: ${required_command_name}"
  fi
}

should_skip_composer_install="false"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-composer-install)
      should_skip_composer_install="true"
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

require_command sudo
require_command apt-get

printf 'Installing Linux prerequisites for Tusho\n'
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  libsqlite3-dev \
  php-cli \
  php-curl \
  php-mbstring \
  php-xml \
  php-sqlite3 \
  composer \
  unzip

if [[ "${should_skip_composer_install}" != "true" ]]; then
  require_command composer

  printf 'Installing PHP development dependencies with Composer\n'
  (
    cd "${project_root_directory_path}"
    COMPOSER_ALLOW_SUPERUSER=1 composer install --no-interaction --prefer-dist
  )
fi

printf '\nSetup completed successfully.\n'
printf 'Recommended next steps:\n'
printf '  1. ./scripts/build.sh --run-tests\n'
printf '  2. ./scripts/run_crawler.sh --database-path /tmp/tusho-demo/catalog.sqlite3 --crawl-root /tmp/tusho-demo --log-file-path /tmp/tusho-demo/crawler.jsonl --rebuild-database\n'
printf '  3. ./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3\n'
