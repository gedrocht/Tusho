<?php

declare(strict_types=1);

namespace TushoWebTests\Support;

use PDO;
use PHPUnit\Framework\Assert;

final class TestDatabaseFactory
{
    public static function createTemporaryDatabase(): string
    {
        $databasePath = tempnam(sys_get_temp_dir(), 'tusho-web-');

        if ($databasePath === false) {
            Assert::fail('A temporary database path could not be created.');
        }

        $databaseConnection = new PDO('sqlite:' . $databasePath);
        self::executeStatementOrFail(
            $databaseConnection,
            'CREATE TABLE scan_runs (
                scan_run_identifier INTEGER PRIMARY KEY AUTOINCREMENT,
                crawl_root_directory_path TEXT NOT NULL,
                started_at_utc TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                finished_at_utc TEXT,
                accessible_entry_count INTEGER NOT NULL DEFAULT 0,
                inaccessible_entry_count INTEGER NOT NULL DEFAULT 0
            )'
        );
        self::executeStatementOrFail(
            $databaseConnection,
            'CREATE TABLE file_system_entries (
                absolute_path TEXT PRIMARY KEY,
                scan_run_identifier INTEGER NOT NULL,
                parent_directory_path TEXT NOT NULL,
                entry_name TEXT NOT NULL,
                entry_type TEXT NOT NULL,
                symbolic_link_target_path TEXT NOT NULL,
                permissions_octal_text TEXT NOT NULL,
                last_write_time_utc_text TEXT NOT NULL,
                last_status_change_time_utc_text TEXT NOT NULL,
                file_size_bytes INTEGER NOT NULL,
                owner_user_identifier INTEGER NOT NULL,
                owner_group_identifier INTEGER NOT NULL,
                inode_number INTEGER NOT NULL,
                device_number INTEGER NOT NULL,
                hard_link_count INTEGER NOT NULL,
                was_accessible INTEGER NOT NULL
            )'
        );
        self::executeStatementOrFail(
            $databaseConnection,
            "INSERT INTO scan_runs (crawl_root_directory_path, accessible_entry_count, inaccessible_entry_count)
             VALUES ('/sample', 2, 0)"
        );
        self::executeStatementOrFail(
            $databaseConnection,
            "INSERT INTO file_system_entries (
                absolute_path, scan_run_identifier, parent_directory_path, entry_name, entry_type,
                symbolic_link_target_path, permissions_octal_text, last_write_time_utc_text,
                last_status_change_time_utc_text, file_size_bytes, owner_user_identifier,
                owner_group_identifier, inode_number, device_number, hard_link_count, was_accessible
             ) VALUES
             ('/sample/alpha.txt', 1, '/sample', 'alpha.txt', 'file', '', '0644', '2026-01-01T00:00:00Z', '2026-01-01T00:00:00Z', 10, 1000, 1000, 1, 1, 1, 1),
             ('/sample/beta', 1, '/sample', 'beta', 'directory', '', '0755', '2026-01-01T00:00:00Z', '2026-01-01T00:00:00Z', 0, 1000, 1000, 2, 1, 1, 1)"
        );

        return $databasePath;
    }

    private static function executeStatementOrFail(PDO $databaseConnection, string $sqlStatement): void
    {
        $executionResult = $databaseConnection->exec($sqlStatement);

        if ($executionResult === false) {
            Assert::fail('A test database SQL statement failed to execute.');
        }
    }
}
