# ADR-0006: Test strategy — unit, integration, end-to-end

- Status: accepted
- Date: 2026-07-25
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

Matthias is a TDD evangelist; the project follows red → green → refactor for every change. Different bug classes need different test levels: config logic needs fast isolated tests, SQL needs a real database, and feature behavior needs the full running stack.

## Decision

A three-level pyramid, all run via Catch2 + CTest:

1. **Unit tests** — pure logic (config loader, domain mapping). No I/O. Milliseconds.
2. **Integration tests** — repositories against the real test database (`coin_catalog_test` in its own container). Fixtures use `TEST_`-tagged rows with cleanup **before and after** each test (self-healing; a transaction-rollback approach cannot work across two connections).
3. **End-to-end tests** — the real compiled binary spawned as a subprocess (test port, test env vars), real HTTP requests via the **cpr** library, assertions on status codes and rendered HTML. Each test run seeds and wipes the test database.

**Every feature starts outside-in**: a failing E2E test describes the user-visible behavior, then drives repository and route implementation until green. Commit only on green.

## Consequences

### Positive

- E2E tests catch the bug class that actually hurt us (route/schema drift, template misconfiguration)
- The test suite is the deployment gate: only all-green increments are promoted to prod
- Tests document intended behavior better than comments

### Negative / Risks

- E2E tests are slow (~1–2 s each) and need process lifecycle management
- Fixture discipline is manual; sloppy fixtures pollute the test DB

## Alternatives considered

- **Browser-based E2E** (Selenium/Playwright): unnecessary — server-rendered HTML means behavior is fully observable over HTTP
- **In-process server for E2E**: faster, but would not test the real binary + real env wiring

## References

- ADR-0005 (environment isolation), ADR-0003 (repository pattern)
