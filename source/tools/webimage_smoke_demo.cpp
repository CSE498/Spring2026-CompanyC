/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Smoke demo to validate WebImage under Emscripten/DOM.
 * @note Status: PROPOSAL
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 **/

#include "tools/WebImage.h"

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
#endif

using cse498::WebImage;

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
extern "C" void RunWebImageSmokeDemo() {
  // Keep alive: if this were local, destructor would remove DOM element immediately.
  static WebImage img("assets/test.png", "test image");

  img.SetParentElementId("root");
  img.SetElementId("demo-img");

  img.SetPositionPx(40, 40);
  img.SetSizePx(180, 180);
  img.SetZIndex(10);
  img.SetOpacity(0.95);

  img.AddCssClass("demo-border");
  img.SetStyle("borderRadius", "16px");
  img.SetStyle("boxShadow", "0 8px 30px rgba(0,0,0,0.25)");
  img.SetStyle("outline", "3px solid red");

  img.EnsureCreated();
  img.SetVisible(true);

#ifdef __EMSCRIPTEN__
  // IMPORTANT: pass a dummy arg to avoid the C++17 variadic-macro warning under -Werror
  EM_ASM({
    setTimeout(() => {
      const el = document.getElementById('demo-img');
      if (!el) return;
      el.style.left = '280px';
      el.style.top  = '140px';
      el.style.transform = 'rotate(10deg)';
    }, 800);
  }, 0);
#endif
}
