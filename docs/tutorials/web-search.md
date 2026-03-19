# Web Search

## Step 1: Point the XML configuration at your database

Edit `web/config/application_configuration.xml` so `databasePath` matches the SQLite file created by the crawler.

## Step 2: Install PHP dependencies

```bash
composer install
```

## Step 3: Start a local PHP server

```bash
php -S 127.0.0.1:8080 -t web/public
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
