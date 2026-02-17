#!/usr/bin/env bash
# Builds WebTextbox smoke demo for the browser using Emscripten.
# Run: bash webtextbox_build_emscripten.sh
# Then: python3 -m http.server 8080
# Open:  http://localhost:8080/webtextbox_demo.html

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

rm -f webtextbox_demo.js webtextbox_demo.wasm

em++ -std=c++17 -O0 -g -Wall -Wextra -Wpedantic -Werror \
  -I./source \
  ./source/WebTextbox/WebTextbox.cpp \
  ./source/WebTextbox/webtextbox_smoke_demo.cpp \
  -sWASM=1 \
  -sEXPORTED_FUNCTIONS="['_RunWebTextboxDemo']" \
  -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -o webtextbox_demo.js

echo "Build complete. Open webtextbox_demo.html in a browser."
