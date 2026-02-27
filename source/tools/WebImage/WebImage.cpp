
/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Main WebImage implementation, including Emscripten JS bridge and native stubs.
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 * @author Sadwal Patel
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 **/






#include "WebImage.h"

#include <algorithm>
#include <cassert>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

namespace {

// -----------------------------
// JS bridge (Emscripten only)
// -----------------------------
#ifdef __EMSCRIPTEN__

EM_JS(int, cse498_webimage_create, (), {
  if (!Module.__cse498WebImage) {
    Module.__cse498WebImage = { nextId: 1, map: new Map() };
  }
  var st = Module.__cse498WebImage;

  var id = st.nextId++;
  var img = document.createElement('img');

  img.style.position = 'absolute';
  img.style.left = '0px';
  img.style.top = '0px';
  img.style.display = 'none';
  img.style.opacity = '1';
  img.style.zIndex = '0';
  img.decoding = 'async';

  st.map.set(id, img);
  return id;
});

EM_JS(void, cse498_webimage_destroy, (int handle), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  if (img.parentNode) img.remove();
  st.map.delete(handle);
});

EM_JS(void, cse498_webimage_detach, (int handle), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  if (img.parentNode) img.remove();
});

EM_JS(void, cse498_webimage_attach, (int handle, const char* parent_id_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var parent = null;
  if (parent_id_ptr) {
    var pid = UTF8ToString(parent_id_ptr);
    if (pid.length > 0) parent = document.getElementById(pid);
  }
  if (!parent) parent = document.body;

  if (img.parentNode !== parent) parent.appendChild(img);
});

EM_JS(void, cse498_webimage_set_src, (int handle, const char* src_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var s = src_ptr ? UTF8ToString(src_ptr) : "";
  if (s.length === 0) img.removeAttribute("src");
  else img.src = s;
});

EM_JS(void, cse498_webimage_set_alt, (int handle, const char* alt_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var a = alt_ptr ? UTF8ToString(alt_ptr) : "";
  img.alt = a;
});

EM_JS(void, cse498_webimage_set_pos, (int handle, double left_px, double top_px), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  img.style.left = left_px.toFixed(3) + "px";
  img.style.top  = top_px.toFixed(3) + "px";
});

EM_JS(void, cse498_webimage_set_size, (int handle, double w_px, double h_px), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  if (w_px > 0) img.style.width = w_px.toFixed(3) + "px";
  else img.style.width = "";

  if (h_px > 0) img.style.height = h_px.toFixed(3) + "px";
  else img.style.height = "";
});

EM_JS(void, cse498_webimage_set_visible, (int handle, int visible), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  img.style.display = visible ? "block" : "none";
});

EM_JS(void, cse498_webimage_set_opacity, (int handle, double opacity), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  img.style.opacity = "" + opacity;
});

EM_JS(void, cse498_webimage_set_zindex, (int handle, int z), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  img.style.zIndex = "" + z;
});

EM_JS(void, cse498_webimage_set_id, (int handle, const char* id_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var v = id_ptr ? UTF8ToString(id_ptr) : "";
  if (v.length === 0) img.removeAttribute("id");
  else img.id = v;
});

EM_JS(void, cse498_webimage_class_add, (int handle, const char* c_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  var c = c_ptr ? UTF8ToString(c_ptr) : "";
  if (c.length > 0) img.classList.add(c);
});

EM_JS(void, cse498_webimage_class_remove, (int handle, const char* c_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;
  var c = c_ptr ? UTF8ToString(c_ptr) : "";
  if (c.length > 0) img.classList.remove(c);
});

EM_JS(void, cse498_webimage_style_set, (int handle, const char* prop_ptr, const char* val_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var p = prop_ptr ? UTF8ToString(prop_ptr) : "";
  var v = val_ptr ? UTF8ToString(val_ptr) : "";
  if (p.length === 0) return;
  img.style[p] = v;
});

EM_JS(void, cse498_webimage_style_clear, (int handle, const char* prop_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;
  var img = st.map.get(handle);
  if (!img) return;

  var p = prop_ptr ? UTF8ToString(prop_ptr) : "";
  if (p.length === 0) return;
  img.style[p] = "";
});

#else  // __EMSCRIPTEN__ not defined

