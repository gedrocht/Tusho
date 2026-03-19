# Logging and Operations

## Crawler logging

The crawler emits newline-delimited JSON logs to standard output and to a log file. This makes the logs easy to:

- read with `cat`
- filter with `jq`
- collect with Fluent Bit or Vector
- forward with systemd or container logs

## Web application logging

The PHP application writes one JSON line per request. The log includes route and format information so it is easy to understand how the web UI is being used.

## Operational advice

- Keep the SQLite database on a fast local disk when possible.
- Schedule crawler runs with `systemd` timers or cron.
- Keep log files in a known directory and rotate them with `logrotate`.
- Test on a small directory before scanning `/`.
