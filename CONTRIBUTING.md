# Contributing

## Development principles

- Prefer clarity over cleverness.
- Use self-descriptive names instead of abbreviations.
- Keep beginner readability in mind when writing code comments and docs.
- Treat warnings as problems to fix, not as background noise.
- Add or update tests whenever behavior changes.

## Local validation

Before opening a pull request, aim to run:

```bash
cmake -S . -B build -G Ninja -DTUSHO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
composer install
composer run lint:php
composer run analyse:php
composer run test:php
```

## Pull request expectations

- Explain the purpose of the change in plain language.
- Call out security, operational, and performance impacts.
- Add examples or docs for user-facing behavior.
- Keep commits logically grouped.
- Do not merge code that bypasses the quality gates.
