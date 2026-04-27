/**
 * @file AppUI.cpp
 * @brief Implements the Company C landing page UI composition.
 *
 * Spring 2026, CSE 498 Sec 2 - Company C
 * Uses WebTextbox, WebButton, WebImage, and WebCanvas to build
 * a cohesive landing page.
 * Citation - ChatGPT LLM (OpenAI) was used to plan the structure of this file and help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 *
 * @author Group 24 Web Interface - Prijam Khanal
 * Copyright (c) 2026 Group 24
 * SPDX-License-Identifier: MIT
 */

#include "AppUI.hpp"

#include "tools/WebButton.hpp"
#include "tools/WebImage.hpp"
#include "tools/WebTextbox.hpp"
#include "tools/WebCanvas.hpp"

#include <string>

namespace cse498 {

namespace AppUI {

namespace {

// Layout constants for consistent positioning.
constexpr int kContentWidth = 600;

// Pointer for button callback (static locals cannot be captured by lambda).
cse498::WebTextbox* g_click_message = nullptr;

constexpr int kCanvasWidth = 800;
constexpr int kCanvasHeight = 450;
constexpr double kHeadingTop = 80.0;
constexpr double kImageTop = 140.0;
constexpr double kSubtitleTop = 260.0;
constexpr double kButtonTop = 320.0;

}  // namespace

void Run() {
  // WebCanvas: decorative background with geometric shapes.
  static cse498::WebCanvas canvas("hero-canvas", kCanvasWidth, kCanvasHeight);
  canvas.AppendTo("canvas-bg");
  canvas.Clear();
  canvas.SetFillColor("rgba(26, 26, 46, 0.95)");
  canvas.FillRect(0, 0, static_cast<float>(kCanvasWidth),
                 static_cast<float>(kCanvasHeight));
  canvas.SetFillColor("rgba(232, 232, 232, 0.08)");
  for (int i = 0; i < 12; ++i) {
    const float cx = 80.0f + i * 65.0f;
    const float cy = 100.0f + (i % 3) * 120.0f;
    canvas.FillCircle(cx, cy, 20.0f);
  }
  canvas.SetFillColor("rgba(78, 205, 196, 0.15)");
  canvas.FillCircle(700.0f, 80.0f, 60.0f);
  canvas.FillCircle(100.0f, 350.0f, 50.0f);

  // WebTextbox: main heading.
  static cse498::WebTextbox heading;
  heading.SetText("Company C");
  heading.SetFontFamily("Georgia, serif");
  heading.SetFontSize(48);
  heading.SetFontWeight("bold");
  heading.SetTextColor("#e8e8e8");
  heading.SetBackgroundColor("transparent");
  heading.SetTextAlignment("center");
  heading.SetPosition((kContentWidth - 400) / 2.0, kHeadingTop);
  heading.SetSize(400, 70);
  heading.SetParentId("content");
  heading.SetElementId("app-heading");
  heading.EnsureCreated();
  heading.SetVisible(true);

  // WebImage: Company C logo (have placed company_c_logo.png in web_app/assets/).
  static cse498::WebImage logo("assets/company_c_logo.png", "Company C logo");
  logo.SetParentElementId("content");
  logo.SetElementId("app-logo");
  logo.SetPositionPx((kContentWidth - 120) / 2.0, kImageTop);
  logo.SetSizePx(150, 120);
  logo.SetOpacity(0.95);
  logo.SetStyle("borderRadius", "16px");
  logo.SetStyle("boxShadow", "0 8px 24px rgba(0,0,0,0.3)");
  logo.EnsureCreated();
  logo.SetVisible(true);

  // WebTextbox: subtitle.
  static cse498::WebTextbox subtitle;
  subtitle.SetText("Web Interface - Group 24");
  subtitle.SetFontFamily("'Segoe UI', Tahoma, sans-serif");
  subtitle.SetFontSize(18);
  subtitle.SetTextColor("#b0b0b0");
  subtitle.SetBackgroundColor("transparent");
  subtitle.SetTextAlignment("center");
  subtitle.SetPosition((kContentWidth - 300) / 2.0, kSubtitleTop);
  subtitle.SetSize(300, 40);
  subtitle.SetParentId("content");
  subtitle.SetElementId("app-subtitle");
  subtitle.EnsureCreated();
  subtitle.SetVisible(true);

  // WebButton: call-to-action with click handler.
  static cse498::WebTextbox message;
  message.SetParentId("content");
  message.SetElementId("app-message");
  message.SetPosition((kContentWidth - 280) / 2.0, kButtonTop + 50);
  message.SetSize(280, 36);
  message.SetFontSize(16);
  message.SetTextColor("#4ecdc4");
  message.SetTextAlignment("center");
  message.SetVisible(false);

  g_click_message = &message;
  static cse498::WebButton button("Explore");
  button.SetSize(140, 44);
  button.SetBackgroundColor("#4ecdc4");
  button.SetTextColor("#1a1a2e");
  button.SetBorder(0, "#4ecdc4", 8);
  button.SetOnClick([]() {
    if (g_click_message) {
      g_click_message->SetText("Welcome to Company C!");
      g_click_message->EnsureCreated();
      g_click_message->SetVisible(true);
    }
  });
  button.AppendTo("content");
}

}  // namespace AppUI
}  // namespace cse498
