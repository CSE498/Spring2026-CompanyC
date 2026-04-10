/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Smoke demo - creates a textbox in the browser to verify
 * that the Emscripten bridge actually works end-to-end.
 *
 * @author Prijam Khanal
 */

#include "tools/WebTextbox.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using cse498::WebTextbox;

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
extern "C" void RunWebTextboxDemo() {
  // static so the object outlives this function call
  static WebTextbox heading;
  heading.SetText("WebTextbox Demo");
  heading.SetFontFamily("Georgia, serif");
  heading.SetFontSize(36);
  heading.SetFontWeight("bold");
  heading.SetTextColor("#1a1a2e");
  heading.SetBackgroundColor("#e8e8e8");
  heading.SetTextAlignment("center");
  heading.SetPosition(40, 30);
  heading.SetSize(500, 60);
  heading.SetOpacity(0.95);
  heading.SetParentId("root");
  heading.SetElementId("demo-heading");
  heading.EnsureCreated();
  heading.SetVisible(true);

  // second textbox for body text
  static WebTextbox body;
  body.SetText("This text is rendered from C++ through Emscripten. "
               "The WebTextbox class manages a <div> element and lets "
               "you control font, color, position and more from C++.");
  body.SetFontFamily("'Segoe UI', Tahoma, sans-serif");
  body.SetFontSize(18);
  body.SetTextColor("#333");
  body.SetLineHeight(1.6);
  body.SetPosition(40, 110);
  body.SetSize(500, 180);
  body.SetParentId("root");
  body.SetElementId("demo-body");
  body.EnsureCreated();
  body.SetVisible(true);

  // TODO: maybe add a third box that updates on a timer?
}
