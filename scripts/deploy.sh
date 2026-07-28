#!/usr/bin/env bash
# deploy.sh - promote a tested increment to production (port 9000)
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROD_HOME="/home/blackeagle/apps/coin-app"

echo "==> 1/4 Test gate: full pyramid must be green"
ctest --test-dir "$REPO_DIR/build" --output-on-failure

echo "==> 2/4 Build"
cmake --build "$REPO_DIR/build"

echo "==> 3/4 Deploy artifacts to $PROD_HOME"
install -Dm755 "$REPO_DIR/build/coin_app" "$PROD_HOME/bin/coin_app"
rm -rf "$PROD_HOME/templates"
cp -r "$REPO_DIR/templates" "$PROD_HOME/templates"

echo "==> 4/4 Restart prod service"
systemctl --user restart coin-app-prod
sleep 1
systemctl --user --no-pager status coin-app-prod | head -5

echo "==> Done. Prod: http://localhost:9080 - try /health"