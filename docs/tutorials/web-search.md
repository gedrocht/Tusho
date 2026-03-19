# Web Search

## Step 1: Point the XML configuration at your database

The simplest beginner path is to let the helper script generate a local XML
configuration automatically. That means you do not have to edit the tracked XML
file for a local demo.

## Step 2: Install PHP dependencies

```bash
composer install
```

## Step 3: Start a local PHP server

```bash
./scripts/run_web_application.sh --database-path /tmp/tusho-demo/catalog.sqlite3
```

## Step 4: Open the web interface

Visit [http://127.0.0.1:8080/index.php](http://127.0.0.1:8080/index.php).

## Step 5: Try HTML and XML output

HTML search:

```text
http://127.0.0.1:8080/index.php?route=search&query=example
```

XML search:

```text
http://127.0.0.1:8080/index.php?route=search&query=example&format=xml
```

Directory browsing:

```text
http://127.0.0.1:8080/index.php?route=browse&directory=/tmp/tusho-demo
```
