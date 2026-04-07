#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cse498 {

/**
 * Compatibility macro for nodiscard.
 *
 * Some toolchains used in the course environment warn if [[nodiscard]]
 * is applied even when compiling with -std=c++17. We therefore only enable
 * the attribute when the compiler clearly reports support for it.
 */
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard) && (__cplusplus >= 201703L)
#    define CSE498_NODISCARD [[nodiscard]]
#  else
#    define CSE498_NODISCARD
#  endif
#else
#  define CSE498_NODISCARD
#endif

/**
 * @class WebImage
 * @brief Manage an HTML <img> element from C++ code.
 * @author Sadwal Patel
 *
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 *
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this file.
 * The code was then reviewed and heavily edited by the author to ensure
 * correctness and suitability for the project.
 *
 * This class is Group 24's image wrapper for the web interface module.
 * It provides a simple C++ API for common image-related tasks such as:
 *   - choosing the image source asset
 *   - setting alt text
 *   - positioning the image in pixels
 *   - controlling size, visibility, opacity, z-index, and CSS classes
 *   - attaching the element to a parent DOM node
 *
 * When compiled with Emscripten, the class controls a real DOM image
 * through a small JavaScript bridge. When compiled natively, it keeps the
 * same state in memory with no-op bridge calls so that it can still be
 * tested with normal C++ unit tests.
 *
 * Advanced C++ topics represented here include:
 *   - value semantics through a move-only resource-owning type
 *   - constexpr constants for stable defaults and bounds
 *   - a small template helper for deferred DOM callbacks
 *   - lambdas used to bridge cached state changes into DOM synchronization
 *
 * The class is move-only because it conceptually owns one DOM element.
 */
class WebImage final {
 public:
  /// Construct an empty image wrapper with default state.
  WebImage();

  /**
   * Construct an image wrapper with an initial source and optional alt text.
   * @param src Image source path or URL.
   * @param alt_text Alternative text for accessibility.
   */
  explicit WebImage(std::string src, std::string alt_text = {});

  /// Destroy the underlying element if it has been created.
  ~WebImage();

  WebImage(const WebImage&) = delete;
  WebImage& operator=(const WebImage&) = delete;

  /// Move constructor. Ownership of the underlying element moves to *this.
  WebImage(WebImage&& other) noexcept;

  /// Move assignment. Existing owned element is destroyed before takeover.
  WebImage& operator=(WebImage&& other) noexcept;

  /// Set the image source path or URL.
  void SetSource(std::string src);

  /// Get the current image source string.
  CSE498_NODISCARD const std::string& GetSource() const noexcept;

  /// Set alternative text for the image.
  void SetAltText(std::string alt_text);

  /// Get the current alternative text.
  CSE498_NODISCARD const std::string& GetAltText() const noexcept;

  /**
   * Set the CSS left/top position in pixels.
   * @param left_px Horizontal offset in pixels.
   * @param top_px Vertical offset in pixels.
   */
  void SetPositionPx(double left_px, double top_px);

  /// Get the current left position in pixels.
  CSE498_NODISCARD double GetLeftPx() const noexcept;

  /// Get the current top position in pixels.
  CSE498_NODISCARD double GetTopPx() const noexcept;

  /**
   * Set the CSS width/height in pixels.
   *
   * A value of 0 means "auto" for that dimension. Negative values are treated
   * as invalid input in debug builds and are sanitized to 0 in normal builds
   * so that the object never stores an invalid visual size.
   *
   * Examples:
   *   SetSizePx(320, 240) -> fixed size
   *   SetSizePx(0, 240)   -> auto width, fixed height
   *   SetSizePx(0, 0)     -> auto width and auto height
   */
  void SetSizePx(double width_px, double height_px);

  /// Get the stored width in pixels. A value of 0 means "auto".
  CSE498_NODISCARD double GetWidthPx() const noexcept;

  /// Get the stored height in pixels. A value of 0 means "auto".
  CSE498_NODISCARD double GetHeightPx() const noexcept;

  /// Show or hide the image.
  void SetVisible(bool visible);

  /// Return true if the image should currently be visible.
  CSE498_NODISCARD bool IsVisible() const noexcept;

  /**
   * Set the image opacity.
   * Values are clamped into the range [0, 1].
   */
  void SetOpacity(double opacity_0_to_1);

  /// Get the current opacity in the range [0, 1].
  CSE498_NODISCARD double GetOpacity() const noexcept;

  /// Set the CSS z-index.
  void SetZIndex(int z_index);

  /// Get the current z-index.
  CSE498_NODISCARD int GetZIndex() const noexcept;

  /**
   * Set the parent DOM element id.
   * An empty string means that the image should attach to document.body.
   */
  void SetParentElementId(std::string parent_id);

