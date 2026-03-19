<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\ApplicationConfiguration;
use TushoWeb\FileSystemCatalogRepository;
use TushoWeb\FrontController;
use TushoWeb\HtmlDocumentRenderer;
use TushoWeb\XmlDocumentRenderer;
use TushoWebTests\Support\TestDatabaseFactory;

final class FrontControllerTest extends TestCase
{
    public function testHomeRouteRendersHtmlDashboard(): void
    {
        $frontController = $this->createFrontController();

        $response = $frontController->handle([]);

        self::assertSame('text/html; charset=UTF-8', $response['contentType']);
        self::assertStringContainsString('Catalog Statistics', $response['body']);
        self::assertStringContainsString('Recent Scan Runs', $response['body']);
    }

    public function testSearchRouteCanRenderHtml(): void
    {
        $frontController = $this->createFrontController();

        $response = $frontController->handle([
            'route' => 'search',
            'query' => 'alpha',
            'entry_type' => 'file',
        ]);

        self::assertSame('text/html; charset=UTF-8', $response['contentType']);
        self::assertStringContainsString('Search Results', $response['body']);
        self::assertStringContainsString('/sample/alpha.txt', $response['body']);
    }

    public function testSearchRouteCanRenderXml(): void
    {
        $frontController = $this->createFrontController();

        $response = $frontController->handle([
            'route' => 'search',
            'format' => 'xml',
            'query' => 'alpha',
            'entry_type' => 'file',
        ]);

        self::assertSame('application/xml; charset=UTF-8', $response['contentType']);
        self::assertStringContainsString('<searchResults>', $response['body']);
        self::assertStringContainsString('/sample/alpha.txt', $response['body']);
    }

    public function testBrowseRouteCanRenderHtml(): void
    {
        $frontController = $this->createFrontController();

        $response = $frontController->handle([
            'route' => 'browse',
            'directory' => '/sample',
        ]);

        self::assertSame('text/html; charset=UTF-8', $response['contentType']);
        self::assertStringContainsString('Directory Browser', $response['body']);
        self::assertStringContainsString('alpha.txt', $response['body']);
    }

    public function testBrowseRouteCanRenderXml(): void
    {
        $frontController = $this->createFrontController();

        $response = $frontController->handle([
            'route' => 'browse',
            'format' => 'xml',
            'directory' => '/sample',
        ]);

        self::assertSame('application/xml; charset=UTF-8', $response['contentType']);
        self::assertStringContainsString('<directoryBrowseResults>', $response['body']);
        self::assertStringContainsString('<directoryPath>/sample</directoryPath>', $response['body']);
    }

    private function createFrontController(): FrontController
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();

        return new FrontController(
            new ApplicationConfiguration('Tusho Test', $databasePath, '/tmp/tusho-web.log', 25),
            new FileSystemCatalogRepository($databasePath),
            new HtmlDocumentRenderer(),
            new XmlDocumentRenderer(),
        );
    }
}
