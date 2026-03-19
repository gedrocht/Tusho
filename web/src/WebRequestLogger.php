<?php

declare(strict_types=1);

namespace TushoWeb;

/**
 * Writes simple newline-delimited JSON request logs for the PHP interface.
 */
final class WebRequestLogger
{
    public function __construct(private readonly string $logFilePath)
    {
    }

    /**
     * @param array<string, string> $contextFields
     */
    public function log(string $message, array $contextFields = []): void
    {
        $logDirectoryPath = dirname($this->logFilePath);
        $directoryAlreadyExists = is_dir($logDirectoryPath);
        $directoryCreationSucceeded = false;

        if (!$directoryAlreadyExists) {
            set_error_handler(static function (): bool {
                return true;
            });

            try {
                $directoryCreationSucceeded = mkdir($logDirectoryPath, 0775, true);
            } finally {
                restore_error_handler();
            }
        }

        if (!$directoryAlreadyExists && !$directoryCreationSucceeded && !is_dir($logDirectoryPath)) {
            throw new \RuntimeException('The web log directory could not be created.');
        }

        $logPayload = [
            'timestamp' => gmdate('c'),
            'message' => $message,
        ];

        foreach ($contextFields as $fieldName => $fieldValue) {
            $logPayload[$fieldName] = $fieldValue;
        }

        $encodedLogPayload = json_encode($logPayload, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);

        if ($encodedLogPayload === false) {
            throw new \RuntimeException('The web log payload could not be encoded as JSON.');
        }

        $bytesWritten = file_put_contents(
            $this->logFilePath,
            $encodedLogPayload . PHP_EOL,
            FILE_APPEND | LOCK_EX,
        );

        if ($bytesWritten === false) {
            throw new \RuntimeException('The web log payload could not be written to disk.');
        }
    }
}
