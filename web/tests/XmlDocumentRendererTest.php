<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\XmlDocumentRenderer;

final class XmlDocumentRendererTest extends TestCase
{
    public function testSearchResultsDocumentIncludesContextAndEntries(): void
    {
        $xmlDocumentRenderer = new XmlDocumentRenderer();

        $renderedDocument = $xmlDocumentRenderer->renderSearchResults(
            'Tusho Test',
            'alpha',
            'file',
            [
                [
                    'absolute_path' => '/sample/alpha.txt',
                    'entry_name' => 'alpha.txt',
                    'file_size_bytes' => 10,
                    'metadata' => ['kind' => 'demo'],
                ],
            ],
        );

        self::assertStringContainsString('<searchResults>', $renderedDocument);
        self::assertStringContainsString('<query>alpha</query>', $renderedDocument);
        self::assertStringContainsString('<entryTypeFilter>file</entryTypeFilter>', $renderedDocument);
        self::assertStringContainsString('<absolute_path>/sample/alpha.txt</absolute_path>', $renderedDocument);
        self::assertStringContainsString('<metadata>{"kind":"demo"}</metadata>', $renderedDocument);
    }

    public function testDirectoryBrowseDocumentIncludesRequestedDirectory(): void
    {
        $xmlDocumentRenderer = new XmlDocumentRenderer();

        $renderedDocument = $xmlDocumentRenderer->renderDirectoryBrowseResults(
            'Tusho Test',
            '/sample',
            [],
        );

        self::assertStringContainsString('<directoryBrowseResults>', $renderedDocument);
        self::assertStringContainsString('<directoryPath>/sample</directoryPath>', $renderedDocument);
        self::assertStringContainsString('<entries/>', $renderedDocument);
    }
}
