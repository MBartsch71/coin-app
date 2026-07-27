# ADR-0008: Container lifecycle via Quadlet and systemd linger

- Status: accepted
- Date: 2026-07-27
- Deciders: Matthias (with Hermes as mentor/reviewer)

## Context

The three PostgreSQL containers (ADR-0005) were started with plain `podman run -d`: no boot start, no crash recovery, no log integration. A server application needs containers that start when the machine boots and restart when they crash — without anyone logging in.

## Decision

All three postgres containers are managed as **systemd user services via podman Quadlet**:

- One `.container` file per environment in `~/.config/containers/systemd/` (`coin-postgres-{dev,test,prod}.container`)
- `Restart=always` for crash recovery
- `[Install] WantedBy=default.target` for automatic start
- **`loginctl enable-linger blackeagle`** so user services start at boot *without an interactive login* — the critical, easily-forgotten piece
- Prod credentials live in `~/containers/coin-postgres-prod.env` (mode 600), referenced via `EnvironmentFile=` — secrets stay out of the quadlet files

Container data persists in host bind mounts (`~/containers/postgres-data*`), so containers remain disposable and recreatable.

## Consequences

### Positive

- Boot start + crash recovery with zero custom scripts
- `systemctl --user status|restart` and `journalctl --user -u ...` for all containers
- The same pattern will host the prod **app** service in Phase 0.5, including ordering (`After=coin-postgres-prod.service`)
- Declarative, reviewable container definitions

### Negative / Risks

- Quadlet files live outside the git repo (host-specific ops config) — documented here and in arc42 §7
- Linger is a machine-level prerequisite; rebuilding the host requires re-running it

## Alternatives considered

- **`podman run --restart=always`**: restart works, but no journal integration, no dependencies, no declarative definition — rejected
- **`podman generate systemd`**: deprecated in favor of Quadlet — rejected

## References

- ADR-0005 (environment isolation)
- https://docs.podman.io/en/latest/markdown/podman-systemd.unit.5.html
