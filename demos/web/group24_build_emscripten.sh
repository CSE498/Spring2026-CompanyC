#!/usr/bin/env bash
# Builds Group 24 stub demo (Group24_main + WebCanvas + WebTextbox + StubWorldAdapter).
# Run from repository root: bash demos/web/group24_build_emscripten.sh
# Then: python3 -m http.server 8000
# Open: http://localhost:8000/group24_demo.html

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

rm -f group24_demo.js group24_demo.wasm group24_demo.html

if ! command -v em++ &>/dev/null; then
  for emsdk in "$HOME/emsdk" "./emsdk" "/opt/emsdk"; do
    if [[ -f "$emsdk/emsdk_env.sh" ]]; then
      source "$emsdk/emsdk_env.sh"
      break
    fi
  done
fi

if ! command -v em++ &>/dev/null; then
  echo "Error: em++ not found. Source emsdk_env.sh or install Emscripten."
  exit 1
fi

em++ -std=c++20 -O0 -g -Wall -Wextra -Wpedantic -lembind \
  -I./source \
  ./source/Group24_main.cpp \
  ./source/tools/StubWorldAdapter.cpp \
  ./source/tools/WebCanvas.cpp \
  ./source/tools/WebTextbox.cpp \
  -sWASM=1 \
  -sEXPORTED_FUNCTIONS="['_main','_Group24HandleAction']" \
  -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -o group24_demo.html

echo "Build complete. Serve repo root and open group24_demo.html"
