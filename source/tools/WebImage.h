#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498 {

/**
 * WebImage
 * -------
 * A small wrapper around an HTML <img> element, controllable from C++.
 *
 * Goals (per course spec):
 *  - simplify asset loading and usage
 *  - allow size, position, alt text, and other basic presentation controls from C++
 *
 * Design notes:
 *  - Under Emscripten (__EMSCRIPTEN__), this owns a real DOM element.
 *  - Under native compilation, this is a "headless" implementation (stateful + testable).
 *  - Copy is disabled to avoid double-owning the DOM element; move is supported.
 * 
 * 
 * citation - ChatGPT LLM (OpenAI) was used to help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Sadwal Patel
 * 
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 */
class WebImage final {
 public:
  WebImage();
  explicit WebImage(std::string src, std::string alt_text = {});
  ~WebImage();

  WebImage(const WebImage&) = delete;
  WebImage& operator=(const WebImage&) = delete;

  WebImage(WebImage&& other) noexcept;
  WebImage& operator=(WebImage&& other) noexcept;

  // --- Core properties ---
  void SetSource(std::string src);
  const std::string& GetSource() const noexcept;

  void SetAltText(std::string alt_text);
  const std::string& GetAltText() const noexcept;

  // Position in pixels (CSS left/top).
  void SetPositionPx(double left_px, double top_px);
  double GetLeftPx() const noexcept;
  double GetTopPx() const noexcept;

  // Size in pixels (CSS width/height). If <= 0 => "auto" (style cleared).
  void SetSizePx(double width_px, double height_px);
  double GetWidthPx() const noexcept;
  double GetHeightPx() const noexcept;

  void SetVisible(bool visible);
  bool IsVisible() const noexcept;

  // Opacity is clamped to [0, 1].
  void SetOpacity(double opacity_0_to_1);
  double GetOpacity() const noexcept;

  void SetZIndex(int z_index);
  int GetZIndex() const noexcept;

  // Parent element by id. Empty => document.body.
  void SetParentElementId(std::string parent_id);
  const std::string& GetParentElementId() const noexcept;

  // DOM element id. Empty => no id attribute.
  void SetElementId(std::string element_id);
  const std::string& GetElementId() const noexcept;

  // Classes and styles.
  void AddCssClass(const std::string& css_class);
  void RemoveCssClass(const std::string& css_class);
  bool HasCssClass(const std::string& css_class) const;

  void SetStyle(const std::string& property, const std::string& value);
  void ClearStyle(const std::string& property);

  // --- Lifecycle ---
  /**
   * Ensure that the underlying element exists.
   *
   * - If not yet created, this will create the underlying element and apply all current state.
   * - If already created, this is safe to call again (idempotent).
   * - After Destroy(), calling EnsureCreated() will create a brand-new element/handle and re-apply state.
   */
  void EnsureCreated();

  /**
   * Detach the element from the DOM without destroying it.
   *
   * The element/handle remains valid (IsCreated() stays true). This is useful for temporarily removing
   * an element from the page without losing its loaded source/state.
   *
   * To attach it again, call SetParentElementId(...) (even with the same parent id) so the element is
   * re-appended to that parent.
   */
  void RemoveFromDom();

  /**
   * Destroy the underlying element and invalidate the handle.
   *
   * After Destroy(), IsCreated() becomes false and GetHandle() becomes 0. You can call EnsureCreated()
   * later to recreate a new element and re-apply the stored state.
   */
  void Destroy();

  bool IsCreated() const noexcept;

  // Under Emscripten this is a JS-side handle to the element; under native, it's a stable id.
  std::int32_t GetHandle() const noexcept;

 private:
  void MoveFrom_(WebImage&& other) noexcept;

  void SyncAllToDom_();
  void SyncSource_();
  void SyncAlt_();
  void SyncGeometry_();
  void SyncVisibility_();
  void SyncOpacity_();
  void SyncZIndex_();
  void SyncParent_();
  void SyncId_();
  void SyncClasses_();
  void SyncStyles_();

  static double Clamp01_(double v) noexcept;

  std::int32_t handle_ = 0;
  bool created_ = false;

  std::string src_;
  std::string alt_text_;
  double left_px_ = 0.0;
  double top_px_ = 0.0;
  double width_px_ = 0.0;   // <= 0 means auto
  double height_px_ = 0.0;  // <= 0 means auto
  bool visible_ = true;
  double opacity_ = 1.0;
  int z_index_ = 0;

  std::string parent_id_;
  std::string element_id_;

  std::vector<std::string> classes_;
  std::unordered_map<std::string, std::string> styles_;
};

}  // namespace cse498