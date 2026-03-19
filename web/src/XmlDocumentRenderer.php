<?php

declare(strict_types=1);

namespace TushoWeb;

use DOMDocument;

/**
 * Creates machine-readable XML responses for search and browse requests.
 */
final class XmlDocumentRenderer
{
    /**
     * @param list<array<string, mixed>> $searchResults
     */
    public function renderSearchResults(
        string $siteTitle,
        string $searchPhrase,
        string $entryTypeFilter,
        array $searchResults,
    ): string {
        return $this->renderEntryDocument(
            $siteTitle,
            'searchResults',
            [
                'query' => $searchPhrase,
                'entryTypeFilter' => $entryTypeFilter,
            ],
            $searchResults,
        );
    }

    /**
     * @param list<array<string, mixed>> $directoryEntries
     */
    public function renderDirectoryBrowseResults(
        string $siteTitle,
        string $directoryPath,
        array $directoryEntries,
    ): string {
        return $this->renderEntryDocument(
            $siteTitle,
            'directoryBrowseResults',
            [
                'directoryPath' => $directoryPath,
            ],
            $directoryEntries,
        );
    }

    /**
     * @param array<string, string> $contextFields
     * @param list<array<string, mixed>> $entryRows
     */
    private function renderEntryDocument(
        string $siteTitle,
        string $rootElementName,
        array $contextFields,
        array $entryRows,
    ): string {
        $xmlDocument = new DOMDocument('1.0', 'UTF-8');
        $xmlDocument->formatOutput = true;

        $rootElement = $xmlDocument->createElement($rootElementName);
        $rootElement->appendChild($xmlDocument->createElement('siteTitle', $siteTitle));

        foreach ($contextFields as $fieldName => $fieldValue) {
            $rootElement->appendChild($xmlDocument->createElement($fieldName, $fieldValue));
        }

        $entriesElement = $xmlDocument->createElement('entries');

        foreach ($entryRows as $entryRow) {
            $entryElement = $xmlDocument->createElement('entry');

            foreach ($entryRow as $fieldName => $fieldValue) {
                $entryElement->appendChild(
                    $xmlDocument->createElement((string) $fieldName, $this->stringifyValue($fieldValue))
                );
            }

            $entriesElement->appendChild($entryElement);
        }

        $rootElement->appendChild($entriesElement);
        $xmlDocument->appendChild($rootElement);

        return (string) $xmlDocument->saveXML();
    }

    private function stringifyValue(mixed $rawValue): string
    {
        if ($rawValue === null) {
            return '';
        }

        if (is_scalar($rawValue)) {
            return (string) $rawValue;
        }

        return json_encode($rawValue, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE) ?: '';
    }
}
