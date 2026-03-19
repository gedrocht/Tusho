<?php

declare(strict_types=1);

namespace TushoWeb;

/**
 * Routes incoming HTTP requests to the correct query and renderer.
 */
final class FrontController
{
    public function __construct(
        private readonly ApplicationConfiguration $applicationConfiguration,
        private readonly FileSystemCatalogRepository $fileSystemCatalogRepository,
        private readonly HtmlDocumentRenderer $htmlDocumentRenderer,
        private readonly XmlDocumentRenderer $xmlDocumentRenderer,
    ) {
    }

    /**
     * @param array<string, string> $queryParameters
     * @return array{contentType: string, body: string}
     */
    public function handle(array $queryParameters): array
    {
        $routeName = $queryParameters['route'] ?? 'home';
        $responseFormat = $queryParameters['format'] ?? 'html';
        $maximumRowCount = $this->applicationConfiguration->defaultPageSize;

        if ($routeName === 'search') {
            $searchPhrase = trim($queryParameters['query'] ?? '');
            $entryTypeFilter = trim($queryParameters['entry_type'] ?? '');
            $searchResults = $this->fileSystemCatalogRepository->searchEntries(
                $searchPhrase,
                $entryTypeFilter,
                $maximumRowCount,
            );

            return $responseFormat === 'xml'
                ? [
                    'contentType' => 'application/xml; charset=UTF-8',
                    'body' => $this->xmlDocumentRenderer->renderSearchResults(
                        $this->applicationConfiguration->siteTitle,
                        $searchPhrase,
                        $entryTypeFilter,
                        $searchResults,
                    ),
                ]
                : [
                    'contentType' => 'text/html; charset=UTF-8',
                    'body' => $this->htmlDocumentRenderer->renderSearchResultsPage(
                        $this->applicationConfiguration->siteTitle,
                        $searchPhrase,
                        $entryTypeFilter,
                        $searchResults,
                    ),
                ];
        }

        if ($routeName === 'browse') {
            $directoryPath = trim($queryParameters['directory'] ?? '/');
            $directoryEntries = $this->fileSystemCatalogRepository->browseDirectory($directoryPath, $maximumRowCount);

            return $responseFormat === 'xml'
                ? [
                    'contentType' => 'application/xml; charset=UTF-8',
                    'body' => $this->xmlDocumentRenderer->renderDirectoryBrowseResults(
                        $this->applicationConfiguration->siteTitle,
                        $directoryPath,
                        $directoryEntries,
                    ),
                ]
                : [
                    'contentType' => 'text/html; charset=UTF-8',
                    'body' => $this->htmlDocumentRenderer->renderDirectoryBrowsePage(
                        $this->applicationConfiguration->siteTitle,
                        $directoryPath,
                        $directoryEntries,
                    ),
                ];
        }

        $catalogStatistics = $this->fileSystemCatalogRepository->fetchCatalogStatistics();
        $recentScanRuns = $this->fileSystemCatalogRepository->fetchRecentScanRuns(10);

        return [
            'contentType' => 'text/html; charset=UTF-8',
            'body' => $this->htmlDocumentRenderer->renderHomePage(
                $this->applicationConfiguration->siteTitle,
                $catalogStatistics,
                $recentScanRuns,
            ),
        ];
    }
}