// -----------------------------
// Native stubs (test-friendly)
// -----------------------------
int NextNativeHandle() {
  static int next = 1;
  return next++;
}
int cse498_webimage_create() { return NextNativeHandle(); }
void cse498_webimage_destroy(int) {}
void cse498_webimage_detach(int) {}
void cse498_webimage_attach(int, const char*) {}
void cse498_webimage_set_src(int, const char*) {}
void cse498_webimage_set_alt(int, const char*) {}
void cse498_webimage_set_pos(int, double, double) {}
void cse498_webimage_set_size(int, double, double) {}
void cse498_webimage_set_visible(int, int) {}
void cse498_webimage_set_opacity(int, double) {}
void cse498_webimage_set_zindex(int, int) {}
void cse498_webimage_set_id(int, const char*) {}
void cse498_webimage_class_add(int, const char*) {}
void cse498_webimage_class_remove(int, const char*) {}
void cse498_webimage_style_set(int, const char*, const char*) {}
void cse498_webimage_style_clear(int, const char*) {}

#endif

bool Contains_(const std::vector<std::string>& v, const std::string& s) {
  return std::find(v.begin(), v.end(), s) != v.end();
}

}  // namespace

// -----------------------------
// WebImage implementation
// -----------------------------

WebImage::WebImage() = default;

WebImage::WebImage(std::string src, std::string alt_text)
    : src_(std::move(src)), alt_text_(std::move(alt_text)) {}

WebImage::~WebImage() {
  Destroy();
}

WebImage::WebImage(WebImage&& other) noexcept {
  MoveFrom_(std::move(other));
}

WebImage& WebImage::operator=(WebImage&& other) noexcept {
  if (this == &other) return *this;
  Destroy();
  MoveFrom_(std::move(other));
  return *this;
}

void WebImage::MoveFrom_(WebImage&& other) noexcept {
  handle_ = other.handle_;
  created_ = other.created_;

  src_ = std::move(other.src_);
  alt_text_ = std::move(other.alt_text_);
  left_px_ = other.left_px_;
  top_px_ = other.top_px_;
  width_px_ = other.width_px_;
  height_px_ = other.height_px_;
  visible_ = other.visible_;
  opacity_ = other.opacity_;
  z_index_ = other.z_index_;

  parent_id_ = std::move(other.parent_id_);
  element_id_ = std::move(other.element_id_);
  classes_ = std::move(other.classes_);
  styles_ = std::move(other.styles_);

  other.handle_ = 0;
  other.created_ = false;
}

// double WebImage::Clamp01_(double v) noexcept {
//   if (v < 0.0) return 0.0;
//   if (v > 1.0) return 1.0;
//   return v;
// }


double WebImage::Clamp01_(double v) noexcept {
#if defined(__cpp_lib_clamp)
  return std::clamp(v, 0.0, 1.0);
#else
  // Fallback for older toolchains that may not provide std::clamp even in C++17 mode.
  if (v < 0.0) return 0.0;
  if (v > 1.0) return 1.0;
  return v;
#endif
}

void WebImage::EnsureCreated() {
  if (created_) return;

  handle_ = static_cast<std::int32_t>(cse498_webimage_create());
  created_ = (handle_ != 0);

  // If creation failed, keep state but don't crash.
  if (!created_) return;

  SyncAllToDom_();
}

void WebImage::RemoveFromDom() {
  if (!created_) return;
  cse498_webimage_detach(handle_);
}

void WebImage::Destroy() {
  if (!created_) return;
  cse498_webimage_destroy(handle_);
  handle_ = 0;
  created_ = false;
}

bool WebImage::IsCreated() const noexcept { return created_; }
std::int32_t WebImage::GetHandle() const noexcept { return handle_; }

void WebImage::SetSource(std::string src) {
  src_ = std::move(src);
  SyncSource_();
}

const std::string& WebImage::GetSource() const noexcept { return src_; }

void WebImage::SetAltText(std::string alt_text) {
  alt_text_ = std::move(alt_text);
  SyncAlt_();
}

const std::string& WebImage::GetAltText() const noexcept { return alt_text_; }

void WebImage::SetPositionPx(double left_px, double top_px) {
  left_px_ = left_px;
  top_px_ = top_px;
  SyncGeometry_();
}

double WebImage::GetLeftPx() const noexcept { return left_px_; }
double WebImage::GetTopPx() const noexcept { return top_px_; }

void WebImage::SetSizePx(double width_px, double height_px) {
  width_px_ = width_px;
  height_px_ = height_px;
  SyncGeometry_();
}

