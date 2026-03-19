<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\FileSystemCatalogRepository;
use TushoWebTests\Support\TestDatabaseFactory;

final class FileSystemCatalogRepositoryTest extends TestCase
{
    public function testFetchCatalogStatisticsReturnsExpectedCounts(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $fileSystemCatalogRepository = new FileSystemCatalogRepository($databasePath);

        $catalogStatistics = $fileSystemCatalogRepository->fetchCatalogStatistics();

        self::assertSame(
            [
                'entryCount' => 2,
                'scanRunCount' => 1,
                'directoryCount' => 1,
                'fileCount' => 1,
            ],
            $catalogStatistics,
        );
    }

    public function testFetchRecentScanRunsReturnsNewestRowsFirst(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $fileSystemCatalogRepository = new FileSystemCatalogRepository($databasePath);

        $recentScanRuns = $fileSystemCatalogRepository->fetchRecentScanRuns(10);

        self::assertCount(1, $recentScanRuns);
        self::assertSame('/sample', $recentScanRuns[0]['crawl_root_directory_path']);
        self::assertSame(2, $recentScanRuns[0]['accessible_entry_count']);
    }

    public function testSearchEntriesReturnsMatchingRows(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $fileSystemCatalogRepository = new FileSystemCatalogRepository($databasePath);

        $searchResults = $fileSystemCatalogRepository->searchEntries('alpha', 'file', 10);

        self::assertCount(1, $searchResults);
        self::assertSame('/sample/alpha.txt', $searchResults[0]['absolute_path']);
    }

    public function testBrowseDirectoryReturnsChildrenOfRequestedDirectory(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $fileSystemCatalogRepository = new FileSystemCatalogRepository($databasePath);

        $directoryEntries = $fileSystemCatalogRepository->browseDirectory('/sample', 10);

        self::assertCount(2, $directoryEntries);
    }

    public function testSearchEntriesWithoutFiltersCanReturnMultipleRows(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $fileSystemCatalogRepository = new FileSystemCatalogRepository($databasePath);

        $searchResults = $fileSystemCatalogRepository->searchEntries('', '', 10);

        self::assertCount(2, $searchResults);
    }
}
