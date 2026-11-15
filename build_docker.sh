#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(pwd -P)"

docker build -t legacyex-builder .

docker run --rm \
    -v "$PROJECT_ROOT:$PROJECT_ROOT" \
    -w "$PROJECT_ROOT" \
    -e WORKSPACE="$PROJECT_ROOT" \
    legacyex-builder "$@"
