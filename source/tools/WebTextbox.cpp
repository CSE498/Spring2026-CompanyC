/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebTextbox implementation - Emscripten JS bridge + native stubs.
 *
 * Citation - LLM (OpenAI) was used to help generate parts of this file,
 * and maintain consistency with the project. The code was then reviewed
 * and heavily edited by the author to ensure correctness and suitability
 * for the project.
 *
 * Under Emscripten we create a real <div> in the DOM and control it
 * through a small JS bridge. Under native builds everything is a
 * no-op stub so we can still run unit tests without a brower.
 *
 * How to compile this file alone (native g++, from repository root). Use ONE
 * line, or if you break lines with backslash, the next line must NOT start
 * with * (star) or the shell will treat * as "all files in this folder":
 *   g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -O0 -g -I./source -c ./source/tools/WebTextbox.cpp
 *
 * How to run the full WebTextbox unit test suite (recommended):
 *   bash tests/run_webtextbox_tests.sh
 *   That script uses -std=c++20 -Wall -Wextra -Wpedantic -Werror and must
 *   finish with no compiler warnings and "All WebTextbox tests passed."
 *
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */

#include "WebTextbox.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

namespace {

// CSS value for sidebar-style embedding (avoids magic string in ApplyFlowLayoutStyles).
constexpr const char* kFlowLayoutMinHeightCss = "2.5em";

}  // namespace

// -------------------------------------------------------
// JS bridge (Emscripten) / native stubs
// -------------------------------------------------------

#ifdef __EMSCRIPTEN__

// Each EM_JS block defines a C-callable function whose body is JavaScript.
// We keep all the textbox DOM elements in a Module-level map so we can
// look them up by integer handle from C++ without needing string IDs.

/**
 * @brief Creates a new DOM <div> used as a textbox and returns its handle.
 * @return Integer handle used to reference this element from C++.
 */
EM_JS(int, cse498_wtb_create, (), {
  if (!Module.__cse498WebTextbox) {
    Module.__cse498WebTextbox = { nextId: 1, map: new Map() };
  }
  var st = Module.__cse498WebTextbox;
  var id = st.nextId++;
  var div = document.createElement('div');
  div.style.position = 'absolute';
  div.style.left = '0px';
  div.style.top = '0px';
  div.style.display = 'none';
  div.style.opacity = '1';
  st.map.set(id, div);
  return id;
});

/**
 * @brief Destroys the DOM element associated with the given handle.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 */
EM_JS(void, cse498_wtb_destroy, (int handle), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  if (el.parentNode) el.remove();
  st.map.delete(handle);
});

/**
 * @brief Detaches the DOM element associated with the handle from its parent.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 */
EM_JS(void, cse498_wtb_detach, (int handle), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  if (el.parentNode) el.remove();
});

/**
 * @brief Sets text content of the DOM element associated with the handle.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 * @param txt_ptr UTF-8 encoded C string containing the new text.
 */
EM_JS(void, cse498_wtb_set_text, (int handle, const char* txt_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (el) el.textContent = UTF8ToString(txt_ptr);
});

/**
 * @brief Sets a CSS style property on the DOM element.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 * @param prop_ptr UTF-8 encoded C string naming the style property (camelCase).
 * @param val_ptr UTF-8 encoded C string containing the style value.
 */
EM_JS(void, cse498_wtb_set_style, (int handle, const char* prop_ptr,
                                    const char* val_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  el.style[UTF8ToString(prop_ptr)] = UTF8ToString(val_ptr);
});

/**
 * @brief Sets or clears the DOM id attribute on the element.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 * @param id_ptr UTF-8 encoded C string for the id. Empty clears the attribute.
 */
EM_JS(void, cse498_wtb_set_id, (int handle, const char* id_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  var v = id_ptr ? UTF8ToString(id_ptr) : "";
  if (v.length === 0) el.removeAttribute('id');
  else el.id = v;
});

/**
 * @brief Attaches the element to a parent element (or document body).
 * @param handle Integer handle previously returned by cse498_wtb_create().
 * @param parent_id_ptr UTF-8 encoded C string of the parent id. Empty = body.
 */
EM_JS(void, cse498_wtb_attach, (int handle, const char* parent_id_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  var pid = parent_id_ptr ? UTF8ToString(parent_id_ptr) : "";
  var parent = (pid.length > 0) ? document.getElementById(pid) : null;
  if (!parent) parent = document.body;
  if (el.parentNode !== parent) parent.appendChild(el);
});

