# ADR-0003: Repository pattern for database access

- Status: accepted
- Date: 2026-07-25 (documented retroactively)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

The first vertical slice had SQL queries directly inside HTTP route handlers (`main.cpp`). This caused real bugs: the route queried columns (`title`, `country`, `denomination`, `metak`) that did not exist in the `coins` table — schema drift went unnoticed because routes were untestable without a running database and server.

## Decision

All SQL lives behind **repository classes** (e.g. `coins::CoinRepository`). Repositories:

- take a connection string at construction
- return **domain objects** (`coins::Coin` struct) — never raw pqxx rows
- are the only place in the codebase that knows SQL, table names, and JOINs

Routes stay thin: call repository → map domain objects to template context → render. The HTTP layer does not even include `<pqxx/pqxx>`.

## Consequences

### Positive

- Query logic is integration-testable against the test database
- Schema drift surfaces in exactly one place
- Routes became ~15 lines and declarative
- The domain model (`Coin`) is the explicit contract between DB and UI — it forced the "what fields do we actually show?" conversation

### Negative / Risks

- Extra layer of indirection for trivial CRUD
- One connection per request for now (connection pooling deferred — see arc42 §11)

## Alternatives considered

- **Active Record style** (domain object saves itself): couples domain to DB library, hurts testability
- **ORM**: heavy dependency, hides SQL — counterproductive for learning SQL + modern C++

## References

- PoEAA, "Repository" pattern
