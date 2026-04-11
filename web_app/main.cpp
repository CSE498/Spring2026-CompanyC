/**
 * @file main.cpp
 * @brief Entry point for the Company C Web Interface module.
 *
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Exports RunWebInterface for Emscripten; delegates to AppUI::Run().
 *
 * Citation - ChatGPT LLM (OpenAI) was used to plan the structure of this file. The code was then written, reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Group 24 Web Interface - Prijam Khanal
 * Copyright (c) 2026 Group 24
 * SPDX-License-Identifier: MIT
 */

#include "AppUI.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
extern "C" void RunWebInterface() {
  cse498::AppUI::Run();
}
