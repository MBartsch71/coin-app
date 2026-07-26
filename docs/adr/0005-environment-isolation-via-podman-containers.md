# ADR-0005: Environment isolation via separate podman containers

- Status: accepted
- Date: 2026-07-25
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

The app needs dev, test, and prod environments. One shared PostgreSQL container with multiple databases would allow tests to accidentally reach dev/prod data. E2E tests must be free to wipe and seed their database aggressively; prod holds the real coin collection.

## Decision

One dedicated PostgreSQL **podman container per environment**:

| Environment | Container | Host port | Database | App port |
|---|---|---|---|---|
| dev | `coin-postgres-dev` | 5432 | `coin_catalog_dev` | 9000 |
| test | `coin-postgres-test` | 5433 | `coin_catalog_test` | 9001 |
| prod | `coin-postgres-prod` | 5434 | `coin_catalog_prod` | 9080 |

- The test container is **disposable**: recreated/wiped/seeded freely
- Prod uses **distinct credentials** from dev/test — shared passwords defeat isolation
- Migrations are applied per environment (manually for now; runner is on the debt list)
- The app selects its environment purely through env vars (see ADR-0002)

## Consequences

### Positive

- Hard isolation: a buggy test cannot touch the real collection
- Test environment is reproducible from scratch in seconds
- Mirrors real deployment topologies (per-env services)

### Negative / Risks

- Three containers to keep running (RAM, lifecycle management)
- Container setup is manual podman CLI for now (candidates: `podman kube play`, quadlet files)

## Alternatives considered

- **One container, three databases**: weaker isolation; rejected on Matthias's explicit preference
- **Separate machines**: overkill for a single-host hobby project

## References

- ADR-0002 (configuration via environment)
