# Tusho

This repository is bootstrapped with a GitHub-first quality and security baseline.

## What is included

- Strict repository hygiene defaults
- Pull request and issue templates
- Security policy and CODEOWNERS
- Dependabot automation
- GitHub Actions for linting, workflow validation, dependency review, code scanning, secret scanning, and repository score checks

## Quality posture

The CI configuration is designed to fail fast on:

- malformed workflows or insecure GitHub Actions patterns
- vulnerable or risky dependency changes in pull requests
- leaked secrets
- common repository hygiene regressions in Markdown, YAML, JSON, and shell snippets
- GitHub Actions supply chain risks

## Next step

When application code is added, extend CI with language-specific jobs for unit tests, integration tests, and coverage thresholds.
