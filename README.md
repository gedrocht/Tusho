# Tusho

Tusho is a Linux-first file catalog system made of two parts:

1. A lean C++ crawler that walks a Linux file system and stores file and directory metadata in SQLite.
2. A PHP web application that uses XML configuration to search and browse that catalog as either HTML or XML.

The project is designed to be understandable to beginners without giving up engineering discipline. Variable names are intentionally descriptive, source files are heavily documented, the repository ships with strict CI, and the documentation stack includes both API reference generation and a separate serveable wiki layer.

## Project goals

- Keep the crawler lean and fast by using C++, the Linux file system APIs, and SQLite.
- Keep the web layer simple and portable by using PHP with PDO SQLite and XML configuration.
- Make operations transparent with structured JSON logging for both the crawler and the web application.
- Make onboarding approachable with beginner-friendly docs, tutorials, architecture notes, and a wiki deployment.
- Make quality visible and enforceable with strict GitHub Actions checks.

## Repository layout

```text
.
|-- CMakeLists.txt                    # C++ build entry point
|-- include/                          # Public C++ headers with Doxygen comments
|-- src/                              # C++ crawler implementation
|-- tests/                            # C++ unit and integration tests
|-- web/                              # PHP web application, XML config, and PHP tests
|-- docs/                             # GitHub Pages documentation source
|-- wiki/                             # DokuWiki-based beginner wiki setup
|-- .github/workflows/                # CI, security, docs, and release checks
`-- composer.json                     # PHP dependency and quality tooling definition
```

## What the crawler stores

For each discovered file system entry, Tusho stores:

- absolute path
- parent directory path
- entry name
- entry type
- symbolic link target path when relevant
- file size
- permissions in octal text
- owner and group identifiers
- inode and device identifiers
- hard link count
- last write time
- last metadata change time
- whether the entry was accessible during the scan
- which scan run recorded the row

## Quick start for Linux

### 1. Install system packages

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build libsqlite3-dev php-cli php-xml php-sqlite3 composer
```

### 2. Build the crawler

```bash
cmake -S . -B build -G Ninja -DTUSHO_BUILD_TESTS=ON
cmake --build build
```

### 3. Run a first crawl

```bash
./build/tusho_crawler \
  --database-path /var/lib/tusho/catalog.sqlite3 \
  --crawl-root /home \
  --log-file-path /var/log/tusho/crawler.jsonl \
  --rebuild-database
```

### 4. Install PHP dependencies

```bash
composer install
```

### 5. Launch the web application locally

```bash
php -S 127.0.0.1:8080 -t web/public
```

Then open [http://127.0.0.1:8080/index.php](http://127.0.0.1:8080/index.php).

## XML configuration

The PHP web application uses [`web/config/application_configuration.xml`](web/config/application_configuration.xml). That file is validated against [`web/config/application_configuration.xsd`](web/config/application_configuration.xsd) before the application serves requests.

Example:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<applicationConfiguration>
  <siteTitle>Tusho File Catalog</siteTitle>
  <databasePath>/var/lib/tusho/catalog.sqlite3</databasePath>
  <webLogFilePath>/var/log/tusho/web-application.log</webLogFilePath>
  <defaultPageSize>50</defaultPageSize>
</applicationConfiguration>
```

## Logging

- The crawler writes newline-delimited JSON logs to standard output and the configured log file.
- The PHP application writes newline-delimited JSON request logs to the configured web log file.
- Because the logs are plain JSON lines, they work well with `jq`, systemd journal forwarding, Fluent Bit, Vector, Loki pipelines, and many hosted log products.

Example crawler log entry:

```json
{"timestamp":"2026-03-18T12:00:00Z","severity":"information","message":"The crawler completed the recursive scan.","scan_run_identifier":"1","accessible_entry_count":"120","inaccessible_entry_count":"3"}
```

## Testing and quality

The repository is set up to enforce:

- strict C++ warnings
- `clang-format`
- `clang-tidy`
- `cppcheck`
- C++ unit and integration tests with CTest
- sanitizer builds
- C++ coverage thresholds
- PHP linting, static analysis, and PHPUnit
- PHP coverage thresholds
- Markdown, YAML, shell, and GitHub Actions linting
- secret scanning
- CodeQL
- dependency review
- OpenSSF Scorecards
- documentation builds for GitHub Pages

## Documentation layers

- Beginner guides and architecture docs: [`docs/`](docs/)
- Generated API reference: Doxygen, built in CI and published with Pages
- Beginner wiki with actual wiki software: [`wiki/`](wiki/)

## External library documentation

- SQLite: [https://www.sqlite.org/docs.html](https://www.sqlite.org/docs.html)
- PDO SQLite: [https://www.php.net/manual/en/ref.pdo-sqlite.php](https://www.php.net/manual/en/ref.pdo-sqlite.php)
- CMake: [https://cmake.org/cmake/help/latest/](https://cmake.org/cmake/help/latest/)
- Doxygen: [https://www.doxygen.nl/manual/index.html](https://www.doxygen.nl/manual/index.html)
- DokuWiki: [https://www.dokuwiki.org/manual](https://www.dokuwiki.org/manual)

## Current status

This repository now contains a working project skeleton with real crawler, web, test, and documentation code. Because this development environment does not currently have a Linux C++ compiler or PHP runtime installed, the Linux-targeted build and runtime verification are enforced primarily in GitHub Actions and should also be run on an Ubuntu machine locally.