  /// Get the current parent element id string.
  CSE498_NODISCARD const std::string& GetParentElementId() const noexcept;

  /**
   * Set the DOM id attribute of the underlying image element.
   * An empty string removes the id attribute.
   */
  void SetElementId(std::string element_id);

  /// Get the current DOM element id string.
  CSE498_NODISCARD const std::string& GetElementId() const noexcept;

  /// Add a CSS class if it is non-empty and not already present.
  void AddCssClass(const std::string& css_class);

  /// Remove a CSS class if it is present.
  void RemoveCssClass(const std::string& css_class);

  /// Return true if the image currently tracks the given CSS class.
  CSE498_NODISCARD bool HasCssClass(const std::string& css_class) const;

  /// Set a style property to the provided value.
  void SetStyle(const std::string& property, const std::string& value);

  /// Clear a tracked style property.
  void ClearStyle(const std::string& property);

  /**
   * Ensure that the underlying element exists.
   *
   * If the element has not yet been created, this function creates it and
   * synchronizes all currently stored state to the DOM. If it already exists,
   * the function does nothing.
   */
  void EnsureCreated();

  /**
   * Detach the element from the DOM without destroying it.
   *
   * After this call, IsCreated() remains true and the handle is still valid.
   * The element can be attached again later by syncing the parent.
   */
  void RemoveFromDom();

  /**
   * Destroy the underlying element and invalidate the handle.
   *
   * Stored C++ state remains available so that a later EnsureCreated() call
   * can recreate the DOM element and reapply the current state.
   */
  void Destroy();

  /// Return true if the underlying element/handle currently exists.
  CSE498_NODISCARD bool IsCreated() const noexcept;

  /**
   * Return the underlying handle.
   *
   * Under Emscripten, this is a JavaScript-side identifier.
   * Under native compilation, this is a stable test-friendly id.
   */
  CSE498_NODISCARD std::int32_t GetHandle() const noexcept;

 private:
  // ---------------------------------------------------------------------------
  // Stable defaults and bounds
  //
  // These constants are kept near the top of the class to make default behavior
  // easy to find and to avoid scattering visual defaults across the file.
  // ---------------------------------------------------------------------------
  static constexpr double kAutoSizePx = 0.0;
  static constexpr double kDefaultLeftPx = 0.0;
  static constexpr double kDefaultTopPx = 0.0;
  static constexpr double kDefaultOpacity = 1.0;
  static constexpr int kDefaultZIndex = 0;
  static constexpr double kMinOpacity = 0.0;
  static constexpr double kMaxOpacity = 1.0;

  /// Move all state from another WebImage into this one.
  void MoveFrom_(WebImage&& other) noexcept;

  /// Synchronize all stored state to the DOM element.
  void SyncAllToDom_();

  /// Synchronize the source attribute.
  void SyncSource_();

  /// Synchronize the alt attribute.
  void SyncAlt_();

  /// Synchronize geometry-related fields.
  void SyncGeometry_();

  /// Synchronize visibility.
  void SyncVisibility_();

  /// Synchronize opacity.
  void SyncOpacity_();

  /// Synchronize z-index.
  void SyncZIndex_();

  /// Synchronize the parent element relationship.
  void SyncParent_();

  /// Synchronize the DOM id attribute.
  void SyncId_();

  /// Synchronize tracked CSS classes.
  void SyncClasses_();

  /// Synchronize tracked inline styles.
  void SyncStyles_();

  /// Clamp an opacity value into the range [0, 1].
  static double Clamp01_(double value) noexcept;

  /**
   * Run a callback only when a DOM-backed handle currently exists.
   *
   * This small template is intentionally kept inline in the header so the
   * compiler can optimize simple lambda-based bridge calls with no extra
   * indirection.
   */
  template <typename Callback>
  void WithCreatedHandle_(Callback&& callback) {
    if (!created_) {
      return;
    }
    std::forward<Callback>(callback)(handle_);
  }

  std::int32_t handle_ = 0;
  bool created_ = false;

  std::string src_;
  std::string alt_text_;
  double left_px_ = kDefaultLeftPx;
  double top_px_ = kDefaultTopPx;
  double width_px_ = kAutoSizePx;   ///< 0 means auto width
  double height_px_ = kAutoSizePx;  ///< 0 means auto height
  bool visible_ = true;
  double opacity_ = kDefaultOpacity;
  int z_index_ = kDefaultZIndex;

  std::string parent_id_;
  std::string element_id_;

  std::vector<std::string> classes_;
  std::unordered_map<std::string, std::string> styles_;
};

#undef CSE498_NODISCARD

}  // namespace cse498