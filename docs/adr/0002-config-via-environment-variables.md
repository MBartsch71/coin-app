# ADR-0002: Configuration via environment variables

- Status: accepted
- Date: 2026-07-25 (documented retroactively)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

The app needs database credentials and runtime settings that differ between environments (dev / test / prod). Hardcoding config into source couples the binary to one environment and risks committing secrets to git.

## Decision

All configuration is read from **environment variables** (12-factor style) through a single, tested component:

- `database::EnvironmentLoader::load()` reads `COINAPP_DB_HOST`, `COINAPP_DB_PORT`, `COINAPP_DB_NAME`, `COINAPP_DB_USER`, `COINAPP_DB_PASSWORD` (and `COINAPP_PORT` for the web port), each with a development-safe default
- Per-environment `.env` files live in `config/` (`.env.dev`, `.env.test`, `.env.prod`); `.env` itself is gitignored
- The **same binary** runs in every environment — only the environment differs

## Consequences

### Positive

- One build artifact promotes from dev → test → prod unchanged
- Secrets stay out of version control
- Defaults make a fresh checkout runnable with zero setup
- Config logic is unit-testable (see `tests/test_db_config.cpp`)

### Negative / Risks

- Every new setting must be added in three places: loader, defaults, tests (discipline required)
- `.env.example` must be kept in sync manually

## Alternatives considered

- **Config file parsed at runtime** (JSON/YAML): adds a parser dependency and file-path problems; env vars are the deployment-native mechanism on Linux/systemd
- **Compile-time config**: would require rebuilding per environment — violates "build once, run anywhere"

## References

- https://12factor.net/config
