<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\FileSystemCatalogRepository;
use TushoWebTests\Support\TestDatabaseFactory;

final class FileSystemCatalogRepositoryTest extends TestCase
{
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
}
