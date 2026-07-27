# ADR-0007: Test environment via env-wrapper script

- Status: accepted
- Date: 2026-07-27 (documenting decision of 2026-07-26)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

CTest cannot `source` shell files, so the test environment was initially duplicated into `tests/CMakeLists.txt` via `set_tests_properties(... ENVIRONMENT ...)`. Within a day this duplication produced a real bug: a typo (`COINAPP_DB_PORT:5433`, colon instead of `=`) silently unset the variable, sending the integration test to the wrong container. The `.env` files in `config/` should be the single source of truth — duplicating them invites exactly this class of error.

## Decision

A tiny wrapper script `tests/run_with_env.sh` loads a given env file and then `exec`s the command:

```bash
set -a; source "$ENV_FILE"; set +a; exec "$@"
```

CTest entries invoke it for every DB-dependent suite:

```cmake
add_test(NAME integration
    COMMAND .../run_with_env.sh .../config/.env.test
            $<TARGET_FILE:unit_tests> "[integration]")
```

The `exec` (not a subprocess call) preserves the PID, which later lets the E2E harness kill the spawned app directly (see ADR-0006).

## Consequences

### Positive

- Single source of truth: `config/.env.*`; the typo class is eliminated
- Env file edits take effect immediately — no CMake re-configure
- CMakeLists stays declarative and short
- The same wrapper launches the app in E2E tests and manual dev runs (three uses and counting)

### Negative / Risks

- Requires bash on the test machine (fine for our Linux-only scope)
- One more file that must stay executable (`chmod +x`)

## Alternatives considered

- **ENVIRONMENT property in CMakeLists** (initial approach): duplicated values, caused the typo bug within a day — rejected
- **CMake parses `.env` at configure time**: values go stale until re-configure; fragile parsing — rejected

## References

- ADR-0002 (configuration via environment), ADR-0006 (test strategy)
