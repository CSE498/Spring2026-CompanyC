#!/usr/bin/env bash
# run_autotick_policy_tests.sh - runs AutoTickPolicy unit tests
# Usage (from repository root):
#   bash tests/run_autotick_policy_tests.sh

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

g++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -O0 -g \
  -I./source \
  ./tests/tools/AutoTickPolicyTest.cpp \
  -o autotick_policy_tests

./autotick_policy_tests
