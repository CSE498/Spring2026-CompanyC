/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * AutoTickPolicyTest - unit tests for AutoTickPolicy. 
 * use command: "bash tests/run_autotick_policy_tests.sh" to run the tests.
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
#include <cassert>
#include <iostream>

#include "tools/AutoTickPolicy.hpp"

int main() {
  using cse498::AutoTickStartDecision;
  using cse498::AutoTickStartBlocker;
  using cse498::ClampAutoTickIntervalMs;
  using cse498::DecideAutoTickStart;

  // Start gate should delay while popup remains visible.
  const AutoTickStartDecision blocked = DecideAutoTickStart(true);
  assert(blocked.should_start == false);
  assert(blocked.blocker == AutoTickStartBlocker::PopupVisible);
  assert(blocked.retry_delay_ms == 100);

  const AutoTickStartDecision ready = DecideAutoTickStart(false);
  assert(ready.should_start == true);
  assert(ready.blocker == AutoTickStartBlocker::None);
  assert(ready.retry_delay_ms == 0);

  // Interval clamp should enforce lower bound used by WebApp.
  assert(ClampAutoTickIntervalMs(500) == 500);
  assert(ClampAutoTickIntervalMs(50) == 50);
  assert(ClampAutoTickIntervalMs(1) == 50);
  assert(ClampAutoTickIntervalMs(-100) == 50);
  assert(ClampAutoTickIntervalMs(25, 10) == 25);
  assert(ClampAutoTickIntervalMs(5, 10) == 10);

  std::cout << "AutoTickPolicyTest passed." << std::endl;
  return 0;
}
