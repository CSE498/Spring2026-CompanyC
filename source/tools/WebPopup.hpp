/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebPopup — modal messages using WebTextbox + WebButton on a thin DOM shell.
 *
 * Citation - LLM (OpenAI) was used to help generate parts of this file,
 * and maintain consistency with the project. The code was then reviewed
 * and heavily edited by the author to ensure correctness and suitability
 * for the project.
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */
 
#pragma once

#include <optional>
#include <string>

namespace cse498 {

/// Controls how a popup is dismissed: OK button, automatic timer, or both.
struct WebPopupOptions {
  bool               show_ok_button  = true;         ///< Show the primary dismiss button
  bool               auto_dismiss    = false;        ///< Enable timed close behavior
  std::optional<int> auto_dismiss_ms = std::nullopt; ///< Optional delay used when auto_dismiss is true
};

/// One queued popup: message plus behavior (for WebInterface → WebApp → EnqueueWebPopup).
struct WebPopupRequest {
  std::string     message;
  WebPopupOptions options;
};

/// Queues a modal in the browser (Emscripten). Message uses WebTextbox; OK uses
/// WebButton when show_ok_button is true. Timer uses setTimeout when auto_dismiss
/// is true. On native builds this is a no-op.
void EnqueueWebPopup(const std::string&              message,
                     const WebPopupOptions& options = {});

}  // namespace cse498
