# Architecture

## C++ crawler

The crawler is intentionally split into small classes:

- `CrawlerApplication` coordinates a run.
- `StructuredLogger` writes structured logs.
- `SqliteDatabaseConnection` owns the SQLite connection lifetime.
- `FileSystemCatalogRepository` owns schema creation and row persistence.
- `FileSystemCrawler` performs the recursive file system walk.

## PHP web application

The PHP side is also kept small:

- `ApplicationConfigurationLoader` validates XML configuration.
- `FileSystemCatalogRepository` performs read-only queries through PDO SQLite.
- `FrontController` routes web requests.
- `HtmlDocumentRenderer` creates HTML responses.
- `XmlDocumentRenderer` creates XML responses.
- `WebRequestLogger` records request logs.

## Why this split matters

This structure keeps each class focused on one main job, which makes the code easier to test, document, and change safely.
