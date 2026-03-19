<?php

declare(strict_types=1);

namespace TushoWeb;

/**
 * Represents the validated XML configuration consumed by the PHP application.
 */
final class ApplicationConfiguration
{
    public function __construct(
        public readonly string $siteTitle,
        public readonly string $databasePath,
        public readonly string $webLogFilePath,
        public readonly int $defaultPageSize,
    ) {
    }
}
