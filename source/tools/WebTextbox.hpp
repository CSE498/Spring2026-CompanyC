/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebTextbox - manages an HTML text block (div) from C++.
 * Handles font, color, position, and content through a
 * simple interface that works with Emscripten or natively.
 * Citation - LLM (OpenAI) was used to help generate parts of this file,
 * and maintain consistency with the project. The code was then reviewed
 * and heavily edited by the author to ensure correctness and suitability
 * for the project.
 *
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>

namespace cse498 {

class WebTextbox final {
 public:
  // named defaults so we avoid magic numbers scattered everywhere
  static constexpr int kDefaultFontSize = 16;
  static constexpr double kDefaultLineHeight = 1.4;
  static constexpr double kDefaultOpacity = 1.0;
  static constexpr double kMinOpacity = 0.0;
  static constexpr double kMaxOpacity = 1.0;

  WebTextbox();
  ~WebTextbox();

  // no copies -- one DOM element per object
  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  // moves are fine though
  WebTextbox(WebTextbox&& other) noexcept;
  WebTextbox& operator=(WebTextbox&& other) noexcept;

  // -- Content --
  void SetText(const std::string& text);
  [[nodiscard]] const std::string& GetText() const noexcept;
  void AppendText(const std::string& text);
  void ClearText();

  // -- Font / Typography --
  void SetFontFamily(const std::string& family);
  [[nodiscard]] const std::string& GetFontFamily() const noexcept;
  void SetFontSize(int size_px);
  [[nodiscard]] int GetFontSize() const noexcept;
  void SetFontWeight(const std::string& weight);
  [[nodiscard]] const std::string& GetFontWeight() const noexcept;
  void SetFontStyle(const std::string& style);
  [[nodiscard]] const std::string& GetFontStyle() const noexcept;
  void SetLineHeight(double multiplier);
  [[nodiscard]] double GetLineHeight() const noexcept;

  // -- Colors --
  void SetTextColor(const std::string& color);
  [[nodiscard]] const std::string& GetTextColor() const noexcept;
  void SetBackgroundColor(const std::string& color);
  [[nodiscard]] const std::string& GetBackgroundColor() const noexcept;

  // -- Text decoration & wrapping --
  void SetTextDecoration(const std::string& decoration);
  [[nodiscard]] const std::string& GetTextDecoration() const noexcept;
  void SetWordWrap(const std::string& wrap_mode);
  [[nodiscard]] const std::string& GetWordWrap() const noexcept;

  // -- Alignment --
  void SetTextAlignment(const std::string& alignment);
  [[nodiscard]] const std::string& GetTextAlignment() const noexcept;

  // -- Layout --
  void SetPosition(double left_px, double top_px);
  [[nodiscard]] double GetLeftPx() const noexcept;
  [[nodiscard]] double GetTopPx() const noexcept;
  void SetSize(double width_px, double height_px);
  [[nodiscard]] double GetWidthPx() const noexcept;
  [[nodiscard]] double GetHeightPx() const noexcept;
  void SetPadding(double top, double right, double bottom, double left);
  [[nodiscard]] const std::string& GetPadding() const noexcept;

  // -- Visibility --
  void SetVisible(bool visible);
  [[nodiscard]] bool IsVisible() const noexcept;
  void SetOpacity(double opacity);
  [[nodiscard]] double GetOpacity() const noexcept;

  // -- DOM identity --
  void SetElementId(const std::string& id);
  [[nodiscard]] const std::string& GetElementId() const noexcept;
  void SetParentId(const std::string& parent_id);
  [[nodiscard]] const std::string& GetParentId() const noexcept;

  // -- Lifecycle --
  void EnsureCreated();
  void RemoveFromDom();
  void Destroy();
  [[nodiscard]] bool IsCreated() const noexcept;
  [[nodiscard]] int32_t GetHandle() const noexcept;

 private:
  void MoveFrom_(WebTextbox&& other) noexcept;

  // pushes all stored state to the DOM at once
  void SyncAll_();
  // sets a single css property on the underlying element
  void PushStyle_(const std::string& prop, const std::string& val);
  // updates the text content shown in the div
  void PushText_();
  // attaches element to parent (or body if empty)
  void PushParent_();
  // sets the element's id attribute in the DOM
  void PushElementId_();

  static double ClampOpacity_(double v) noexcept;

  int32_t handle_ = 0;
  bool created_ = false;

  std::string text_;
  std::string font_family_;
  int font_size_px_ = kDefaultFontSize;
  std::string font_weight_ = "normal";
  std::string font_style_ = "normal";
  double line_height_ = kDefaultLineHeight;

  std::string text_color_;
  std::string bg_color_;
  std::string text_decoration_;
  std::string word_wrap_ = "normal";
  std::string text_align_ = "left";
  std::string padding_;

  double left_px_ = 0.0;
  double top_px_ = 0.0;
  double width_px_ = 0.0;   // 0 means auto
  double height_px_ = 0.0;
  bool visible_ = true;
  double opacity_ = kDefaultOpacity;

  std::string element_id_;
  std::string parent_id_;
};

}  // namespace cse498
