#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

g++ -std=c++17 -Wall -Wextra -Wpedantic -O0 -g \
  -I./source -I./source/tools \
  ./tests/WebImage.cpp ./source/tools/WebImage.cpp \
  -o webimage_tests

./webimage_tests
