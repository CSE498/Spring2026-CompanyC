#!/usr/bin/env bash
# Builds the Company C Web Interface app for the browser using Emscripten.
# Run from repo root: bash web_app/build.sh
# Then: cd web_app && python3 -m http.server 8080
# Open: http://localhost:8080/
#Citation - ChatGPT LLM (OpenAI) was used to generate this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
#Author - Prijam Khanal
#Copyright (c) 2026 Prijam Khanal
#SPDX-License-Identifier: MIT

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

# If em++ is not in PATH, source emsdk from common locations
if ! command -v em++ &>/dev/null; then
  for emsdk in "$HOME/emsdk" "./emsdk" "/opt/emsdk"; do
    if [[ -f "$emsdk/emsdk_env.sh" ]]; then
      echo "Sourcing Emscripten from $emsdk..."
      source "$emsdk/emsdk_env.sh"
      break
    fi
  done
fi

if ! command -v em++ &>/dev/null; then
  echo "Error: em++ not found. Install Emscripten first."
  echo "See web_app/README.md for setup instructions."
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
rm -f "$SCRIPT_DIR/app.js" "$SCRIPT_DIR/app.wasm"

em++ -std=c++17 -O0 -g -Wall -Wextra -Wpedantic -Werror -Wno-dollar-in-identifier-extension \
  -I./source \
  -I./web_app \
  ./web_app/main.cpp \
  ./web_app/AppUI.cpp \
  ./source/tools/WebTextbox/WebTextbox.cpp \
  ./source/tools/WebButton/WebButton.cpp \
  ./source/tools/WebImage/WebImage.cpp \
  ./source/web/WebCanvas.cpp \
  -sWASM=1 \
  -sEXPORTED_FUNCTIONS="['_RunWebInterface']" \
  -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -lembind \
  -o "$SCRIPT_DIR/app.js"

echo "Build complete. Serve with: cd web_app && python3 -m http.server 8080"
echo "Then open: http://localhost:8080/"
