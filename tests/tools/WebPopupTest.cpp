/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Unit tests for WebPopup types, EnqueueWebPopup (native stub), and
 * WebInterface popup Notify / TakePendingPopups behavior.
 *
 * Compiles natively (no Emscripten): WebPopup's browser path is ifdef'd out;
 * WebInterface popup queue logic is pure C++.
 *
 * How to run (from repository root; recommended):
 *   bash tests/run_webpopup_tests.sh
 *
 * Manual compile (must print "All WebPopup tests passed." and exit 0):
 *   g++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -O0 -g -I./source \
 *     ./tests/tools/WebPopupTest.cpp ./source/tools/WebInterface.cpp \
 *     ./source/tools/WebPopup.cpp -o webpopup_tests && ./webpopup_tests
 *
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "core/AgentBase.hpp"
#include "core/WorldBase.hpp"
#include "tools/WebInterface.hpp"
#include "tools/WebPopup.hpp"

using cse498::AgentBase;
using cse498::WebInterface;
using cse498::WebPopupOptions;
using cse498::WebPopupRequest;
using cse498::EnqueueWebPopup;
using cse498::WorldBase;

// ----- Minimal world for WebInterface construction (header-only WorldBase) -----

namespace {

class MinimalWorld : public WorldBase {
 public:
  int DoAction(AgentBase& /*agent*/, size_t /*action_id*/) override { return 0; }
};

}  // namespace

// ----- WebPopupOptions / WebPopupRequest -----

static void TestWebPopupOptionsDefaults() {
  WebPopupOptions o;
  assert(o.show_ok_button == true);
  assert(o.auto_dismiss == false);
  assert(!o.auto_dismiss_ms.has_value());
}

static void TestWebPopupRequestAggregate() {
  WebPopupOptions o;
  o.show_ok_button = false;
  o.auto_dismiss = true;
  o.auto_dismiss_ms = 500;
  WebPopupRequest r{"hello", o};
  assert(r.message == "hello");
  assert(r.options.show_ok_button == false);
  assert(r.options.auto_dismiss == true);
  assert(r.options.auto_dismiss_ms.has_value());
  assert(*r.options.auto_dismiss_ms == 500);
}

// ----- EnqueueWebPopup native (no-op, must not throw) -----

static void TestEnqueueWebPopupNativeNoOp() {
  EnqueueWebPopup("x");
  EnqueueWebPopup("y", WebPopupOptions{});
  WebPopupOptions o;
  o.auto_dismiss = true;
  o.auto_dismiss_ms = 100;
  o.show_ok_button = false;
  EnqueueWebPopup("z", o);
}

// ----- WebInterface: popup / popup_timed / popup_timed_ok -----

static void TestInterfacePopupBasicOkOnly() {
  MinimalWorld w;
  WebInterface ui(0, "Player", w);

  ui.Notify("You win!", "popup");
  std::vector<WebPopupRequest> batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "You win!");
  assert(batch[0].options.show_ok_button == true);
  assert(batch[0].options.auto_dismiss == false);

  assert(ui.TakePendingPopups().empty());
}

static void TestInterfacePopupTimedPipeMsAndText() {
  MinimalWorld w;
  WebInterface ui(1, "Player", w);

  ui.Notify("3000|Boss wave!", "popup_timed");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "Boss wave!");
  assert(batch[0].options.show_ok_button == false);
  assert(batch[0].options.auto_dismiss == true);
  assert(batch[0].options.auto_dismiss_ms.has_value());
  assert(*batch[0].options.auto_dismiss_ms == 3000);
}

static void TestInterfacePopupTimedNoPipeUsesDefaultMsAndFullText() {
  MinimalWorld w;
  WebInterface ui(2, "Player", w);

  ui.Notify("No pipe here", "popup_timed");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "No pipe here");
  assert(batch[0].options.auto_dismiss == true);
  assert(batch[0].options.auto_dismiss_ms.has_value());
  assert(*batch[0].options.auto_dismiss_ms == 1500);
}

static void TestInterfacePopupTimedOkBoth() {
  MinimalWorld w;
  WebInterface ui(3, "Player", w);

  ui.Notify("5000|Hurry up", "popup_timed_ok");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "Hurry up");
  assert(batch[0].options.show_ok_button == true);
  assert(batch[0].options.auto_dismiss == true);
  assert(batch[0].options.auto_dismiss_ms.has_value());
  assert(*batch[0].options.auto_dismiss_ms == 5000);
}

static void TestInterfacePopupTimedInvalidMsPrefixKeepsDefaultMs() {
  MinimalWorld w;
  WebInterface ui(4, "Player", w);

  ui.Notify("not_a_number|Still ok", "popup_timed");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "Still ok");
  assert(batch[0].options.auto_dismiss_ms.has_value());
  assert(*batch[0].options.auto_dismiss_ms == 1500);
}

static void TestInterfacePopupTimedNonPositiveDisablesAutoDismiss() {
  MinimalWorld w;
  WebInterface ui(5, "Player", w);

  ui.Notify("0|edge", "popup_timed");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 1);
  assert(batch[0].message == "edge");
  assert(batch[0].options.auto_dismiss == false);
  assert(!batch[0].options.auto_dismiss_ms.has_value());
}

static void TestInterfacePopupFifoOrder() {
  MinimalWorld w;
  WebInterface ui(6, "Player", w);

  ui.Notify("first", "popup");
  ui.Notify("2000|second", "popup_timed");
  auto batch = ui.TakePendingPopups();
  assert(batch.size() == 2);
  assert(batch[0].message == "first");
  assert(batch[1].message == "second");
  assert(batch[1].options.auto_dismiss == true);
}

static void TestInterfaceTakePendingPopupsClearsQueue() {
  MinimalWorld w;
  WebInterface ui(7, "Player", w);

  ui.Notify("a", "popup");
  assert(ui.TakePendingPopups().size() == 1);
  assert(ui.TakePendingPopups().empty());
}

static void TestInterfaceOtherNotifyTypesDoNotTouchPopupQueue() {
  MinimalWorld w;
  WebInterface ui(8, "Player", w);

  ui.Notify("hello", "status");
  ui.Notify("live", "mode_change");
  assert(ui.TakePendingPopups().empty());
}

// ----- main -----

int main() {
  TestWebPopupOptionsDefaults();
  TestWebPopupRequestAggregate();
  TestEnqueueWebPopupNativeNoOp();
  TestInterfacePopupBasicOkOnly();
  TestInterfacePopupTimedPipeMsAndText();
  TestInterfacePopupTimedNoPipeUsesDefaultMsAndFullText();
  TestInterfacePopupTimedOkBoth();
  TestInterfacePopupTimedInvalidMsPrefixKeepsDefaultMs();
  TestInterfacePopupTimedNonPositiveDisablesAutoDismiss();
  TestInterfacePopupFifoOrder();
  TestInterfaceTakePendingPopupsClearsQueue();
  TestInterfaceOtherNotifyTypesDoNotTouchPopupQueue();

  std::cout << "All WebPopup tests passed." << std::endl;
  return 0;
}