/**
 * @brief Sets visibility by toggling CSS display between block and none.
 * @param handle Integer handle previously returned by cse498_wtb_create().
 * @param visible Non-zero to show, zero to hide.
 */
EM_JS(void, cse498_wtb_set_visible, (int handle, int visible), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (el) el.style.display = visible ? 'block' : 'none';
});

#else  // native stubs for testing without emscripten

namespace {

int g_next_handle = 1;

/**
 * @brief Native stub: returns a unique handle for unit tests.
 * @return Integer handle.
 */
int cse498_wtb_create() { return g_next_handle++; }

/**
 * @brief Native stub: no-op destroy.
 * @param handle Unused.
 */
void cse498_wtb_destroy(int) {}

/**
 * @brief Native stub: no-op detach.
 * @param handle Unused.
 */
void cse498_wtb_detach(int) {}

/**
 * @brief Native stub: no-op text setter.
 * @param handle Unused.
 * @param txt Unused.
 */
void cse498_wtb_set_text(int, const char*) {}

/**
 * @brief Native stub: no-op style setter.
 * @param handle Unused.
 * @param prop Unused.
 * @param val Unused.
 */
void cse498_wtb_set_style(int, const char*, const char*) {}

/**
 * @brief Native stub: no-op id setter.
 * @param handle Unused.
 * @param id Unused.
 */
void cse498_wtb_set_id(int, const char*) {}

/**
 * @brief Native stub: no-op attach.
 * @param handle Unused.
 * @param parent_id Unused.
 */
void cse498_wtb_attach(int, const char*) {}

/**
 * @brief Native stub: no-op visibility setter.
 * @param handle Unused.
 * @param visible Unused.
 */
void cse498_wtb_set_visible(int, int) {}

}  // anonymous namespace

#endif

/**
 * @brief Constructs a WebTextbox in an uncreated (lazy) state.
 *
 * The underlying DOM element is not created until EnsureCreated() is called.
 */
WebTextbox::WebTextbox() = default;

/**
 * @brief Destroys the WebTextbox and releases any underlying resources.
 */
WebTextbox::~WebTextbox() {
  Destroy();
}

// -------------------------------------------------------
// Move operations
// -------------------------------------------------------

/**
 * @brief Move-constructs a WebTextbox, transferring ownership from another.
 * @param other Object to move from.
 */
WebTextbox::WebTextbox(WebTextbox&& other) noexcept {
  MoveFrom_(std::move(other));
}

/**
 * @brief Move-assigns a WebTextbox, destroying current resources first.
 * @param other Object to move from.
 * @return Reference to this object.
 */
WebTextbox& WebTextbox::operator=(WebTextbox&& other) noexcept {
  if (this == &other) return *this;
  Destroy();
  MoveFrom_(std::move(other));
  return *this;
}

/**
 * @brief Internal helper to transfer state from another WebTextbox.
 * @param other Object to move from.
 */
void WebTextbox::MoveFrom_(WebTextbox&& other) noexcept {
  handle_ = other.handle_;
  created_ = other.created_;

  text_ = std::move(other.text_);
  font_family_ = std::move(other.font_family_);
  font_size_px_ = other.font_size_px_;
  font_weight_ = std::move(other.font_weight_);
  font_style_ = std::move(other.font_style_);
  line_height_ = other.line_height_;

  text_color_ = std::move(other.text_color_);
  bg_color_ = std::move(other.bg_color_);
  text_decoration_ = std::move(other.text_decoration_);
  word_wrap_ = std::move(other.word_wrap_);
  text_align_ = std::move(other.text_align_);
  padding_ = std::move(other.padding_);

  left_px_ = other.left_px_;
  top_px_ = other.top_px_;
  width_px_ = other.width_px_;
  height_px_ = other.height_px_;
  visible_ = other.visible_;
  opacity_ = other.opacity_;

  element_id_ = std::move(other.element_id_);
  parent_id_ = std::move(other.parent_id_);

  // leave the source in a safe "empty" state
  other.handle_ = 0;
  other.created_ = false;
}

