# Getting Started

## What this project does

The C++ crawler walks through a Linux directory tree and records metadata about every discovered file system entry in SQLite. The PHP web application then reads that SQLite database and makes it searchable and browsable.

## Why these technologies were chosen

- C++ keeps the crawler lean and close to the operating system.
- SQLite keeps deployment simple because the database is just a file.
- PHP is easy to serve on many Linux systems.
- XML provides a strict, validated configuration format for the web application.

## Beginner mental model

Think of Tusho as three layers:

1. Discovery layer: the crawler asks Linux what files and directories exist.
2. Storage layer: SQLite stores the answers in tables.
3. Presentation layer: PHP turns those rows into HTML pages and XML responses.

## The shortest possible beginner workflow

Run these commands in order:

```bash
./scripts/setup_linux.sh
mkdir -p /tmp/tusho-demo
touch /tmp/tusho-demo/example.txt
./scripts/build.sh --run-tests
./scripts/run_crawler.sh --database-path /tmp/tusho-demo/catalog.sqlite3 --crawl-root /tmp/tusho-demo --log-file-path /tmp/tusho-demo/crawler.jsonl --rebuild-database
./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3
```

Then open [http://127.0.0.1:8080/index.php](http://127.0.0.1:8080/index.php).

## What each script is for

### `./scripts/setup_linux.sh`

Use this first. It installs the Linux packages needed for building the C++
crawler and running the PHP application. It also installs the PHP development
dependencies from Composer so the local PHP tests can run later.

### `./scripts/build.sh --run-tests`

Use this after setup. It configures CMake, builds the crawler, and runs the C++
tests so you know the compiled part of the project is healthy before you start
indexing anything.

### `./scripts/run_crawler.sh`

Use this to create or refresh the SQLite catalog. This script simply forwards
your arguments to the built crawler executable, but it gives you an obvious
entry point and checks that the project has been built first.

### `./scripts/run_web_application.sh`

Use this to start the PHP web interface. The script generates a temporary local
XML configuration file for you, so you do not have to edit the tracked
configuration file just to point the web app at your local SQLite database.

### `./scripts/test.sh`

Use this any time you want one obvious command for local verification. It runs
the C++ build and test workflow and then runs the PHP coding standards, PHP
static analysis, and PHPUnit commands.

## Recommended first demo directory

For a first run, use a small directory under `/tmp` so the results stay easy to
inspect:

```bash
mkdir -p /tmp/tusho-demo
touch /tmp/tusho-demo/example.txt
mkdir -p /tmp/tusho-demo/folder
```

## How to tell whether each step worked

### Setup succeeded when

- the script finishes without an error message
- `composer install` completes
- the script prints the next-step commands

### Build succeeded when

- the script prints `Build completed successfully.`
- the crawler exists at `./build/tusho_crawler`
- the C++ test run finishes with all tests passing

### Crawl succeeded when

- the SQLite file exists at the path you supplied with `--database-path`
- the log file exists at the path you supplied with `--log-file-path`
- the crawler prints JSON log lines describing the scan

### Web server succeeded when

- the script prints a local URL such as `http://127.0.0.1:8080`
- opening that address in a browser shows the Tusho home page

### Full local testing succeeded when

- `./scripts/test.sh` finishes without stopping
- the final line says `All local tests completed successfully.`
