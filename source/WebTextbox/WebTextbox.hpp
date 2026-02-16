/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebTextbox - manages an HTML text block (div) from C++.
 * Handles font, color, position, and content through a
 * simple interface that works with Emscripten or natively.
 * Citation - LLM (OpenAI) was used to help generate parts of this file, and maintain consistency with the project. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 *
 * @author Prijam Khanal
 */

#pragma once

#include <cstdint>
#include <string>
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

class WebTextbox final {
 public:
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
  const std::string& GetText() const noexcept;
  void AppendText(const std::string& text);
  void ClearText();

  // -- Font / Typography --
  void SetFontFamily(const std::string& family);
  const std::string& GetFontFamily() const noexcept;
  void SetFontSize(int size_px);
  int GetFontSize() const noexcept;
  void SetFontWeight(const std::string& weight);
  const std::string& GetFontWeight() const noexcept;
  void SetFontStyle(const std::string& style);
  const std::string& GetFontStyle() const noexcept;
  void SetLineHeight(double multiplier);
  double GetLineHeight() const noexcept;

  // -- Colors --
  void SetTextColor(const std::string& color);
  const std::string& GetTextColor() const noexcept;
  void SetBackgroundColor(const std::string& color);
  const std::string& GetBackgroundColor() const noexcept;

  // -- Alignment --
  void SetTextAlignment(const std::string& alignment);
  const std::string& GetTextAlignment() const noexcept;

  // -- Layout --
  void SetPosition(double left_px, double top_px);
  double GetLeftPx() const noexcept;
  double GetTopPx() const noexcept;
  void SetSize(double width_px, double height_px);
  double GetWidthPx() const noexcept;
  double GetHeightPx() const noexcept;

  // -- Visibility --
  void SetVisible(bool visible);
  bool IsVisible() const noexcept;
  void SetOpacity(double opacity);
  double GetOpacity() const noexcept;

  // -- DOM identity --
  void SetElementId(const std::string& id);
  const std::string& GetElementId() const noexcept;
  void SetParentId(const std::string& parent_id);
  const std::string& GetParentId() const noexcept;

  // -- Lifecycle --
  void EnsureCreated();
  void Destroy();
  bool IsCreated() const noexcept;
  int32_t GetHandle() const noexcept;

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
  int font_size_px_ = 16;
  std::string font_weight_ = "normal";
  std::string font_style_ = "normal";
  double line_height_ = 1.4;

  std::string text_color_;
  std::string bg_color_;
  std::string text_align_ = "left";

  double left_px_ = 0.0;
  double top_px_ = 0.0;
  double width_px_ = 0.0;   // 0 means auto
  double height_px_ = 0.0;
  bool visible_ = true;
  double opacity_ = 1.0;

  std::string element_id_;
  std::string parent_id_;
};

}  // namespace cse498
