/**
 * @file WebCanvas.hpp
 * @author Sadwal Patel
 * @brief Declaration of the WebCanvas wrapper used to manage an HTML canvas
 *        element for Group 24's browser-based interface.
 *
 * This class provides reusable drawing and interaction functionality for the
 * module, including primitive rendering, grid/cell helpers, entity drawing,
 * coordinate mapping, and click handling.
 *
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 *
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this
 * file. The code was then reviewed and heavily edited by the author to ensure
 * correctness and suitability for the project.
 */



#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {

class WebCanvas {
 public:
  WebCanvas(const std::string& canvas_id, int width, int height);
  ~WebCanvas() = default;

  const std::string& GetElementID() const noexcept { return id_; }
  int GetWidth() const noexcept { return width_; }
  int GetHeight() const noexcept { return height_; }

  void AppendTo(const std::string& parent_id);
  void Resize(int new_width, int new_height);
  bool IsReady() const noexcept;

  void Clear();
  void SetFillColor(const std::string& css);
  void SetStrokeColor(const std::string& css);
  void SetLineWidth(float width);
  void SetFont(const std::string& css_font);

  void FillRect(float x, float y, float width, float height);
  void StrokeRect(float x, float y, float width, float height);
  void DrawLine(float x1, float y1, float x2, float y2);
  void FillCircle(float cx, float cy, float radius);
  void DrawText(float x, float y, const std::string& text);

  void DrawCell(int col, int row, int cell_width, int cell_height,
                const std::string& fill_css,
                const std::string& glyph = std::string(),
                const std::string& glyph_css = "#000000");

  void DrawEntity(float center_x, float center_y, float radius,
                  const std::string& fill_css,
                  const std::string& glyph = std::string(),
                  const std::string& glyph_css = "#ffffff");

  void Present();

  void SetClickHandler(const std::function<void(int, int)>& handler);
  std::pair<int, int> PixelToCell(int pixel_x, int pixel_y,
                                  int cell_width, int cell_height) const;

  // Public on purpose: browser click bridge and native tests can both route
  // clicks through the same path.
  void HandleClick(int pixel_x, int pixel_y);

 private:
#ifdef __EMSCRIPTEN__
  void InitDOM();
  void EnsureReady_() const;
  void RegisterClickBridge_();
#endif

  std::string id_;
  int width_;
  int height_;
  std::function<void(int, int)> click_handler_;

#ifdef __EMSCRIPTEN__
  emscripten::val canvas_;
  emscripten::val ctx_;
  bool click_bridge_registered_ = false;
#endif
};

}  // namespace cse498
