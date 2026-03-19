# Testing and Quality

Tusho is set up so that quality checks are not optional extras.

## C++ quality gates

- compiler warnings as errors
- unit tests
- integration tests
- `clang-format`
- `clang-tidy`
- `cppcheck`
- sanitizer builds
- coverage reporting and thresholds

## PHP quality gates

- `phpunit`
- `phpstan`
- `phpcs`
- PHP syntax validation
- coverage reporting and thresholds

## Repository quality gates

- Markdown linting
- YAML linting
- shell linting
- GitHub Actions linting
- secret scanning
- CodeQL
- dependency review
- OpenSSF Scorecards

## Why this matters

The goal is not simply to have "many checks". The goal is to make the repository resistant to accidental regressions, easy to review, and safe to grow over time.
