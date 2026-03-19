<?php

declare(strict_types=1);

namespace TushoWebTests;

use PHPUnit\Framework\TestCase;
use TushoWeb\WebRequestLogger;

final class WebRequestLoggerTest extends TestCase
{
    public function testLogWritesStructuredJsonLine(): void
    {
        $temporaryDirectoryPath = sys_get_temp_dir() . DIRECTORY_SEPARATOR . 'tusho-web-log-tests';
        $logFilePath = $temporaryDirectoryPath . DIRECTORY_SEPARATOR . 'requests.jsonl';
        $webRequestLogger = new WebRequestLogger($logFilePath);

        @unlink($logFilePath);
        @rmdir($temporaryDirectoryPath);

        $webRequestLogger->log('Request completed successfully.', [
            'route' => 'search',
            'format' => 'xml',
        ]);

        $logFileContents = file($logFilePath, FILE_IGNORE_NEW_LINES);

        self::assertIsArray($logFileContents);
        self::assertCount(1, $logFileContents);

        $decodedLogPayload = json_decode($logFileContents[0], true, 512, JSON_THROW_ON_ERROR);

        self::assertIsArray($decodedLogPayload);

        self::assertSame('Request completed successfully.', $decodedLogPayload['message']);
        self::assertSame('search', $decodedLogPayload['route']);
        self::assertSame('xml', $decodedLogPayload['format']);

        @unlink($logFilePath);
        @rmdir($temporaryDirectoryPath);
    }

    public function testLogRaisesExceptionWhenParentPathCannotBecomeDirectory(): void
    {
        $blockingFilePath = tempnam(sys_get_temp_dir(), 'tusho-web-log-blocker-');

        self::assertNotFalse($blockingFilePath);

        $webRequestLogger = new WebRequestLogger($blockingFilePath . DIRECTORY_SEPARATOR . 'requests.jsonl');

        $this->expectException(\RuntimeException::class);
        $this->expectExceptionMessage('The web log directory could not be created.');

        try {
            $webRequestLogger->log('This write should fail.');
        } finally {
            @unlink($blockingFilePath);
        }
    }
}
