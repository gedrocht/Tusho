<?php

declare(strict_types=1);

namespace TushoWeb;

use RuntimeException;
use SimpleXMLElement;

/**
 * Loads and validates the XML configuration file for the PHP web application.
 */
final class ApplicationConfigurationLoader
{
    public function load(string $configurationFilePath, string $schemaFilePath): ApplicationConfiguration
    {
        // The explicit file checks below produce friendlier error messages than the
        // lower-level parser would provide on its own.
        if (!is_file($configurationFilePath)) {
            throw new RuntimeException('The application configuration XML file does not exist.');
        }

        if (!is_file($schemaFilePath)) {
            throw new RuntimeException('The application configuration XML schema file does not exist.');
        }

        // Internal libxml errors let us validate cleanly without dumping parser
        // noise directly into the HTTP response.
        $previousLibxmlState = libxml_use_internal_errors(true);

        $xmlDocument = new \DOMDocument();

        if ($xmlDocument->load($configurationFilePath) === false) {
            libxml_use_internal_errors($previousLibxmlState);
            throw new RuntimeException('The application configuration XML file could not be loaded.');
        }

        if ($xmlDocument->schemaValidate($schemaFilePath) === false) {
            libxml_use_internal_errors($previousLibxmlState);
            throw new RuntimeException('The application configuration XML file failed schema validation.');
        }

        $applicationConfigurationElement = simplexml_load_file($configurationFilePath);
        libxml_use_internal_errors($previousLibxmlState);

        if (!$applicationConfigurationElement instanceof SimpleXMLElement) {
            throw new RuntimeException('The application configuration XML file could not be parsed as SimpleXML.');
        }

        // At this point the XML has already been validated against the schema, so
        // these field extractions are a safe translation step from XML into a
        // strongly shaped PHP value object.
        return new ApplicationConfiguration(
            siteTitle: trim((string) $applicationConfigurationElement->siteTitle),
            databasePath: trim((string) $applicationConfigurationElement->databasePath),
            webLogFilePath: trim((string) $applicationConfigurationElement->webLogFilePath),
            defaultPageSize: (int) $applicationConfigurationElement->defaultPageSize,
        );
    }
}
