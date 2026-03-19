<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\HtmlDocumentRenderer;

final class HtmlDocumentRendererTest extends TestCase
{
    public function testHomePageRendersStatisticsAndRecentRuns(): void
    {
        $htmlDocumentRenderer = new HtmlDocumentRenderer();

        $renderedDocument = $htmlDocumentRenderer->renderHomePage(
            'Tusho <Test>',
            [
                'entryCount' => 2,
                'scanRunCount' => 1,
            ],
            [
                [
                    'scan_run_identifier' => 7,
                    'crawl_root_directory_path' => '/sample',
                    'accessible_entry_count' => 2,
                    'inaccessible_entry_count' => 0,
                ],
            ],
        );

        self::assertStringContainsString('<!DOCTYPE html>', $renderedDocument);
        self::assertStringContainsString('Tusho &lt;Test&gt;', $renderedDocument);
        self::assertStringContainsString('entryCount', $renderedDocument);
        self::assertStringContainsString('/sample', $renderedDocument);
    }

    public function testSearchResultsPageEscapesUserInputAndRendersRows(): void
    {
        $htmlDocumentRenderer = new HtmlDocumentRenderer();

        $renderedDocument = $htmlDocumentRenderer->renderSearchResultsPage(
            'Tusho Test',
            '<alpha>',
            'file',
            [
                [
                    'entry_type' => 'file',
                    'entry_name' => 'alpha.txt',
                    'absolute_path' => '/sample/alpha.txt',
                    'file_size_bytes' => 10,
                    'permissions_octal_text' => '0644',
                    'last_write_time_utc_text' => ['when' => '2026-01-01T00:00:00Z'],
                ],
            ],
        );

        self::assertStringContainsString('&lt;alpha&gt;', $renderedDocument);
        self::assertStringContainsString('/sample/alpha.txt', $renderedDocument);
        self::assertStringContainsString('&quot;when&quot;:&quot;2026-01-01T00:00:00Z&quot;', $renderedDocument);
    }

    public function testDirectoryBrowsePageShowsEmptyStateWhenNoRowsMatch(): void
    {
        $htmlDocumentRenderer = new HtmlDocumentRenderer();

        $renderedDocument = $htmlDocumentRenderer->renderDirectoryBrowsePage(
            'Tusho Test',
            '/empty',
            [],
        );

        self::assertStringContainsString('Directory Browser', $renderedDocument);
        self::assertStringContainsString('No rows matched the request.', $renderedDocument);
    }
}
