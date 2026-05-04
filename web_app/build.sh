#!/usr/bin/env bash
# Build script for the Company C web app (Emscripten/WASM)
# Run from the repository root: bash web_app/build.sh

set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$REPO_ROOT/web_app"

em++ \
  -std=c++20 \
  -I"$REPO_ROOT/source" \
  -I"$REPO_ROOT/web_app" \
  "$REPO_ROOT/web_app/main.cpp" \
  "$REPO_ROOT/web_app/AppUI.cpp" \
  "$REPO_ROOT/source/tools/WebButton.cpp" \
  "$REPO_ROOT/source/tools/WebCanvas.cpp" \
  "$REPO_ROOT/source/tools/WebImage.cpp" \
  "$REPO_ROOT/source/tools/WebLayout.cpp" \
  "$REPO_ROOT/source/tools/WebTextbox.cpp" \
  -lembind \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_RunWebInterface"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -o "$OUT_DIR/app.js"

echo "Build complete: web_app/app.js and web_app/app.wasm"
