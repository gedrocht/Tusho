#!/usr/bin/env bash

set -euo pipefail

# This script is a thin convenience wrapper around the built crawler
# executable. It exists so beginners can stay inside the scripts directory for
# the whole local workflow instead of switching back to raw build paths.

script_directory_path="$(
  cd "$(dirname "${BASH_SOURCE[0]}")"
  pwd
)"
project_root_directory_path="$(
  cd "${script_directory_path}/.."
  pwd
)"
crawler_executable_path="${project_root_directory_path}/build/tusho_crawler"

print_usage() {
  cat <<'USAGE_TEXT'
Tusho crawler launcher

Usage:
  ./scripts/run_crawler.sh <crawler arguments>

Common example:
  ./scripts/run_crawler.sh \
    --database-path /tmp/tusho-demo/catalog.sqlite3 \
    --crawl-root /tmp/tusho-demo \
    --log-file-path /tmp/tusho-demo/crawler.jsonl \
    --rebuild-database

Tip:
  Build the project first with ./scripts/build.sh
USAGE_TEXT
}

fail_with_message() {
  local failure_message_text="$1"
  printf 'Crawler launcher error: %s\n' "${failure_message_text}" >&2
  exit 1
}

if [[ $# -eq 0 ]] || [[ "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

if [[ ! -x "${crawler_executable_path}" ]]; then
  fail_with_message "The crawler executable was not found at ${crawler_executable_path}. Run ./scripts/build.sh first."
fi

exec "${crawler_executable_path}" "$@"
