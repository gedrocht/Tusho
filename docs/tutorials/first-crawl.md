# First Crawl

This tutorial walks through a first small scan so you can understand what the crawler is doing without indexing an entire machine immediately.

## Step 1: Build the crawler

```bash
cmake -S . -B build -G Ninja -DTUSHO_BUILD_TESTS=ON
cmake --build build
```

## Step 2: Choose a safe starting directory

For your first run, scan a small location such as your home directory:

```bash
mkdir -p /tmp/tusho-demo
touch /tmp/tusho-demo/example.txt
mkdir -p /tmp/tusho-demo/folder
```

## Step 3: Run the crawler

```bash
./build/tusho_crawler \
  --database-path /tmp/tusho-demo/catalog.sqlite3 \
  --crawl-root /tmp/tusho-demo \
  --log-file-path /tmp/tusho-demo/crawler.jsonl \
  --rebuild-database
```

## Step 4: Inspect the results

You can inspect the SQLite database directly:

```bash
sqlite3 /tmp/tusho-demo/catalog.sqlite3 "SELECT entry_type, entry_name, absolute_path FROM file_system_entries ORDER BY absolute_path;"
```

You can inspect the logs:

```bash
cat /tmp/tusho-demo/crawler.jsonl | jq .
```

## What just happened

- The crawler created the database if it did not already exist.
- The schema was created.
- A scan run record was inserted.
- Each discovered file or directory became one row in `file_system_entries`.
- A completion log entry was written when the scan finished.