// Using advance feature: value semantics (snapshot is an independent copy of
// style fields; safe to pass around without sharing the WebTextbox DOM handle.)
WebTextbox::StyleSnapshot WebTextbox::CaptureStyle() const noexcept {
  StyleSnapshot s;
  s.text = text_;
  s.font_family = font_family_;
  s.font_size_px = font_size_px_;
  s.font_weight = font_weight_;
  s.font_style = font_style_;
  s.line_height = line_height_;
  s.text_color = text_color_;
  s.bg_color = bg_color_;
  s.text_decoration = text_decoration_;
  s.word_wrap = word_wrap_;
  s.text_align = text_align_;
  s.padding = padding_;
  s.left_px = left_px_;
  s.top_px = top_px_;
  s.width_px = width_px_;
  s.height_px = height_px_;
  s.visible = visible_;
  s.opacity = opacity_;
  s.element_id = element_id_;
  s.parent_id = parent_id_;
  return s;
}

// Using advance feature: value semantics (apply a copied snapshot into this
// object; optionally refreshes the DOM if the element already exists.)
void WebTextbox::ApplySnapshot(const StyleSnapshot& snapshot) {
  assert(snapshot.font_size_px > 0 && "snapshot font size must be positive");
  assert(snapshot.line_height > 0.0 && "snapshot line height must be positive");
  assert(snapshot.width_px >= 0.0 && snapshot.height_px >= 0.0);

  text_ = snapshot.text;
  font_family_ = snapshot.font_family;
  font_size_px_ = snapshot.font_size_px;
  font_weight_ = snapshot.font_weight;
  font_style_ = snapshot.font_style;
  line_height_ = snapshot.line_height;
  text_color_ = snapshot.text_color;
  bg_color_ = snapshot.bg_color;
  text_decoration_ = snapshot.text_decoration;
  word_wrap_ = snapshot.word_wrap;
  text_align_ = snapshot.text_align;
  padding_ = snapshot.padding;
  left_px_ = snapshot.left_px;
  top_px_ = snapshot.top_px;
  width_px_ = snapshot.width_px;
  height_px_ = snapshot.height_px;
  visible_ = snapshot.visible;
  opacity_ = ClampOpacity_(snapshot.opacity);
  element_id_ = snapshot.element_id;
  parent_id_ = snapshot.parent_id;
  if (created_) {
    SyncAll_();
  }
}

// -------------------------------------------------------
// Content
// -------------------------------------------------------
/**
 * @brief Gets the current textbox content.
 * @return Const reference to the current text.
 */
const std::string& WebTextbox::GetText() const noexcept {
  return text_;
}

/**
 * @brief Clears all textbox content.
 */
void WebTextbox::ClearText() {
  text_.clear();
  PushText_();
}

// -------------------------------------------------------
// Font / Typography
// -------------------------------------------------------

/**
 * @brief Sets the CSS font-family.
 * @param family CSS font family string (e.g., "Arial", "system-ui").
 */
void WebTextbox::SetFontFamily(const std::string& family) {
  font_family_ = family;
  if (created_) PushStyle_("fontFamily", font_family_);
}

/**
 * @brief Gets the current font-family setting.
 * @return Const reference to the font family string.
 */
const std::string& WebTextbox::GetFontFamily() const noexcept {
  return font_family_;
}

/**
 * @brief Sets the font size in pixels.
 * @param size_px Font size in pixels. Must be positive.
 */
void WebTextbox::SetFontSize(int size_px) {
  assert(size_px > 0 && "font size must be positive");
  font_size_px_ = size_px;
  if (created_) PushStyle_("fontSize", std::to_string(font_size_px_) + "px");
}

/**
 * @brief Gets the font size in pixels.
 * @return Font size in pixels.
 */
int WebTextbox::GetFontSize() const noexcept {
  return font_size_px_;
}

/**
 * @brief Sets the CSS font-weight.
 * @param weight CSS font-weight value (e.g., "normal", "bold", "600").
 */
void WebTextbox::SetFontWeight(const std::string& weight) {
  font_weight_ = weight;
  if (created_) PushStyle_("fontWeight", font_weight_);
}

/**
 * @brief Gets the CSS font-weight.
 * @return Const reference to the font-weight string.
 */
const std::string& WebTextbox::GetFontWeight() const noexcept {
  return font_weight_;
}

/**
 * @brief Sets the CSS font-style.
 * @param style CSS font-style value (e.g., "normal", "italic").
 */
