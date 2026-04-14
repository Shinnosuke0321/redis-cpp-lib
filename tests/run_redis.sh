#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

case "${1:-up}" in
    up)
        echo "Starting Redis..."
        docker compose -f "$SCRIPT_DIR/docker-compose.yml" up -d
        echo "Redis is running on localhost:6379"
        ;;
    down)
        echo "Stopping Redis..."
        docker compose -f "$SCRIPT_DIR/docker-compose.yml" down
        ;;
    logs)
        docker compose -f "$SCRIPT_DIR/docker-compose.yml" logs -f
        ;;
    *)
        echo "Usage: $0 [up|down|logs]"
        exit 1
        ;;
esac
