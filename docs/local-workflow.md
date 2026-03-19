# Local Workflow

This page is the simplest possible checklist for a complete beginner who wants
to install prerequisites, build the project, run the crawler, start the web
application, and then test the whole repository.

## One-command summary for each phase

1. Install prerequisites: `./scripts/setup_linux.sh`
2. Build and run the C++ tests: `./scripts/build.sh --run-tests`
3. Run the crawler: `./scripts/run_crawler.sh --database-path /tmp/tusho-demo/catalog.sqlite3 --crawl-root /tmp/tusho-demo --log-file-path /tmp/tusho-demo/crawler.jsonl --rebuild-database`
4. Start the web application: `./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3`
5. Run all local tests: `./scripts/test.sh`

## Copy-and-paste beginner walkthrough

```bash
./scripts/setup_linux.sh
mkdir -p /tmp/tusho-demo
touch /tmp/tusho-demo/example.txt
mkdir -p /tmp/tusho-demo/folder
./scripts/build.sh --run-tests
./scripts/run_crawler.sh --database-path /tmp/tusho-demo/catalog.sqlite3 --crawl-root /tmp/tusho-demo --log-file-path /tmp/tusho-demo/crawler.jsonl --rebuild-database
./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3
```

Open [http://127.0.0.1:8080/index.php](http://127.0.0.1:8080/index.php) in a
browser after the last command starts the PHP server.

## What each result file means

- `/tmp/tusho-demo/catalog.sqlite3`: the SQLite database created by the crawler
- `/tmp/tusho-demo/crawler.jsonl`: the crawler log file in JSON Lines format
- `/tmp/tusho-demo/web-application.jsonl`: the web application request log file

## If you only want to re-run the tests later

```bash
./scripts/test.sh
```

## If you prefer raw commands instead of helper scripts

If you use the raw PHP server command, remember that the web app must still
know which XML configuration file to load. The helper script handles that
automatically. The manual path expects either an edited
`web/config/application_configuration.xml` file or a
`TUSHO_WEB_CONFIGURATION_PATH` environment variable that points at a local XML
configuration file.

```bash
cmake -S . -B build -G Ninja -DTUSHO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
composer install
composer run lint:php
composer run analyse:php
composer run test:php
php -S 127.0.0.1:8080 -t web/public
```
