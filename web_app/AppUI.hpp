/**
 * @file AppUI.hpp
 * @brief Declares the main UI composition entry point for the Company C web app.
 *
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Web Interface module - builds the landing page using WebTextbox, WebButton,
 * WebImage, WebLayout, and WebCanvas.
 *
 *Citation - ChatGPT LLM (OpenAI) was used to plan the structure of this file and help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Group 24 Web Interface - Prijam Khanal
 * Copyright (c) 2026 Group 24
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace cse498 {

namespace AppUI {

/**
 * @brief Builds and displays the Company C landing page.
 *
 * Creates and configures all UI elements (canvas background, heading, subtitle,
 * image, button) and attaches them to the DOM. Intended to be called once when
 * the Emscripten runtime is initialized.
 */
void Run();

}  // namespace AppUI
}  // namespace cse498
