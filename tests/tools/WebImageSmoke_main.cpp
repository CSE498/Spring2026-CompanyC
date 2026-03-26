/**
 * @file WebImageSmoke_main.cpp
 * @author Sadwal Patel
 * @brief Emscripten smoke-test driver for the Group 24 WebImage class.
 *
 * This file creates a minimal browser page and uses the WebImage wrapper to
 * render a visible image element from C++. It is intended as a lightweight
 * verification that the WebImage class correctly creates, styles, positions,
 * and displays an HTML <img> element through the Emscripten JavaScript bridge.
 *
 * The smoke test does not represent the full Group 24 UI module. Its purpose is
 * only to validate the standalone behavior of WebImage in a browser runtime.
 *
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 *
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this
 * file. The code was then reviewed and heavily edited by the author to ensure
 * correctness and suitability for the project.
 */



#include "tools/WebImage.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <string>

#ifdef __EMSCRIPTEN__
EM_JS(void, SetupWebImageSmokePage, (), {
  document.body.innerHTML = "";
  document.body.style.margin = "0";
  document.body.style.fontFamily = "Arial, sans-serif";
  document.body.style.background = "#f4f6f8";

  const wrapper = document.createElement("div");
  wrapper.id = "smoke-root";
  wrapper.style.minHeight = "100vh";
  wrapper.style.display = "flex";
  wrapper.style.flexDirection = "column";
  wrapper.style.alignItems = "center";
  wrapper.style.justifyContent = "center";
  wrapper.style.gap = "16px";

  const title = document.createElement("h1");
  title.textContent = "WebImage Smoke Test";
  title.style.margin = "0";
  title.style.fontSize = "28px";
  title.style.color = "#1f2937";

  const subtitle = document.createElement("div");
  subtitle.textContent = "Group 24 Emscripten image wrapper check";
  subtitle.style.fontSize = "16px";
  subtitle.style.color = "#4b5563";

  const imageHost = document.createElement("div");
  imageHost.id = "image-host";
  imageHost.style.position = "relative";
  imageHost.style.width = "320px";
  imageHost.style.height = "180px";

  wrapper.appendChild(title);
  wrapper.appendChild(subtitle);
  wrapper.appendChild(imageHost);
  document.body.appendChild(wrapper);
});
#endif

int main() {
#ifdef __EMSCRIPTEN__
  SetupWebImageSmokePage();
#endif

  cse498::WebImage img;

  const std::string svg =
      "data:image/svg+xml;utf8,"
      "<svg xmlns='http://www.w3.org/2000/svg' width='240' height='120'>"
      "<rect width='240' height='120' fill='%232b6cb0'/>"
      "<rect x='6' y='6' width='228' height='108' fill='none' stroke='white' stroke-width='4'/>"
      "<text x='120' y='55' font-size='24' text-anchor='middle' fill='white' font-family='Arial'>"
      "WebImage"
      "</text>"
      "<text x='120' y='85' font-size='16' text-anchor='middle' fill='white' font-family='Arial'>"
      "Group 24 smoke test"
      "</text>"
      "</svg>";

  img.SetSource(svg);
  img.SetAltText("Group 24 WebImage smoke test");
  img.SetElementId("group24-webimage-smoke");
  img.SetParentElementId("image-host");
  img.SetPositionPx(40.0, 30.0);
  img.SetSizePx(240.0, 120.0);
  img.SetVisible(true);
  img.SetOpacity(1.0);
  img.SetZIndex(10);
  img.SetStyle("border", "3px solid #111");
  img.SetStyle("boxShadow", "0 8px 20px rgba(0,0,0,0.25)");
  img.SetStyle("backgroundColor", "#ffffff");

  img.EnsureCreated();

#ifdef __EMSCRIPTEN__
  emscripten_exit_with_live_runtime();
#endif

  return 0;
}