void WebTextbox::SetFontStyle(const std::string& style) {
  font_style_ = style;
  if (created_) PushStyle_("fontStyle", font_style_);
}

/**
 * @brief Gets the CSS font-style.
 * @return Const reference to the font-style string.
 */
const std::string& WebTextbox::GetFontStyle() const noexcept {
  return font_style_;
}

/**
 * @brief Sets the line-height multiplier.
 * @param multiplier Line-height multiplier. Must be positive.
 */
void WebTextbox::SetLineHeight(double multiplier) {
  assert(multiplier > 0.0 && "line height must be positive");
  line_height_ = multiplier;
  if (created_) PushStyle_("lineHeight", std::to_string(line_height_));
}

/**
 * @brief Gets the line-height multiplier.
 * @return Line-height multiplier.
 */
double WebTextbox::GetLineHeight() const noexcept {
  return line_height_;
}

// -------------------------------------------------------
// Colors
// -------------------------------------------------------

/**
 * @brief Sets the CSS text color.
 * @param color CSS color string (e.g., "#fff", "rgb(...)", "red").
 */
void WebTextbox::SetTextColor(const std::string& color) {
  text_color_ = color;
  if (created_) PushStyle_("color", text_color_);
}

/**
 * @brief Gets the CSS text color.
 * @return Const reference to the text color string.
 */
const std::string& WebTextbox::GetTextColor() const noexcept {
  return text_color_;
}

/**
 * @brief Sets the CSS background color.
 * @param color CSS color string.
 */
void WebTextbox::SetBackgroundColor(const std::string& color) {
  bg_color_ = color;
  if (created_) PushStyle_("backgroundColor", bg_color_);
}

/**
 * @brief Gets the CSS background color.
 * @return Const reference to the background color string.
 */
const std::string& WebTextbox::GetBackgroundColor() const noexcept {
  return bg_color_;
}

// -------------------------------------------------------
// Text decoration & wrapping
// -------------------------------------------------------

/**
 * @brief Sets the CSS text-decoration.
 * @param decoration CSS text-decoration value (e.g., "none", "underline").
 */
void WebTextbox::SetTextDecoration(const std::string& decoration) {
  text_decoration_ = decoration;
  if (created_) PushStyle_("textDecoration", text_decoration_);
}

/**
 * @brief Gets the CSS text-decoration.
 * @return Const reference to the text-decoration string.
 */
const std::string& WebTextbox::GetTextDecoration() const noexcept {
  return text_decoration_;
}

/**
 * @brief Sets the word wrap mode (mapped to a CSS property).
 * @param wrap_mode Wrap mode string.
 */
void WebTextbox::SetWordWrap(const std::string& wrap_mode) {
  word_wrap_ = wrap_mode;
  if (created_) PushStyle_("wordWrap", word_wrap_);
}

/**
 * @brief Gets the word wrap mode string.
 * @return Const reference to the wrap mode string.
 */
const std::string& WebTextbox::GetWordWrap() const noexcept {
  return word_wrap_;
}

// -------------------------------------------------------
// Alignment
// -------------------------------------------------------

/**
 * @brief Sets the CSS text-align.
 * @param alignment CSS alignment string (e.g., "left", "center", "right").
 */
void WebTextbox::SetTextAlignment(const std::string& alignment) {
  text_align_ = alignment;
  if (created_) PushStyle_("textAlign", text_align_);
}

/**
 * @brief Gets the CSS text alignment.
 * @return Const reference to the alignment string.
 */
const std::string& WebTextbox::GetTextAlignment() const noexcept {
  return text_align_;
}

// -------------------------------------------------------
// Layout
// -------------------------------------------------------

/**
 * @brief Sets the absolute position of the element (in pixels).
 * @param left_px Left position in pixels.
 * @param top_px Top position in pixels.
 */
void WebTextbox::SetPosition(double left_px, double top_px) {
  left_px_ = left_px;
  top_px_ = top_px;
  if (created_) {
    PushStyle_("left", std::to_string(left_px_) + "px");
    PushStyle_("top", std::to_string(top_px_) + "px");
  }
}

/**
 * @brief Gets the left position in pixels.
 * @return Left position in pixels.
 */
double WebTextbox::GetLeftPx() const noexcept { return left_px_; }

/**
 * @brief Gets the top position in pixels.
 * @return Top position in pixels.
 */
