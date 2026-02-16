#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g \
  -I./source \
  ./tests/WebTextbox.cpp ./source/WebTextbox/WebTextbox.cpp \
  -o webtextbox_tests

./webtextbox_tests
