# Database Schema

Tusho currently uses two main tables:

## `scan_runs`

Each crawl creates one row in `scan_runs`. That row records:

- which root directory was scanned
- when the scan started
- when the scan finished
- how many entries were accessible
- how many entries were inaccessible

## `file_system_entries`

Each discovered entry becomes one row in `file_system_entries`.

Important columns:

- `absolute_path`
- `parent_directory_path`
- `entry_name`
- `entry_type`
- `symbolic_link_target_path`
- `permissions_octal_text`
- `file_size_bytes`
- `owner_user_identifier`
- `owner_group_identifier`
- `inode_number`
- `device_number`
- `hard_link_count`
- `last_write_time_utc_text`
- `last_status_change_time_utc_text`
- `was_accessible`
- `scan_run_identifier`

## Why paths are stored as text

Storing paths as text makes search and web rendering simple. It also lets administrators inspect the catalog directly with ordinary SQLite queries.
