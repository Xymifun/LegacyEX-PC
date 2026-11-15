#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${1:?Usage: clangd-docker PROJECT_ROOT [TARGET]}"
TARGET="${2:-pc64}"

PROJECT_ROOT="$(realpath "$PROJECT_ROOT")"

exec docker run --rm -i \
    -v "$PROJECT_ROOT:$PROJECT_ROOT" \
    -w "$PROJECT_ROOT" \
    -e WORKSPACE="$PROJECT_ROOT" \
    --entrypoint clangd \
    legacyex-builder \
    --background-index \
    --compile-commands-dir="$PROJECT_ROOT/build/$TARGET" \
    --query-driver="/usr/bin/*mingw*g++*,/usr/bin/*mingw*gcc*"
