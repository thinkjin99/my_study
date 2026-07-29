#!/usr/bin/env bash
set -euo pipefail

REPOSITORY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
QUARTZ_DIR="$REPOSITORY_DIR/quartz-site"

if [[ $# -gt 1 || ( $# -eq 1 && "$1" != "--serve" ) ]]; then
  echo "Usage: $0 [--serve]"
  exit 64
fi

"$REPOSITORY_DIR/scripts/sync-quartz-content.sh" "$QUARTZ_DIR"

if [[ ! -d "$QUARTZ_DIR/node_modules" ]]; then
  echo "Quartz dependencies are missing. Run: cd quartz-site && npm ci"
  exit 69
fi

cd "$QUARTZ_DIR"

if [[ ${1:-} == "--serve" ]]; then
  exec node quartz/bootstrap-cli.mjs build --serve
fi

node quartz/bootstrap-cli.mjs build
