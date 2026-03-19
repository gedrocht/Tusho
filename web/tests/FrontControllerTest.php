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
    public function testSearchRouteCanRenderXml(): void
    {
        $databasePath = TestDatabaseFactory::createTemporaryDatabase();
        $frontController = new FrontController(
            new ApplicationConfiguration('Tusho Test', $databasePath, '/tmp/tusho-web.log', 25),
            new FileSystemCatalogRepository($databasePath),
            new HtmlDocumentRenderer(),
            new XmlDocumentRenderer(),
        );

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
}
