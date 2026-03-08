#!/usr/bin/env bash
# run_webtextbox_tests.sh - runs the WebTextbox unit tests
# Citation - LLM (OpenAI) was used to help generate parts of this file,
# and maintain consistency with the project. The code was then reviewed
# and heavily edited by the author to ensure correctness and suitability
# for the project.
# @author Prijam Khanal
# Copyright (c) 2026 Prijam Khanal
# SPDX-License-Identifier: MIT

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -O0 -g \
  -I./source \
  ./tests/tools/WebTextbox/WebTextbox.cpp ./source/tools/WebTextbox/WebTextbox.cpp \
  -o webtextbox_tests

./webtextbox_tests
