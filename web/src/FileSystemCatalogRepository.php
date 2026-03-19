<?php

declare(strict_types=1);

namespace TushoWeb;

use PDO;
use PDOStatement;

/**
 * Encapsulates all read-only database queries used by the PHP web application.
 */
final class FileSystemCatalogRepository
{
    private PDO $databaseConnection;

    public function __construct(string $databasePath)
    {
        $this->databaseConnection = new PDO('sqlite:' . $databasePath);
        $this->databaseConnection->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $this->databaseConnection->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
    }

    /**
     * @return array<string, int>
     */
    public function fetchCatalogStatistics(): array
    {
        return [
            'entryCount' => $this->fetchSingleInteger('SELECT COUNT(*) FROM file_system_entries'),
            'scanRunCount' => $this->fetchSingleInteger('SELECT COUNT(*) FROM scan_runs'),
            'directoryCount' => $this->fetchSingleInteger(
                "SELECT COUNT(*) FROM file_system_entries WHERE entry_type = 'directory'"
            ),
            'fileCount' => $this->fetchSingleInteger(
                "SELECT COUNT(*) FROM file_system_entries WHERE entry_type = 'file'"
            ),
        ];
    }

    /**
     * @return list<array<string, mixed>>
     */
    public function fetchRecentScanRuns(int $maximumRowCount): array
    {
        $preparedStatement = $this->prepareStatementOrFail(
            'SELECT scan_run_identifier, crawl_root_directory_path, started_at_utc, '
            . 'finished_at_utc, accessible_entry_count, inaccessible_entry_count '
            . 'FROM scan_runs '
            . 'ORDER BY scan_run_identifier DESC '
            . 'LIMIT :maximumRowCount'
        );

        $preparedStatement->bindValue(':maximumRowCount', $maximumRowCount, PDO::PARAM_INT);
        $preparedStatement->execute();

        /** @var list<array<string, mixed>> $rows */
        $rows = $preparedStatement->fetchAll();
        return $rows;
    }

    /**
     * @return list<array<string, mixed>>
     */
    public function searchEntries(string $searchPhrase, string $entryTypeFilter, int $maximumRowCount): array
    {
        $preparedStatement = $this->prepareStatementOrFail(
            'SELECT absolute_path, parent_directory_path, entry_name, entry_type, '
            . 'file_size_bytes, permissions_octal_text, last_write_time_utc_text '
            . 'FROM file_system_entries '
            . 'WHERE (:entryTypeFilter = "" OR entry_type = :entryTypeFilter) '
            . 'AND ('
            . '     :searchPhrase = "" '
            . '     OR absolute_path LIKE :searchPattern '
            . '     OR entry_name LIKE :searchPattern'
            . ') '
            . 'ORDER BY absolute_path ASC '
            . 'LIMIT :maximumRowCount'
        );

        return $this->executeCatalogQuery(
            $preparedStatement,
            $searchPhrase,
            $entryTypeFilter,
            $maximumRowCount,
        );
    }

    /**
     * @return list<array<string, mixed>>
     */
    public function browseDirectory(string $directoryPath, int $maximumRowCount): array
    {
        $preparedStatement = $this->prepareStatementOrFail(
            'SELECT absolute_path, parent_directory_path, entry_name, entry_type, '
            . 'file_size_bytes, permissions_octal_text, last_write_time_utc_text '
            . 'FROM file_system_entries '
            . 'WHERE parent_directory_path = :directoryPath '
            . 'ORDER BY entry_type DESC, entry_name ASC '
            . 'LIMIT :maximumRowCount'
        );

        $preparedStatement->bindValue(':directoryPath', $directoryPath);
        $preparedStatement->bindValue(':maximumRowCount', $maximumRowCount, PDO::PARAM_INT);
        $preparedStatement->execute();

        /** @var list<array<string, mixed>> $rows */
        $rows = $preparedStatement->fetchAll();
        return $rows;
    }

    private function fetchSingleInteger(string $sqlStatement): int
    {
        $queryResult = $this->databaseConnection->query($sqlStatement);
        $scalarValue = $queryResult === false ? false : $queryResult->fetchColumn();

        return $scalarValue === false ? 0 : (int) $scalarValue;
    }

    /**
     * @return list<array<string, mixed>>
     */
    private function executeCatalogQuery(
        PDOStatement $preparedStatement,
        string $searchPhrase,
        string $entryTypeFilter,
        int $maximumRowCount,
    ): array {
        $preparedStatement->bindValue(':searchPhrase', $searchPhrase);
        $preparedStatement->bindValue(':searchPattern', '%' . $searchPhrase . '%');
        $preparedStatement->bindValue(':entryTypeFilter', $entryTypeFilter);
        $preparedStatement->bindValue(':maximumRowCount', $maximumRowCount, PDO::PARAM_INT);
        $preparedStatement->execute();

        /** @var list<array<string, mixed>> $rows */
        $rows = $preparedStatement->fetchAll();
        return $rows;
    }

    private function prepareStatementOrFail(string $sqlStatement): PDOStatement
    {
        $preparedStatement = $this->databaseConnection->prepare($sqlStatement);

        if (!$preparedStatement instanceof PDOStatement) {
            throw new \RuntimeException('The PDO layer could not prepare a SQL statement.');
        }

        return $preparedStatement;
    }
}
