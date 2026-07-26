# ADR-0001: Crow as web framework

- Status: accepted
- Date: 2026-07-25 (documented retroactively)
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

coin-app needs a lightweight C++ web framework for server-rendered HTML with HTMX interactivity. Requirements: simple routing, built-in templating, no heavy runtime or application server, good fit for a modern C++26 learning project.

## Decision

We use **Crow** (header-only, ASIO-based) with its built-in **mustache** template engine.

## Consequences

### Positive

- Minimal dependency footprint; integrates cleanly with CMake (`find_package(Crow)`)
- Routing macros (`CROW_ROUTE`) keep handlers compact and readable
- Mustache templating included — logic-less templates enforce a thin view layer
- Standalone ASIO — no external web server needed

### Negative / Risks

- Smaller community than Boost.Beast / cpp-httplib ecosystems
- Real gotcha found in practice: `crow::mustache::set_base()` (route-level template path) is **reset by the router on every request** to the global base. Template directory must be set via `crow::mustache::set_global_base()`. (Debugged 2026-07-25.)
- Template path is resolved relative to the process working directory by default; we bake an absolute path in at compile time (`COINAPP_TEMPLATE_DIR` compile definition).

## Alternatives considered

- **Boost.Beast**: full control but far more boilerplate for a CRUD app
- **cpp-httplib**: very simple, but no integrated template engine

## References

- https://crowcpp.org
