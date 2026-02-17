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
  // Keep it alive for the entire runtime (no destructor called immediately).
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
  img.SetStyle("outline", "3px solid red"); // extra obvious

  img.EnsureCreated();
  img.SetVisible(true); // force a visible-sync after creation

#ifdef __EMSCRIPTEN__
  // Prove it updates live
  EM_ASM_({
    setTimeout(() => {
      const el = document.getElementById('demo-img');
      if (!el) return;
      el.style.left = '280px';
      el.style.top  = '140px';
      el.style.transform = 'rotate(10deg)';
    }, 800);
  });
#endif
}
