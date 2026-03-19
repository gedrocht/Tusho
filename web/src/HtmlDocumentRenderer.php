<?php

declare(strict_types=1);

namespace TushoWeb;

/**
 * Creates human-friendly HTML pages for browsing the file catalog.
 */
final class HtmlDocumentRenderer
{
    /**
     * @param array<string, int> $catalogStatistics
     * @param list<array<string, mixed>> $recentScanRuns
     */
    public function renderHomePage(string $siteTitle, array $catalogStatistics, array $recentScanRuns): string
    {
        $statisticsListMarkup = '';

        foreach ($catalogStatistics as $statisticLabel => $statisticValue) {
            $statisticsListMarkup .= '<li><strong>'
                . $this->escape($statisticLabel)
                . ':</strong> '
                . $this->escape((string) $statisticValue)
                . '</li>';
        }

        $recentScanRowsMarkup = '';

        foreach ($recentScanRuns as $recentScanRun) {
            $recentScanRowsMarkup .= $this->renderRecentScanRow($recentScanRun);
        }

        return $this->wrapInDocument(
            $siteTitle,
            implode(
                '',
                [
                    '<section><h1>',
                    $this->escape($siteTitle),
                    '</h1><p>This interface lets you search and browse the Linux file catalog created by the ',
                    'C++ crawler.</p></section>',
                    '<section><h2>Quick Search</h2>',
                    '<form method="get" action="index.php">',
                    '<input type="hidden" name="route" value="search"/>',
                    '<label for="query">Search phrase</label>',
                    '<input id="query" name="query" type="text" placeholder="example.txt"/>',
                    '<label for="entry_type">Entry type</label>',
                    '<select id="entry_type" name="entry_type">',
                    '<option value="">Any</option>',
                    '<option value="file">File</option>',
                    '<option value="directory">Directory</option>',
                    '<option value="symbolic_link">Symbolic link</option>',
                    '</select>',
                    '<button type="submit">Search catalog</button>',
                    '</form></section>',
                    '<section><h2>Catalog Statistics</h2><ul>',
                    $statisticsListMarkup,
                    '</ul></section>',
                    '<section><h2>Recent Scan Runs</h2><table><thead><tr>',
                    '<th>Identifier</th><th>Root</th><th>Accessible entries</th><th>Inaccessible entries</th>',
                    '</tr></thead><tbody>',
                    $recentScanRowsMarkup,
                    '</tbody></table></section>',
                ]
            )
        );
    }

    /**
     * @param list<array<string, mixed>> $searchResults
     */
    public function renderSearchResultsPage(
        string $siteTitle,
        string $searchPhrase,
        string $entryTypeFilter,
        array $searchResults,
    ): string {
        return $this->wrapInDocument(
            $siteTitle,
            implode(
                '',
                [
                    '<section><h1>Search Results</h1><p><strong>Query:</strong> ',
                    $this->escape($searchPhrase),
                    '</p><p><strong>Entry type filter:</strong> ',
                    $this->escape($entryTypeFilter === '' ? 'Any' : $entryTypeFilter),
                    '</p></section>',
                    $this->renderEntryTable($searchResults),
                ]
            )
        );
    }

    /**
     * @param list<array<string, mixed>> $directoryEntries
     */
    public function renderDirectoryBrowsePage(
        string $siteTitle,
        string $directoryPath,
        array $directoryEntries,
    ): string {
        return $this->wrapInDocument(
            $siteTitle,
            implode(
                '',
                [
                    '<section><h1>Directory Browser</h1><p><strong>Directory:</strong> ',
                    $this->escape($directoryPath),
                    '</p></section>',
                    $this->renderEntryTable($directoryEntries),
                ]
            )
        );
    }

    /**
     * @param list<array<string, mixed>> $entryRows
     */
    private function renderEntryTable(array $entryRows): string
    {
        $tableRowsMarkup = '';

        foreach ($entryRows as $entryRow) {
            $tableRowsMarkup .= $this->renderEntryRow($entryRow);
        }

        if ($tableRowsMarkup === '') {
            $tableRowsMarkup = '<tr><td colspan="6">No rows matched the request.</td></tr>';
        }

        return implode(
            '',
            [
                '<section><table><thead><tr>',
                '<th>Type</th><th>Name</th><th>Absolute path</th><th>Size (bytes)</th>',
                '<th>Permissions</th><th>Last write time</th>',
                '</tr></thead><tbody>',
                $tableRowsMarkup,
                '</tbody></table></section>',
            ]
        );
    }

    private function wrapInDocument(string $siteTitle, string $bodyMarkup): string
    {
        return implode(
            '',
            [
                '<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"/>',
                '<meta name="viewport" content="width=device-width, initial-scale=1"/>',
                '<title>',
                $this->escape($siteTitle),
                '</title><link rel="stylesheet" href="assets/application.css"/></head><body><main>',
                '<nav><a href="index.php">Home</a> | ',
                '<a href="index.php?route=browse&amp;directory=/">Browse root</a> | ',
                '<a href="index.php?route=search&amp;format=xml">XML search endpoint</a></nav>',
                $bodyMarkup,
                '</main></body></html>',
            ]
        );
    }

    /**
     * @param array<string, mixed> $recentScanRun
     */
    private function renderRecentScanRow(array $recentScanRun): string
    {
        return implode(
            '',
            [
                '<tr><td>',
                $this->escape($this->stringifyValue($recentScanRun['scan_run_identifier'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($recentScanRun['crawl_root_directory_path'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($recentScanRun['accessible_entry_count'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($recentScanRun['inaccessible_entry_count'] ?? null)),
                '</td></tr>',
            ]
        );
    }

    /**
     * @param array<string, mixed> $entryRow
     */
    private function renderEntryRow(array $entryRow): string
    {
        return implode(
            '',
            [
                '<tr><td>',
                $this->escape($this->stringifyValue($entryRow['entry_type'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($entryRow['entry_name'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($entryRow['absolute_path'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($entryRow['file_size_bytes'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($entryRow['permissions_octal_text'] ?? null)),
                '</td><td>',
                $this->escape($this->stringifyValue($entryRow['last_write_time_utc_text'] ?? null)),
                '</td></tr>',
            ]
        );
    }

    private function stringifyValue(mixed $rawValue): string
    {
        if ($rawValue === null) {
            return '';
        }

        if (is_scalar($rawValue)) {
            return (string) $rawValue;
        }

        return json_encode($rawValue, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE) ?: '';
    }

    private function escape(string $rawText): string
    {
        return htmlspecialchars($rawText, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
    }
}
