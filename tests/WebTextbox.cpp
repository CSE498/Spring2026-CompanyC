/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Unit tests for WebTextbox
 *
 * Compiles natively (no Emscripten needed) because WebTextbox
 * has native stubs that let us exercise all the C++ logic
 * without an actual browser.
 *
 * Citation - LLM (OpenAI) was used to help generate parts of this file, and maintain consistency with the project. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Prijam Khanal
 */

#include <cassert>
#include <iostream>
#include <string>
#include <utility>

#include "WebTextbox/WebTextbox.hpp"

using cse498::WebTextbox;

// ----- Test: default-constructed state -----

static void TestDefaults() {
  WebTextbox box;

  assert(box.GetText().empty());
  assert(box.GetFontFamily().empty());
  assert(box.GetFontSize() == 16);
  assert(box.GetFontWeight() == "normal");
  assert(box.GetFontStyle() == "normal");
  assert(box.GetLineHeight() == 1.4);
  assert(box.GetTextColor().empty());
  assert(box.GetBackgroundColor().empty());
  assert(box.GetTextAlignment() == "left");
  assert(box.GetLeftPx() == 0.0);
  assert(box.GetTopPx() == 0.0);
  assert(box.GetWidthPx() == 0.0);
  assert(box.GetHeightPx() == 0.0);
  assert(box.IsVisible());
  assert(box.GetOpacity() == 1.0);
  assert(box.GetElementId().empty());
  assert(box.GetParentId().empty());
  assert(!box.IsCreated());
  assert(box.GetHandle() == 0);
}

// ----- Test: text content operations -----

static void TestTextContent() {
  WebTextbox box;

  box.SetText("Hello");
  assert(box.GetText() == "Hello");

  box.AppendText(" World");
  assert(box.GetText() == "Hello World");

  box.AppendText("!");
  assert(box.GetText() == "Hello World!");

  box.ClearText();
  assert(box.GetText().empty());

  // setting text after clear should still work
  box.SetText("fresh start");
  assert(box.GetText() == "fresh start");
}

// ----- Test: text with special characters -----

static void TestTextEdgeCases() {
  WebTextbox box;

  // empty string
  box.SetText("");
  assert(box.GetText().empty());

  // newlines and tabs
  box.SetText("line1\nline2\ttab");
  assert(box.GetText() == "line1\nline2\ttab");

  // html-like content (we just store it as text, not html)
  box.SetText("<b>bold</b>");
  assert(box.GetText() == "<b>bold</b>");

  // unicode
  box.SetText("cafe\xCC\x81");  // cafe + combining accent
  assert(box.GetText().size() > 0);

  // long string
  std::string longStr(5000, 'x');
  box.SetText(longStr);
  assert(box.GetText().size() == 5000);
}

// ----- Test: font property setters/getters -----

static void TestFontProperties() {
  WebTextbox box;

  box.SetFontFamily("Arial");
  assert(box.GetFontFamily() == "Arial");

  box.SetFontFamily("'Courier New', monospace");
  assert(box.GetFontFamily() == "'Courier New', monospace");

  box.SetFontSize(24);
  assert(box.GetFontSize() == 24);

  box.SetFontSize(1);  // minimum valid
  assert(box.GetFontSize() == 1);

  box.SetFontWeight("bold");
  assert(box.GetFontWeight() == "bold");

  box.SetFontWeight("700");
  assert(box.GetFontWeight() == "700");

  box.SetFontStyle("italic");
  assert(box.GetFontStyle() == "italic");

  box.SetLineHeight(2.0);
  assert(box.GetLineHeight() == 2.0);

  box.SetLineHeight(0.5);
  assert(box.GetLineHeight() == 0.5);
}

// ----- Test: color setters/getters -----

static void TestColors() {
  WebTextbox box;

  box.SetTextColor("#FF0000");
  assert(box.GetTextColor() == "#FF0000");

  box.SetTextColor("rgb(0, 128, 255)");
  assert(box.GetTextColor() == "rgb(0, 128, 255)");

  box.SetBackgroundColor("white");
  assert(box.GetBackgroundColor() == "white");

  box.SetBackgroundColor("#00FF00");
  assert(box.GetBackgroundColor() == "#00FF00");
}

// ----- Test: alignment -----

static void TestAlignment() {
  WebTextbox box;

  assert(box.GetTextAlignment() == "left");  // default

  box.SetTextAlignment("center");
  assert(box.GetTextAlignment() == "center");

  box.SetTextAlignment("right");
  assert(box.GetTextAlignment() == "right");

  box.SetTextAlignment("justify");
  assert(box.GetTextAlignment() == "justify");
}

// ----- Test: position and size -----

static void TestLayout() {
  WebTextbox box;

  box.SetPosition(100.5, 200.0);
  assert(box.GetLeftPx() == 100.5);
  assert(box.GetTopPx() == 200.0);

  box.SetPosition(0, 0);
  assert(box.GetLeftPx() == 0.0);
  assert(box.GetTopPx() == 0.0);

  // negative positions are valid (off-screen)
  box.SetPosition(-50, -100);
  assert(box.GetLeftPx() == -50.0);
  assert(box.GetTopPx() == -100.0);

  box.SetSize(640, 480);
  assert(box.GetWidthPx() == 640.0);
  assert(box.GetHeightPx() == 480.0);

  // zero size means "auto"
  box.SetSize(0, 0);
  assert(box.GetWidthPx() == 0.0);
  assert(box.GetHeightPx() == 0.0);
}

// ----- Test: visibility and opacity -----

