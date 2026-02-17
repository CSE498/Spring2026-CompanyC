#include <cassert>
#include <iostream>

#include "../../../source/tools/WebImage/WebImage.h"
#include "tools/WebImage/WebImage.h"



/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief testing for WebImage 
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Sadwal Patel
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 **/



using cse498::WebImage;

static void TestDefaults() {
  WebImage img;
  assert(img.GetSource().empty());
  assert(img.GetAltText().empty());
  assert(img.GetLeftPx() == 0.0);
  assert(img.GetTopPx() == 0.0);
  assert(img.GetWidthPx() == 0.0);
  assert(img.GetHeightPx() == 0.0);
  assert(img.IsVisible());
  assert(img.GetOpacity() == 1.0);
  assert(img.GetZIndex() == 0);
  assert(img.GetParentElementId().empty());
  assert(img.GetElementId().empty());
  assert(!img.IsCreated());
}

static void TestSetters() {
  WebImage img;
  img.SetSource("assets/cat.png");
  img.SetAltText("a cat");
  img.SetPositionPx(12.5, 99.0);
  img.SetSizePx(320.0, 240.0);
  img.SetVisible(false);
  img.SetOpacity(0.25);
  img.SetZIndex(42);
  img.SetParentElementId("root");
  img.SetElementId("hero-image");

  assert(img.GetSource() == "assets/cat.png");
  assert(img.GetAltText() == "a cat");
  assert(img.GetLeftPx() == 12.5);
  assert(img.GetTopPx() == 99.0);
  assert(img.GetWidthPx() == 320.0);
  assert(img.GetHeightPx() == 240.0);
  assert(!img.IsVisible());
  assert(img.GetOpacity() == 0.25);
  assert(img.GetZIndex() == 42);
  assert(img.GetParentElementId() == "root");
  assert(img.GetElementId() == "hero-image");
}

static void TestOpacityClamp() {
  WebImage img;
  img.SetOpacity(-10.0);
  assert(img.GetOpacity() == 0.0);
  img.SetOpacity(2.0);
  assert(img.GetOpacity() == 1.0);
}

static void TestClassesAndStyles() {
  WebImage img;
  assert(!img.HasCssClass("rounded"));
  img.AddCssClass("rounded");
  assert(img.HasCssClass("rounded"));
  img.AddCssClass("rounded");  // duplicate should not create weirdness
  assert(img.HasCssClass("rounded"));
  img.RemoveCssClass("rounded");
  assert(!img.HasCssClass("rounded"));

  img.SetStyle("border", "1px solid red");
  img.SetStyle("pointerEvents", "none");
  img.ClearStyle("border");
  // No direct getter for styles (intentional); this is still valuable coverage:
  // it ensures calls don't crash and handles empty keys safely.
  img.SetStyle("", "ignored");
  img.ClearStyle("");
}

static void TestLifecycleMove() {
  WebImage a("a.png", "a");
  assert(!a.IsCreated());
  a.EnsureCreated();
  assert(a.IsCreated());
  assert(a.GetHandle() != 0);

  WebImage b(std::move(a));
  assert(b.IsCreated());
  assert(b.GetHandle() != 0);
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);

  b.Destroy();
  assert(!b.IsCreated());
}

int main() {
  TestDefaults();
  TestSetters();
  TestOpacityClamp();
  TestClassesAndStyles();
  TestLifecycleMove();

  std::cout << "WebImage tests passed.\n";
  return 0;
}


static void TestRemoveFromDomAndDestroyRecreate() {
  WebImage img("assets/test.png", "demo");
  img.SetParentElementId("root");
  img.SetElementId("demo-img");

  img.EnsureCreated();
  assert(img.IsCreated());
  const uintptr_t h1 = img.GetHandle();
  assert(h1 != 0);

  // Remove from DOM should not "un-create" the object.
  img.RemoveFromDom();
  assert(img.IsCreated());
  assert(img.GetHandle() == h1);

  // Destroy should reset the object back to "not created".
  img.Destroy();
  assert(!img.IsCreated());
  assert(img.GetHandle() == 0);

  // Should be able to recreate cleanly.
  img.EnsureCreated();
  assert(img.IsCreated());
  const uintptr_t h2 = img.GetHandle();
  assert(h2 != 0);
  assert(h2 != h1);  // expected if your native stub increments handles
}

static void TestMoveAssign() {
  WebImage a("a.png", "a");
  a.EnsureCreated();
  const uintptr_t h = a.GetHandle();
  assert(h != 0);

  WebImage b("b.png", "b");
  b = std::move(a);

  assert(b.IsCreated());
  assert(b.GetHandle() == h);

  // moved-from object should be safe to use/destroy
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);
}
