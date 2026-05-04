#!/usr/bin/env bash
# run_webpopup_tests.sh - runs WebPopup + WebInterface popup unit tests
# Usage (from repository root):
#   bash tests/run_webpopup_tests.sh
#
# SPDX-License-Identifier: MIT

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

g++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -O0 -g \
  -I./source \
  ./tests/tools/WebPopupTest.cpp \
  ./source/tools/WebInterface.cpp \
  ./source/tools/WebPopup.cpp \
  -o webpopup_tests

./webpopup_tests
