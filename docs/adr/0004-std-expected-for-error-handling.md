# ADR-0004: std::expected for error handling

- Status: accepted
- Date: 2026-07-25 (documented retroactively)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

libpqxx reports all failures (connection loss, SQL errors) via exceptions. Exceptions for *expected* failure modes (database down, bad query) make error paths invisible and easy to forget — the original `/coins` route wrapped everything in a generic `try/catch` that returned a 500 with a raw exception message.

## Decision

Repository methods return `std::expected<T, CoinRepositoryError>` (C++23, available in our C++26 toolchain). The repository is the **exception boundary**: pqxx exceptions are caught there and translated into typed error values:

```cpp
catch (const pqxx::broken_connection&) {
    return std::unexpected(CoinRepositoryError::ConnectionFailed);
}
catch (const std::exception&) {
    return std::unexpected(CoinRepositoryError::QueryFailed);
}
```

Callers handle errors explicitly via `if (!result)` + `switch (result.error())` — mapped to semantically correct HTTP codes (503 vs 500).

## Consequences

### Positive

- Error paths are visible in the type signature — the compiler forces handling
- `switch` over `enum class` warns on unhandled cases when new errors are added (free refactoring safety net)
- No hidden control flow across layers
- Tests assert on error values without `REQUIRE_THROWS` gymnastics

### Negative / Risks

- Slightly more verbose call sites
- Error enum granularity must grow deliberately (avoid one `Unknown` bucket)

## Alternatives considered

- **Exceptions across layers**: invisible error paths; rejected
- **`std::optional` + error log**: loses *why* it failed; rejected
- **Outcome / Boost.Leaf**: extra dependency; `std::expected` is standard

## References

- https://en.cppreference.com/w/cpp/utility/expected
