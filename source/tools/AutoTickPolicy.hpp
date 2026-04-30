/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * AutoTickPolicy - decision making for auto tick loop. 
 *(created specifically for testing purposes)
 *
 * Citation - LLM (OpenAI) was used to help generate parts of this file,
 * and maintain consistency with the project. The code was then reviewed
 * and heavily edited by the author to ensure correctness and suitability
 * for the project.
 *
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */
#pragma once

namespace cse498 {

enum class AutoTickStartBlocker {
  None,
  PopupVisible,
};

struct AutoTickStartDecision {
  bool                 should_start;
  AutoTickStartBlocker blocker;
  int                  retry_delay_ms;
};

inline AutoTickStartDecision DecideAutoTickStart(bool popup_visible,
                                                 int popup_retry_ms = 100) {
  if (popup_visible) {
    return AutoTickStartDecision{false, AutoTickStartBlocker::PopupVisible,
                                 popup_retry_ms};
  }
  return AutoTickStartDecision{true, AutoTickStartBlocker::None, 0};
}

inline int ClampAutoTickIntervalMs(int ms, int min_ms = 50) {
  return (ms < min_ms) ? min_ms : ms;
}

}  // namespace cse498
