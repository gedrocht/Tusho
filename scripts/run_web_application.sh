#!/usr/bin/env bash

set -euo pipefail

# This script starts the PHP web application with a generated local XML
# configuration file. That keeps the beginner workflow simple because the user
# does not have to edit the tracked configuration file just to point the web
# interface at a local demo database.

script_directory_path="$(
  cd "$(dirname "${BASH_SOURCE[0]}")"
  pwd
)"
project_root_directory_path="$(
  cd "${script_directory_path}/.."
  pwd
)"

selected_database_path=""
selected_web_log_file_path="/tmp/tusho-demo/web-application.jsonl"
selected_site_title="Tusho File Catalog"
selected_page_size="50"
selected_host_address="127.0.0.1"
selected_port_number="8080"
selected_generated_configuration_path="${project_root_directory_path}/build/local-web-application-configuration.xml"

print_usage() {
  cat <<'USAGE_TEXT'
Tusho web application launcher

Usage:
  ./scripts/run_web_application.sh --database-path <sqlite database path> [options]

Options:
  --database-path <path>      Required. SQLite database created by the crawler.
  --web-log-file-path <path>  Optional. Default: /tmp/tusho-demo/web-application.jsonl
  --site-title <text>         Optional. Default: Tusho File Catalog
  --page-size <number>        Optional. Default: 50
  --host <address>            Optional. Default: 127.0.0.1
  --port <number>             Optional. Default: 8080
  --help                      Show this help text.

Example:
  ./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3
USAGE_TEXT
}

fail_with_message() {
  local failure_message_text="$1"
  printf 'Web launcher error: %s\n' "${failure_message_text}" >&2
  exit 1
}

require_command() {
  local required_command_name="$1"

  if ! command -v "${required_command_name}" >/dev/null 2>&1; then
    fail_with_message "Required command not found: ${required_command_name}"
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --database-path)
      [[ $# -ge 2 ]] || fail_with_message "The --database-path option requires a value."
      selected_database_path="$2"
      shift 2
      ;;
    --web-log-file-path)
      [[ $# -ge 2 ]] || fail_with_message "The --web-log-file-path option requires a value."
      selected_web_log_file_path="$2"
      shift 2
      ;;
    --site-title)
      [[ $# -ge 2 ]] || fail_with_message "The --site-title option requires a value."
      selected_site_title="$2"
      shift 2
      ;;
    --page-size)
      [[ $# -ge 2 ]] || fail_with_message "The --page-size option requires a value."
      selected_page_size="$2"
      shift 2
      ;;
    --host)
      [[ $# -ge 2 ]] || fail_with_message "The --host option requires a value."
      selected_host_address="$2"
      shift 2
      ;;
    --port)
      [[ $# -ge 2 ]] || fail_with_message "The --port option requires a value."
      selected_port_number="$2"
      shift 2
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

require_command php

if [[ -z "${selected_database_path}" ]]; then
  fail_with_message "The --database-path option is required."
fi

if [[ ! -f "${selected_database_path}" ]]; then
  fail_with_message "The SQLite database file does not exist yet: ${selected_database_path}"
fi

if [[ ! -f "${project_root_directory_path}/vendor/autoload.php" ]]; then
  require_command composer
  printf 'Composer dependencies are missing. Installing them now.\n'
  (
    cd "${project_root_directory_path}"
    COMPOSER_ALLOW_SUPERUSER=1 composer install --no-interaction --prefer-dist
  )
fi

mkdir -p "$(dirname "${selected_generated_configuration_path}")"
mkdir -p "$(dirname "${selected_web_log_file_path}")"

cat >"${selected_generated_configuration_path}" <<CONFIGURATION_TEXT
<?xml version="1.0" encoding="UTF-8"?>
<applicationConfiguration>
  <siteTitle>${selected_site_title}</siteTitle>
  <databasePath>${selected_database_path}</databasePath>
  <webLogFilePath>${selected_web_log_file_path}</webLogFilePath>
  <defaultPageSize>${selected_page_size}</defaultPageSize>
</applicationConfiguration>
CONFIGURATION_TEXT

printf 'Starting the Tusho web application on http://%s:%s\n' "${selected_host_address}" "${selected_port_number}"
printf 'Generated local configuration: %s\n' "${selected_generated_configuration_path}"
printf 'Press Ctrl+C to stop the local PHP server.\n'

export TUSHO_WEB_CONFIGURATION_PATH="${selected_generated_configuration_path}"
exec php -S "${selected_host_address}:${selected_port_number}" -t "${project_root_directory_path}/web/public"
