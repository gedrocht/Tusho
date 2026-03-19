<?php

declare(strict_types=1);

use TushoWeb\ApplicationConfigurationLoader;
use TushoWeb\FileSystemCatalogRepository;
use TushoWeb\FrontController;
use TushoWeb\HtmlDocumentRenderer;
use TushoWeb\WebRequestLogger;
use TushoWeb\XmlDocumentRenderer;

require dirname(__DIR__, 2) . '/vendor/autoload.php';

$applicationConfigurationLoader = new ApplicationConfigurationLoader();
$applicationConfiguration = $applicationConfigurationLoader->load(
    dirname(__DIR__) . '/config/application_configuration.xml',
    dirname(__DIR__) . '/config/application_configuration.xsd',
);

$fileSystemCatalogRepository = new FileSystemCatalogRepository($applicationConfiguration->databasePath);
$frontController = new FrontController(
    $applicationConfiguration,
    $fileSystemCatalogRepository,
    new HtmlDocumentRenderer(),
    new XmlDocumentRenderer(),
);
$webRequestLogger = new WebRequestLogger($applicationConfiguration->webLogFilePath);

/** @var array<string, string> $queryParameters */
$queryParameters = array_map(
    static fn (mixed $value): string => is_string($value) ? $value : '',
    $_GET,
);

$response = $frontController->handle($queryParameters);

$webRequestLogger->log(
    'The PHP web application served a request.',
    [
        'route' => $queryParameters['route'] ?? 'home',
        'format' => $queryParameters['format'] ?? 'html',
        'query' => $queryParameters['query'] ?? '',
        'directory' => $queryParameters['directory'] ?? '',
    ],
);

header('Content-Type: ' . $response['contentType']);
echo $response['body'];