static void TestVisibility() {
  WebTextbox box;
  assert(box.IsVisible());  // default visible

  box.SetVisible(false);
  assert(!box.IsVisible());

  box.SetVisible(true);
  assert(box.IsVisible());

  box.SetOpacity(0.5);
  assert(box.GetOpacity() == 0.5);

  box.SetOpacity(0.0);
  assert(box.GetOpacity() == 0.0);

  box.SetOpacity(1.0);
  assert(box.GetOpacity() == 1.0);
}

// ----- Test: opacity clamping -----

static void TestOpacityClamp() {
  WebTextbox box;

  box.SetOpacity(-5.0);
  assert(box.GetOpacity() == 0.0);

  box.SetOpacity(3.14);
  assert(box.GetOpacity() == 1.0);

  box.SetOpacity(999.0);
  assert(box.GetOpacity() == 1.0);

  box.SetOpacity(-0.001);
  assert(box.GetOpacity() == 0.0);

  // edge: exactly 0 and 1 should stay as-is
  box.SetOpacity(0.0);
  assert(box.GetOpacity() == 0.0);

  box.SetOpacity(1.0);
  assert(box.GetOpacity() == 1.0);
}

// ----- Test: DOM identity -----

static void TestDomIdentity() {
  WebTextbox box;

  box.SetElementId("my-textbox");
  assert(box.GetElementId() == "my-textbox");

  box.SetElementId("");
  assert(box.GetElementId().empty());

  box.SetParentId("container");
  assert(box.GetParentId() == "container");

  box.SetParentId("");
  assert(box.GetParentId().empty());
}

// ----- Test: lifecycle (create, destroy, recreate) -----

static void TestLifecycle() {
  WebTextbox box;
  assert(!box.IsCreated());
  assert(box.GetHandle() == 0);

  box.SetText("before create");
  box.SetFontSize(20);

  box.EnsureCreated();
  assert(box.IsCreated());
  assert(box.GetHandle() != 0);

  auto h1 = box.GetHandle();

  // calling EnsureCreated again should be a no-op
  box.EnsureCreated();
  assert(box.GetHandle() == h1);

  box.Destroy();
  assert(!box.IsCreated());
  assert(box.GetHandle() == 0);

  // state should still be there after destroy (just not in DOM)
  assert(box.GetText() == "before create");
  assert(box.GetFontSize() == 20);

  // should be able to recreate
  box.EnsureCreated();
  assert(box.IsCreated());
  auto h2 = box.GetHandle();
  assert(h2 != 0);
}

// ----- Test: move constructor -----

static void TestMoveCtor() {
  WebTextbox a;
  a.SetText("moved text");
  a.SetFontFamily("Helvetica");
  a.SetPosition(10, 20);
  a.SetOpacity(0.8);
  a.EnsureCreated();

  assert(a.IsCreated());
  auto h = a.GetHandle();
  assert(h != 0);

  WebTextbox b(std::move(a));

  // b should have everything a had
  assert(b.IsCreated());
  assert(b.GetHandle() == h);
  assert(b.GetText() == "moved text");
  assert(b.GetFontFamily() == "Helvetica");
  assert(b.GetLeftPx() == 10.0);
  assert(b.GetTopPx() == 20.0);
  assert(b.GetOpacity() == 0.8);

  // a should be empty/safe
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);

  b.Destroy();
  assert(!b.IsCreated());
}

// ----- Test: move assignment -----

static void TestMoveAssign() {
  WebTextbox a;
  a.SetText("source");
  a.SetFontSize(32);
  a.EnsureCreated();
  auto ha = a.GetHandle();
  assert(ha != 0);

  WebTextbox b;
  b.SetText("target");
  b.SetPosition(99, 99);
  b.EnsureCreated();
  assert(b.IsCreated());

  // move a into b -- b's old element should get destroyed
  b = std::move(a);

  assert(b.IsCreated());
  assert(b.GetHandle() == ha);
  assert(b.GetText() == "source");
  assert(b.GetFontSize() == 32);

  // a is now empty
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);

  b.Destroy();
}

// ----- Test: setters work after EnsureCreated (DOM sync) -----

static void TestSettersAfterCreate() {
  WebTextbox box;
  box.EnsureCreated();

  // these should not crash - they push to native stubs
  box.SetText("live text");
  assert(box.GetText() == "live text");

  box.SetFontFamily("Georgia");
  assert(box.GetFontFamily() == "Georgia");

  box.SetFontSize(14);
  assert(box.GetFontSize() == 14);

  box.SetTextColor("blue");
  assert(box.GetTextColor() == "blue");

  box.SetBackgroundColor("yellow");
  assert(box.GetBackgroundColor() == "yellow");

  box.SetTextAlignment("center");
  assert(box.GetTextAlignment() == "center");

  box.SetPosition(50, 75);
  assert(box.GetLeftPx() == 50.0);

  box.SetSize(300, 150);
  assert(box.GetWidthPx() == 300.0);

  box.SetVisible(false);
  assert(!box.IsVisible());

  box.SetOpacity(0.33);
  assert(box.GetOpacity() == 0.33);

  box.SetElementId("live-box");
  assert(box.GetElementId() == "live-box");

  box.SetParentId("root");
  assert(box.GetParentId() == "root");

  box.Destroy();
}

// ----- main -----

int main() {
  TestDefaults();
  TestTextContent();
  TestTextEdgeCases();
  TestFontProperties();
  TestColors();
  TestAlignment();
  TestLayout();
  TestVisibility();
  TestOpacityClamp();
  TestDomIdentity();
  TestLifecycle();
  TestMoveCtor();
  TestMoveAssign();
  TestSettersAfterCreate();

  std::cout << "All WebTextbox tests passed." << std::endl;
  return 0;
}
