# ADR-0011: Post/Redirect/Get (PRG) for all form submissions

- Status: accepted
- Date: 2026-09-13 (documenting pattern established with Phase 3)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

Phase 3 introduced the first write path (`POST /coins`). A POST that renders a result page directly creates classic problems: refreshing the page re-submits the form (duplicate inserts), the Back button warns "resubmit form data?", and the URL in the address bar doesn't match the displayed content.

## Decision

All successful form-handling POSTs end with a **303 See Other** redirect to a GET route:

```
POST /coins  →  303  →  GET /coins
```

- **303 specifically** (not 301/302): it unambiguously instructs the client to follow with a GET, regardless of the original method.
- The POST itself renders nothing; it validates, mutates, and redirects.
- Errors do NOT redirect: 4xx/5xx responses render inline so the user sees the problem in context.

## Consequences

### Positive
- Refresh after submit is safe (re-GETs the list, never re-POSTs).
- Back button behaves naturally.
- URL always matches the rendered page — bookmarkable, shareable.

### Negative
- One extra HTTP round-trip per form submission (negligible at this scale).
- Test clients must not blindly follow redirects when asserting the contract: cpr needs `cpr::Redirect{false}` in E2E tests — a hard-won lesson (see below).

## Implementation notes / pitfalls discovered

- `cpr::Redirect{0L}` selects the `long maximum` overload ("refuse redirects" → curl reports an *error* and cpr surfaces status 0). `cpr::Redirect{false}` selects the `bool follow` overload — the correct way to observe a raw 303 in tests. Read the header's constructor list when behavior confuses; brace-init on overloaded constructors is a trap.
- E2E tests assert the 303 status and then issue a separate GET to verify the result — mirroring exactly what a browser does.

## Alternatives considered

- Render directly from POST — rejected (all the classic PRG problems above).
- HTMX-driven partial updates without redirects — a valid future enhancement for inline editing; full-page PRG remains the baseline for form pages.