double WebTextbox::GetTopPx() const noexcept { return top_px_; }

/**
 * @brief Sets the size of the element (in pixels).
 * @param width_px Width in pixels.
 * @param height_px Height in pixels.
 */
void WebTextbox::SetSize(double width_px, double height_px) {
  assert(width_px >= 0 && "width cannot be negative");
  assert(height_px >= 0 && "height cannot be negative");
  width_px_ = width_px;
  height_px_ = height_px;
  if (created_) {
    // 0 means auto, anything positive is an explicit pixel size
    PushStyle_("width",
               width_px_ > 0 ? std::to_string(width_px_) + "px" : "");
    PushStyle_("height",
               height_px_ > 0 ? std::to_string(height_px_) + "px" : "");
  }
}

/**
 * @brief Gets the width in pixels.
 * @return Width in pixels.
 */
double WebTextbox::GetWidthPx() const noexcept { return width_px_; }

/**
 * @brief Gets the height in pixels.
 * @return Height in pixels.
 */
double WebTextbox::GetHeightPx() const noexcept { return height_px_; }

/**
 * @brief Sets the padding of the element (in pixels).
 * @param top Top padding in pixels.
 * @param right Right padding in pixels.
 * @param bottom Bottom padding in pixels.
 * @param left Left padding in pixels.
 */
void WebTextbox::SetPadding(double top, double right,
                            double bottom, double left) {
  assert(top >= 0 && "top padding cannot be negative");
  assert(right >= 0 && "right padding cannot be negative");
  assert(bottom >= 0 && "bottom padding cannot be negative");
  assert(left >= 0 && "left padding cannot be negative");
  padding_ = std::to_string(top) + "px " + std::to_string(right) + "px "
           + std::to_string(bottom) + "px " + std::to_string(left) + "px";
  if (created_) PushStyle_("padding", padding_);
}

/**
 * @brief Gets the padding of the element (in pixels).
 * @return Const reference to the padding string.
 */
const std::string& WebTextbox::GetPadding() const noexcept {
  return padding_;
}

// -------------------------------------------------------
// Visibility
// -------------------------------------------------------

/**
 * @brief Sets the visibility of the element.
 * @param visible True to show, false to hide.
 */
void WebTextbox::SetVisible(bool visible) {
  visible_ = visible;
  if (created_) cse498_wtb_set_visible(handle_, visible_ ? 1 : 0);
}

/**
 * @brief Gets the visibility of the element.
 * @return True if visible, false otherwise.
 */
bool WebTextbox::IsVisible() const noexcept { return visible_; }

/**
 * @brief Sets the opacity of the element.
 * @param opacity Opacity value between 0 and 1.
 */
void WebTextbox::SetOpacity(double opacity) {
  // clamp rather than assert - this counts as user input, not a programmer bug
  opacity_ = ClampOpacity_(opacity);
  if (created_) PushStyle_("opacity", std::to_string(opacity_));
}

/**
 * @brief Gets the opacity of the element.
 * @return Opacity value between 0 and 1.
 */
double WebTextbox::GetOpacity() const noexcept { return opacity_; }

// -------------------------------------------------------
// DOM identity
// -------------------------------------------------------

/**
 * @brief Sets the element id.
 * @param id Element id string.
 */
void WebTextbox::SetElementId(const std::string& id) {
  element_id_ = id;
  PushElementId_();
}

/**
 * @brief Gets the element id.
 * @return Const reference to the element id string.
 */
const std::string& WebTextbox::GetElementId() const noexcept {
  return element_id_;
}

/**
 * @brief Sets the parent id.
 * @param parent_id Parent id string.
 */
void WebTextbox::SetParentId(const std::string& parent_id) {
  parent_id_ = parent_id;
  PushParent_();
}

/**
 * @brief Gets the parent id.
 * @return Const reference to the parent id string.
 */
const std::string& WebTextbox::GetParentId() const noexcept {
  return parent_id_;
}

// -------------------------------------------------------
// Lifecycle
// -------------------------------------------------------

/**
 * @brief Ensures the element is created in the DOM.
 */
void WebTextbox::EnsureCreated() {
  if (created_) return;

  handle_ = static_cast<int32_t>(cse498_wtb_create());
  created_ = (handle_ != 0);

  if (!created_) {
    throw std::runtime_error("WebTextbox: failed to create DOM element");
  }

  SyncAll_();
}

