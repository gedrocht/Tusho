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
}
