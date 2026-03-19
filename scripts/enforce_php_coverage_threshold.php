<?php

declare(strict_types=1);

if ($argc < 3) {
    fwrite(STDERR, "Usage: php scripts/enforce_php_coverage_threshold.php <clover-file> <minimum-percentage>\n");
    exit(1);
}

$coverageFilePath = $argv[1];
$minimumCoveragePercentage = (float) $argv[2];

if (!is_file($coverageFilePath)) {
    fwrite(STDERR, "Coverage file not found: {$coverageFilePath}\n");
    exit(1);
}

$coverageDocument = simplexml_load_file($coverageFilePath);

if ($coverageDocument === false) {
    fwrite(STDERR, "Coverage file could not be parsed.\n");
    exit(1);
}

$metricsNodes = $coverageDocument->xpath('//metrics');

if ($metricsNodes === false || $metricsNodes === []) {
    fwrite(STDERR, "Coverage metrics were not found in the Clover report.\n");
    exit(1);
}

$aggregateMetricsNode = end($metricsNodes);
$coveredStatements = (float) ($aggregateMetricsNode['coveredstatements'] ?? 0);
$totalStatements = (float) ($aggregateMetricsNode['statements'] ?? 0);

if ($totalStatements <= 0.0) {
    fwrite(STDERR, "Coverage report does not contain any measured statements.\n");
    exit(1);
}

$actualCoveragePercentage = ($coveredStatements / $totalStatements) * 100.0;

fwrite(STDOUT, sprintf("PHP statement coverage: %.2f%%\n", $actualCoveragePercentage));

if ($actualCoveragePercentage < $minimumCoveragePercentage) {
    fwrite(
        STDERR,
        sprintf(
            "Coverage threshold failure: expected at least %.2f%% but observed %.2f%%\n",
            $minimumCoveragePercentage,
            $actualCoveragePercentage,
        ),
    );
    exit(1);
}