/**
 * @brief Removes the element from the DOM.
 */
void WebTextbox::RemoveFromDom() {
  if (!created_) return;
  cse498_wtb_detach(handle_);
}

/**
 * @brief Destroys the element and releases any underlying resources.
 */
void WebTextbox::Destroy() {
  if (!created_) return;
  cse498_wtb_destroy(handle_);
  handle_ = 0;
  created_ = false;
}

/**
 * @brief Returns whether the underlying element has been created.
 * @return True if created, otherwise false.
 */
bool WebTextbox::IsCreated() const noexcept { return created_; }

/**
 * @brief Returns the JS-bridge handle.
 * @return Handle value, or 0 if not created.
 */
int32_t WebTextbox::GetHandle() const noexcept { return handle_; }

void WebTextbox::ApplyFlowLayoutStyles() {
  if (!created_) return;
  PushStyle_("position", "relative");
  PushStyle_("left", "0");
  PushStyle_("top", "0");
  PushStyle_("width", "100%");
  PushStyle_("boxSizing", "border-box");
  PushStyle_("minHeight", kFlowLayoutMinHeightCss);
}

// -------------------------------------------------------
// Private helpers
// -------------------------------------------------------

/**
 * @brief Clamps an opacity value into the valid range.
 * @param v Input opacity value.
 * @return Clamped opacity value.
 */
double WebTextbox::ClampOpacity_(double v) noexcept {
  return ClampOpacityCompileTime(v);
}

/**
 * @brief Pushes a single CSS style property to the DOM.
 * @param prop Property name (camelCase).
 * @param val Property value.
 */
void WebTextbox::PushStyle_(const std::string& prop, const std::string& val) {
  if (!created_) return;
  cse498_wtb_set_style(handle_, prop.c_str(), val.c_str());
}

/**
 * @brief Pushes the text content to the DOM.
 */
void WebTextbox::PushText_() {
  if (!created_) return;
  cse498_wtb_set_text(handle_, text_.c_str());
}

/**
 * @brief Pushes the parent id to the DOM.
 */
void WebTextbox::PushParent_() {
  if (!created_) return;
  cse498_wtb_attach(handle_, parent_id_.c_str());
}

/**
 * @brief Pushes the element id to the DOM.
 */
void WebTextbox::PushElementId_() {
  if (!created_) return;
  cse498_wtb_set_id(handle_, element_id_.c_str());
}

/**
 * @brief Synchronizes all stored state to the DOM.
 */
void WebTextbox::SyncAll_() {
  assert(created_);

  // attach to the right parent first
  PushParent_();
  PushElementId_();

  // text content
  PushText_();

  const auto push_if_not_empty = [this](const char* prop,
                                        const std::string& val) {
    if (!val.empty()) {
      PushStyle_(prop, val);
    }
  };

  std::vector<std::pair<std::string, std::string>> styles;
  styles.reserve(20);
  push_if_not_empty("fontFamily", font_family_);
  styles.emplace_back("fontSize", std::to_string(font_size_px_) + "px");
  styles.emplace_back("fontWeight", font_weight_);
  styles.emplace_back("fontStyle", font_style_);
  styles.emplace_back("lineHeight", std::to_string(line_height_));
  push_if_not_empty("color", text_color_);
  push_if_not_empty("backgroundColor", bg_color_);
  push_if_not_empty("textDecoration", text_decoration_);
  styles.emplace_back("wordWrap", word_wrap_);
  styles.emplace_back("textAlign", text_align_);
  styles.emplace_back("left", std::to_string(left_px_) + "px");
  styles.emplace_back("top", std::to_string(top_px_) + "px");
  if (width_px_ > 0) {
    styles.emplace_back("width", std::to_string(width_px_) + "px");
  }
  if (height_px_ > 0) {
    styles.emplace_back("height", std::to_string(height_px_) + "px");
  }
  push_if_not_empty("padding", padding_);
  styles.emplace_back("opacity", std::to_string(opacity_));

  // Using advance feature: ranges
  std::ranges::for_each(styles, [this](const std::pair<std::string, std::string>& p) {
    PushStyle_(p.first, p.second);
  });

  cse498_wtb_set_visible(handle_, visible_ ? 1 : 0);
}

}  // namespace cse498
