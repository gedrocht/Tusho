<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\ApplicationConfigurationLoader;

final class ApplicationConfigurationLoaderTest extends TestCase
{
    public function testValidConfigurationIsLoadedSuccessfully(): void
    {
        $applicationConfigurationLoader = new ApplicationConfigurationLoader();

        $applicationConfiguration = $applicationConfigurationLoader->load(
            dirname(__DIR__) . '/config/application_configuration.xml',
            dirname(__DIR__) . '/config/application_configuration.xsd',
        );

        self::assertSame('Tusho File Catalog', $applicationConfiguration->siteTitle);
        self::assertSame(50, $applicationConfiguration->defaultPageSize);
    }

    public function testMissingConfigurationFileRaisesHelpfulException(): void
    {
        $applicationConfigurationLoader = new ApplicationConfigurationLoader();

        $this->expectException(\RuntimeException::class);
        $this->expectExceptionMessage('The application configuration XML file does not exist.');

        $applicationConfigurationLoader->load(
            dirname(__DIR__) . '/config/does-not-exist.xml',
            dirname(__DIR__) . '/config/application_configuration.xsd',
        );
    }

    public function testMissingSchemaFileRaisesHelpfulException(): void
    {
        $applicationConfigurationLoader = new ApplicationConfigurationLoader();

        $this->expectException(\RuntimeException::class);
        $this->expectExceptionMessage('The application configuration XML schema file does not exist.');

        $applicationConfigurationLoader->load(
            dirname(__DIR__) . '/config/application_configuration.xml',
            dirname(__DIR__) . '/config/does-not-exist.xsd',
        );
    }

    public function testSchemaValidationFailureRaisesHelpfulException(): void
    {
        $applicationConfigurationLoader = new ApplicationConfigurationLoader();
        $temporaryConfigurationFilePath = tempnam(sys_get_temp_dir(), 'tusho-config-');

        self::assertNotFalse($temporaryConfigurationFilePath);

        file_put_contents(
            $temporaryConfigurationFilePath,
            <<<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<applicationConfiguration>
  <siteTitle>Tusho File Catalog</siteTitle>
  <databasePath>/var/lib/tusho/catalog.sqlite3</databasePath>
  <webLogFilePath>/var/log/tusho/web-application.log</webLogFilePath>
  <defaultPageSize>0</defaultPageSize>
</applicationConfiguration>
XML
        );

        $this->expectException(\RuntimeException::class);
        $this->expectExceptionMessage('The application configuration XML file failed schema validation.');

        try {
            $applicationConfigurationLoader->load(
                $temporaryConfigurationFilePath,
                dirname(__DIR__) . '/config/application_configuration.xsd',
            );
        } finally {
            @unlink($temporaryConfigurationFilePath);
        }
    }
}
