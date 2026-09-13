# ADR-0012: Form/API field contract and error taxonomy

- Status: accepted
- Date: 2026-09-13
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

While building the add-coin flow, three related contract problems surfaced in production:

1. **Field naming drift** — form, tests, route, and `NewCoinData` used inconsistent names (`new_title` vs `ref_title`, a stray `new_dealer`), caught only by manual testing.
2. **Empty string vs. absent** — browsers submit untouched fields as empty strings (`reference_id=""`, `ref_metal=""`). `std::stoll("")` threw (400 "Invalid reference_id"); `''::metal_type` was rejected by PostgreSQL (500).
3. **Error category confusion** — contract violations (incomplete client data) and database failures both mapped to `QueryFailed`/500, lying about whose fault the failure was and hiding the real exception (30 minutes of debugging until `std::cerr` logging was added to the catch block).

## Decision

**Field contract (one rule):** form/API field names are prefixed `ref_*` for catalog-reference data (`ref_title`, `ref_country`, `ref_metal`) and bare for collection-item data (`year`, `quantity`, `purchase_price`, `purchase_currency`, `dealer`). The same names are used identically in: HTML form, E2E test payloads, route parsing, and `NewCoinData` members. The E2E test is the executable contract.

**Boundary normalization:** empty strings are converted to `std::nullopt` at the route boundary (`get_nonempty` helper) — "empty" and "absent" are the same thing semantically.

**Error taxonomy (three categories):**

| Category | Meaning | HTTP |
|---|---|---|
| `ConnectionFailed` | database unreachable | 503 |
| `InvalidData` | caller violated the contract (validated BEFORE touching the DB) | 400 |
| `QueryFailed` | database rejected a well-formed operation | 500 |

Repositories validate the contract first (guard clause: existing `reference_id` OR all three `ref_*` fields — `!(a && b && c)`), and log the real exception (`std::cerr` → journald) before translating to an error category.

## Consequences

### Positive
- The E2E suite catches contract drift between form/route/repository automatically.
- Empty-field edge cases are handled once, at the boundary, not in every consumer.
- Failures are self-explaining: the journal contains the real PostgreSQL error; the client gets an honest status code.
- `switch` exhaustiveness warnings (-Wall) force every route to consciously handle new error categories as the enum grows.

### Negative
- Slightly more enum/switch boilerplate per route.

## Pitfalls recorded (for future reference)

- `!(a && b && c)` vs `!a && !b && !c` — the De Morgan trap: the first rejects on ANY missing field, the second only when ALL are missing. A stray `!` turned the whole pyramid red; the integration test caught it instantly.
- Browser-only behaviors (placeholder `<option>` submitting `""`) are invisible to HTTP-level tests that omit the field entirely — E2E tests must include the "empty string" submission shapes, not just "absent field" shapes.

## Alternatives considered

- Bean-validation-style per-field error responses — overkill for a single-form app at this stage; 400 + message suffices.
- Letting the database constraint reject bad data and mapping SQLSTATE codes to statuses — precise but couples the HTTP layer to PostgreSQL error internals; the pre-DB guard is simpler and faster.
