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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

// -------------------------------------------------------
// JS bridge (Emscripten) / native stubs
// -------------------------------------------------------

#ifdef __EMSCRIPTEN__

// Each EM_JS block defines a C-callable function whose body is JavaScript.
// We keep all the textbox DOM elements in a Module-level map so we can
// look them up by integer handle from C++ without needing string IDs.

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

EM_JS(void, cse498_wtb_destroy, (int handle), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  if (el.parentNode) el.remove();
  st.map.delete(handle);
});

EM_JS(void, cse498_wtb_detach, (int handle), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  if (el.parentNode) el.remove();
});

EM_JS(void, cse498_wtb_set_text, (int handle, const char* txt_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (el) el.textContent = UTF8ToString(txt_ptr);
});

EM_JS(void, cse498_wtb_set_style, (int handle, const char* prop_ptr,
                                    const char* val_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  el.style[UTF8ToString(prop_ptr)] = UTF8ToString(val_ptr);
});

EM_JS(void, cse498_wtb_set_id, (int handle, const char* id_ptr), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (!el) return;
  var v = id_ptr ? UTF8ToString(id_ptr) : "";
  if (v.length === 0) el.removeAttribute('id');
  else el.id = v;
});

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

EM_JS(void, cse498_wtb_set_visible, (int handle, int visible), {
  var st = Module.__cse498WebTextbox;
  if (!st) return;
  var el = st.map.get(handle);
  if (el) el.style.display = visible ? 'block' : 'none';
});

#else  // native stubs for testing without emscripten

namespace {

int g_next_handle = 1;

int cse498_wtb_create() { return g_next_handle++; }
void cse498_wtb_destroy(int) {}
void cse498_wtb_detach(int) {}
void cse498_wtb_set_text(int, const char*) {}
void cse498_wtb_set_style(int, const char*, const char*) {}
void cse498_wtb_set_id(int, const char*) {}
void cse498_wtb_attach(int, const char*) {}
void cse498_wtb_set_visible(int, int) {}

}  // anonymous namespace

#endif

// -------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------

WebTextbox::WebTextbox() = default;

WebTextbox::~WebTextbox() {
  Destroy();
}

// -------------------------------------------------------
// Move operations
// -------------------------------------------------------

WebTextbox::WebTextbox(WebTextbox&& other) noexcept {
  MoveFrom_(std::move(other));
}

WebTextbox& WebTextbox::operator=(WebTextbox&& other) noexcept {
  if (this == &other) return *this;
  Destroy();
  MoveFrom_(std::move(other));
  return *this;
}

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

// -------------------------------------------------------
// Content
// -------------------------------------------------------

void WebTextbox::SetText(const std::string& text) {
  text_ = text;
  PushText_();
}

const std::string& WebTextbox::GetText() const noexcept {
  return text_;
}

void WebTextbox::AppendText(const std::string& text) {
  text_ += text;
  PushText_();
}

void WebTextbox::ClearText() {
  text_.clear();
  PushText_();
}

// -------------------------------------------------------
// Font / Typography
// -------------------------------------------------------

void WebTextbox::SetFontFamily(const std::string& family) {
  font_family_ = family;
  if (created_) PushStyle_("fontFamily", font_family_);
}

const std::string& WebTextbox::GetFontFamily() const noexcept {
  return font_family_;
}

void WebTextbox::SetFontSize(int size_px) {
  assert(size_px > 0 && "font size must be positive");
  font_size_px_ = size_px;
  if (created_) PushStyle_("fontSize", std::to_string(font_size_px_) + "px");
}

int WebTextbox::GetFontSize() const noexcept {
  return font_size_px_;
}

void WebTextbox::SetFontWeight(const std::string& weight) {
  font_weight_ = weight;
  if (created_) PushStyle_("fontWeight", font_weight_);
}

const std::string& WebTextbox::GetFontWeight() const noexcept {
  return font_weight_;
}

void WebTextbox::SetFontStyle(const std::string& style) {
  font_style_ = style;
  if (created_) PushStyle_("fontStyle", font_style_);
}

const std::string& WebTextbox::GetFontStyle() const noexcept {
  return font_style_;
}

void WebTextbox::SetLineHeight(double multiplier) {
  assert(multiplier > 0.0 && "line height must be positive");
  line_height_ = multiplier;
  if (created_) PushStyle_("lineHeight", std::to_string(line_height_));
}

double WebTextbox::GetLineHeight() const noexcept {
  return line_height_;
}

// -------------------------------------------------------
// Colors
// -------------------------------------------------------

void WebTextbox::SetTextColor(const std::string& color) {
  text_color_ = color;
  if (created_) PushStyle_("color", text_color_);
}

const std::string& WebTextbox::GetTextColor() const noexcept {
  return text_color_;
}

void WebTextbox::SetBackgroundColor(const std::string& color) {
  bg_color_ = color;
  if (created_) PushStyle_("backgroundColor", bg_color_);
}

const std::string& WebTextbox::GetBackgroundColor() const noexcept {
  return bg_color_;
}

// -------------------------------------------------------
// Text decoration & wrapping
// -------------------------------------------------------

void WebTextbox::SetTextDecoration(const std::string& decoration) {
  text_decoration_ = decoration;
  if (created_) PushStyle_("textDecoration", text_decoration_);
}

const std::string& WebTextbox::GetTextDecoration() const noexcept {
  return text_decoration_;
}

void WebTextbox::SetWordWrap(const std::string& wrap_mode) {
  word_wrap_ = wrap_mode;
  if (created_) PushStyle_("wordWrap", word_wrap_);
}

