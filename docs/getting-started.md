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

## Basic workflow

1. Build the crawler.
2. Run the crawler against a root directory.
3. Confirm that the SQLite database file exists.
4. Point the PHP configuration file at that database.
5. Start the PHP web server and search or browse the results.