double WebImage::GetWidthPx() const noexcept { return width_px_; }
double WebImage::GetHeightPx() const noexcept { return height_px_; }

void WebImage::SetVisible(bool visible) {
  visible_ = visible;
  SyncVisibility_();
}

bool WebImage::IsVisible() const noexcept { return visible_; }

void WebImage::SetOpacity(double opacity_0_to_1) {
  // Treat invalid opacity as user input, not a "this should never happen" invariant.
  opacity_ = Clamp01_(opacity_0_to_1);
  SyncOpacity_();
}

double WebImage::GetOpacity() const noexcept { return opacity_; }

void WebImage::SetZIndex(int z_index) {
  z_index_ = z_index;
  SyncZIndex_();
}

int WebImage::GetZIndex() const noexcept { return z_index_; }

void WebImage::SetParentElementId(std::string parent_id) {
  parent_id_ = std::move(parent_id);
  SyncParent_();
}

const std::string& WebImage::GetParentElementId() const noexcept { return parent_id_; }

void WebImage::SetElementId(std::string element_id) {
  element_id_ = std::move(element_id);
  SyncId_();
}

const std::string& WebImage::GetElementId() const noexcept { return element_id_; }

void WebImage::AddCssClass(const std::string& css_class) {
  if (css_class.empty()) return;
  if (Contains_(classes_, css_class)) return;

  classes_.push_back(css_class);
  if (created_) cse498_webimage_class_add(handle_, css_class.c_str());
}

void WebImage::RemoveCssClass(const std::string& css_class) {
  if (css_class.empty()) return;

  auto it = std::find(classes_.begin(), classes_.end(), css_class);
  if (it == classes_.end()) return;

  if (created_) cse498_webimage_class_remove(handle_, css_class.c_str());
  classes_.erase(it);
}

bool WebImage::HasCssClass(const std::string& css_class) const {
  return Contains_(classes_, css_class);
}

void WebImage::SetStyle(const std::string& property, const std::string& value) {
  if (property.empty()) return;
  styles_[property] = value;

  if (created_) cse498_webimage_style_set(handle_, property.c_str(), value.c_str());
}

void WebImage::ClearStyle(const std::string& property) {
  if (property.empty()) return;

  styles_.erase(property);
  if (created_) cse498_webimage_style_clear(handle_, property.c_str());
}

void WebImage::SyncAllToDom_() {
  assert(created_);
  SyncParent_();
  SyncId_();
  SyncSource_();
  SyncAlt_();
  SyncGeometry_();
  SyncOpacity_();
  SyncZIndex_();
  SyncClasses_();
  SyncStyles_();
  SyncVisibility_();
}

void WebImage::SyncSource_() {
  if (!created_) return;
  cse498_webimage_set_src(handle_, src_.c_str());
}

void WebImage::SyncAlt_() {
  if (!created_) return;
  cse498_webimage_set_alt(handle_, alt_text_.c_str());
}

void WebImage::SyncGeometry_() {
  if (!created_) return;
  cse498_webimage_set_pos(handle_, left_px_, top_px_);
  cse498_webimage_set_size(handle_, width_px_, height_px_);
}

void WebImage::SyncVisibility_() {
  if (!created_) return;
  cse498_webimage_set_visible(handle_, visible_ ? 1 : 0);
}

void WebImage::SyncOpacity_() {
  if (!created_) return;
  cse498_webimage_set_opacity(handle_, opacity_);
}

void WebImage::SyncZIndex_() {
  if (!created_) return;
  cse498_webimage_set_zindex(handle_, z_index_);
}

void WebImage::SyncParent_() {
  if (!created_) return;
  const char* pid = parent_id_.empty() ? nullptr : parent_id_.c_str();
  cse498_webimage_attach(handle_, pid);
}

void WebImage::SyncId_() {
  if (!created_) return;
  cse498_webimage_set_id(handle_, element_id_.c_str());
}

void WebImage::SyncClasses_() {
  if (!created_) return;
  // We don’t have a "set all classes" bridge; we apply classes incrementally.
  for (const auto& c : classes_) {
    if (!c.empty()) cse498_webimage_class_add(handle_, c.c_str());
  }
}

void WebImage::SyncStyles_() {
  if (!created_) return;
  for (const auto& kv : styles_) {
    cse498_webimage_style_set(handle_, kv.first.c_str(), kv.second.c_str());
  }
}

}  // namespace cse498
