# ADR-0009: Purchase price is a required fact (0.00 = gift or unknown)

- Status: accepted
- Date: 2026-07-27
- Deciders: Matthias

## Context

The schema allowed `purchase_price` to be NULL. When modeling the `Coin` struct we faced the question: is price `std::optional<double>` ("unknown" is distinct from "0") or a plain `double`? Matthias ruled as domain owner: a price is **always recorded**; when nothing was paid (gift) or the price is not known, it is entered as `0.00`. The ambiguity between "gift" and "unknown" is consciously accepted — both mean "no money spent that I need to track".

## Decision

- `Coin::purchase_price` is a plain `double`, never optional
- Domain rule: **0.00 = nothing paid or unknown**; real prices are > 0
- Schema enforces the rule (migration `02_purchase_price_required.sql`): existing NULLs become `0.00`, column becomes `NOT NULL DEFAULT 0.00`
- The repository may therefore rely on a non-NULL price

## Consequences

### Positive

- Simple, honest type — no optional handling for a field the owner always records
- Schema enforces the discipline, not just convention
- Input forms can require the field (default 0.00)

### Negative / Risks

- "Gift" and "forgot to record" are indistinguishable in the data (accepted by the owner)

## Alternatives considered

- **`std::optional<double>`** (NULL = unknown, 0 = gift): preserves the distinction, but adds optional handling everywhere for a case the owner does not want to model — rejected by domain decision

## References

- ADR-0003 (repository pattern), migration `src/database/migrations/02_purchase_price_required.sql`
