#!/usr/bin/env bash
# Usage: run_with_env.sh <env-file> <command> [args...]
# Loads the env file, then expects the command with those variables
set -euo pipefail

ENV_FILE="$1"
shift

set -a
source "$ENV_FILE"
set +a

exec "$@"
