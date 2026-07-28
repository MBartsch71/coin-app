# ADR-0010: Production deployment via systemd user service and test-gated deploy script

- Status: accepted
- Date: 2026-07-28
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

The app needed a repeatable path from "green in dev" to "running in prod" (port 9080, `coin_catalog_prod`). Constraints: one machine, one user, prod must be independent of the dev checkout, and no deploy should ever ship a red test suite.

## Decision

**Layout** — dev and prod artifacts are fully separated:

- `~/projects/coin-app/` — dev repo (frequent change)
- `~/apps/coin-app/` — prod home: `bin/coin_app`, `templates/`, `.env.prod` (installed **once**; never touched by deploys)

**Runtime config** — the systemd unit reads `~/apps/coin-app/.env.prod` via `EnvironmentFile=`. The template directory became a runtime-overrideable setting (`COINAPP_TEMPLATE_DIR` env var wins; the compile-time baked path is the dev fallback) so the deployed binary never depends on the repo's location.

**Service** — `~/.config/systemd/user/coin-app-prod.service`:

- `Requires=` + `After=coin-postgres-prod.service` — the app starts only after its database (ADR-0008 pattern extended to the app)
- `Restart=on-failure` (+ linger for boot start)
- `WantedBy=default.target`

**Deploy** — `scripts/deploy.sh` (versioned in the repo):

1. `ctest` full pyramid — **the gate**; any failure aborts (`set -euo pipefail`)
2. build
3. copy binary + templates into `~/apps/coin-app/`
4. `systemctl --user restart coin-app-prod`

Migrations remain manual per environment (a runner is on the debt list, arc42 §11).

## Consequences

### Positive

- One command deploys; the test gate is mechanical, not discipline
- Prod survives any dev-repo operation (refactors, clean, experiments)
- Config churn eliminated: code deploys often, config deployed once deliberately
- Boot resilience: DB container and app service both auto-start in the right order

### Negative / Risks

- Prod home (`~/apps/`) is outside git — the env file there is the only copy (mitigated by also keeping a copy in `config/.env.prod`, gitignored)
- No rollback mechanism yet (previous binary is overwritten; acceptable at this stage)

## Alternatives considered

- **Run prod from the repo build dir**: breaks on any repo operation — rejected
- **Containerize the app too (podman/quadlet)**: valid future step; plain service is simpler to debug while the app changes daily — deferred

## References

- ADR-0002 (config via env), ADR-0005 (env isolation), ADR-0006 (test gate), ADR-0008 (quadlet lifecycle)