const std::string& WebTextbox::GetWordWrap() const noexcept {
  return word_wrap_;
}

// -------------------------------------------------------
// Alignment
// -------------------------------------------------------

void WebTextbox::SetTextAlignment(const std::string& alignment) {
  text_align_ = alignment;
  if (created_) PushStyle_("textAlign", text_align_);
}

const std::string& WebTextbox::GetTextAlignment() const noexcept {
  return text_align_;
}

// -------------------------------------------------------
// Layout
// -------------------------------------------------------

void WebTextbox::SetPosition(double left_px, double top_px) {
  left_px_ = left_px;
  top_px_ = top_px;
  if (created_) {
    PushStyle_("left", std::to_string(left_px_) + "px");
    PushStyle_("top", std::to_string(top_px_) + "px");
  }
}

double WebTextbox::GetLeftPx() const noexcept { return left_px_; }
double WebTextbox::GetTopPx() const noexcept { return top_px_; }

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

double WebTextbox::GetWidthPx() const noexcept { return width_px_; }
double WebTextbox::GetHeightPx() const noexcept { return height_px_; }

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

const std::string& WebTextbox::GetPadding() const noexcept {
  return padding_;
}

// -------------------------------------------------------
// Visibility
// -------------------------------------------------------

void WebTextbox::SetVisible(bool visible) {
  visible_ = visible;
  if (created_) cse498_wtb_set_visible(handle_, visible_ ? 1 : 0);
}

bool WebTextbox::IsVisible() const noexcept { return visible_; }

void WebTextbox::SetOpacity(double opacity) {
  // clamp rather than assert - this counts as user input, not a programmer bug
  opacity_ = ClampOpacity_(opacity);
  if (created_) PushStyle_("opacity", std::to_string(opacity_));
}

double WebTextbox::GetOpacity() const noexcept { return opacity_; }

// -------------------------------------------------------
// DOM identity
// -------------------------------------------------------

void WebTextbox::SetElementId(const std::string& id) {
  element_id_ = id;
  PushElementId_();
}

const std::string& WebTextbox::GetElementId() const noexcept {
  return element_id_;
}

void WebTextbox::SetParentId(const std::string& parent_id) {
  parent_id_ = parent_id;
  PushParent_();
}

const std::string& WebTextbox::GetParentId() const noexcept {
  return parent_id_;
}

// -------------------------------------------------------
// Lifecycle
// -------------------------------------------------------

void WebTextbox::EnsureCreated() {
  if (created_) return;

  handle_ = static_cast<int32_t>(cse498_wtb_create());
  created_ = (handle_ != 0);

  if (!created_) {
    throw std::runtime_error("WebTextbox: failed to create DOM element");
  }

  SyncAll_();
}

void WebTextbox::RemoveFromDom() {
  if (!created_) return;
  cse498_wtb_detach(handle_);
}

void WebTextbox::Destroy() {
  if (!created_) return;
  cse498_wtb_destroy(handle_);
  handle_ = 0;
  created_ = false;
}

bool WebTextbox::IsCreated() const noexcept { return created_; }
int32_t WebTextbox::GetHandle() const noexcept { return handle_; }

// -------------------------------------------------------
// Private helpers
// -------------------------------------------------------

double WebTextbox::ClampOpacity_(double v) noexcept {
  return std::clamp(v, kMinOpacity, kMaxOpacity);
}

void WebTextbox::PushStyle_(const std::string& prop, const std::string& val) {
  if (!created_) return;
  cse498_wtb_set_style(handle_, prop.c_str(), val.c_str());
}

void WebTextbox::PushText_() {
  if (!created_) return;
  cse498_wtb_set_text(handle_, text_.c_str());
}

void WebTextbox::PushParent_() {
  if (!created_) return;
  cse498_wtb_attach(handle_, parent_id_.c_str());
}

void WebTextbox::PushElementId_() {
  if (!created_) return;
  cse498_wtb_set_id(handle_, element_id_.c_str());
}

// pushes everything to the dom - called right after element creation
void WebTextbox::SyncAll_() {
  assert(created_);

  // attach to the right parent first
  PushParent_();
  PushElementId_();

  // text content
  PushText_();

  // font stuff
  if (!font_family_.empty())
    PushStyle_("fontFamily", font_family_);
  PushStyle_("fontSize", std::to_string(font_size_px_) + "px");
  PushStyle_("fontWeight", font_weight_);
  PushStyle_("fontStyle", font_style_);
  PushStyle_("lineHeight", std::to_string(line_height_));

  // colors
  if (!text_color_.empty())
    PushStyle_("color", text_color_);
  if (!bg_color_.empty())
    PushStyle_("backgroundColor", bg_color_);

  // decoration and wrapping
  if (!text_decoration_.empty())
    PushStyle_("textDecoration", text_decoration_);
  PushStyle_("wordWrap", word_wrap_);

  // alignment
  PushStyle_("textAlign", text_align_);

  // position + size
  PushStyle_("left", std::to_string(left_px_) + "px");
  PushStyle_("top", std::to_string(top_px_) + "px");
  if (width_px_ > 0)
    PushStyle_("width", std::to_string(width_px_) + "px");
  if (height_px_ > 0)
    PushStyle_("height", std::to_string(height_px_) + "px");

  // padding
  if (!padding_.empty())
    PushStyle_("padding", padding_);

  // opacity + visibility (do visibility last so it appears fully styled)
  PushStyle_("opacity", std::to_string(opacity_));
  cse498_wtb_set_visible(handle_, visible_ ? 1 : 0);
}

}  // namespace cse498
