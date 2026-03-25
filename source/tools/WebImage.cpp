/**
 * @file WebImage.cpp
 * @brief Implementation of the WebImage wrapper, including the Emscripten
 *        JavaScript bridge and native no-op stubs used for local testing.
 */

#include "WebImage.h"

#include <algorithm>
#include <cassert>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

// -----------------------------------------------------------------------------
// Helper bridge functions
//
// These helpers intentionally live in namespace cse498 instead of an anonymous
// namespace so that Group 24 can reuse the same bridge style across web tools.
// -----------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__

EM_JS(int, WebImageCreateBridge, (), {
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

EM_JS(void, WebImageDestroyBridge, (int handle), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  if (img.parentNode) img.remove();
  st.map.delete(handle);
});

EM_JS(void, WebImageDetachBridge, (int handle), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  if (img.parentNode) img.remove();
});

EM_JS(void, WebImageAttachBridge, (int handle, const char* parent_id_ptr), {
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

  if (img.parentNode !== parent) {
    parent.appendChild(img);
  }
});

EM_JS(void, WebImageSetSourceBridge, (int handle, const char* src_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var source = src_ptr ? UTF8ToString(src_ptr) : "";
  if (source.length === 0) img.removeAttribute("src");
  else img.src = source;
});

EM_JS(void, WebImageSetAltBridge, (int handle, const char* alt_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var alt = alt_ptr ? UTF8ToString(alt_ptr) : "";
  img.alt = alt;
});

EM_JS(void, WebImageSetPositionBridge, (int handle, double left_px, double top_px), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  img.style.left = left_px.toFixed(3) + "px";
  img.style.top = top_px.toFixed(3) + "px";
});

EM_JS(void, WebImageSetSizeBridge, (int handle, double width_px, double height_px), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  if (width_px > 0) img.style.width = width_px.toFixed(3) + "px";
  else img.style.width = "";

  if (height_px > 0) img.style.height = height_px.toFixed(3) + "px";
  else img.style.height = "";
});

EM_JS(void, WebImageSetVisibleBridge, (int handle, int visible), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  img.style.display = visible ? "block" : "none";
});

EM_JS(void, WebImageSetOpacityBridge, (int handle, double opacity), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  img.style.opacity = "" + opacity;
});

EM_JS(void, WebImageSetZIndexBridge, (int handle, int z_index), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  img.style.zIndex = "" + z_index;
});

EM_JS(void, WebImageSetIdBridge, (int handle, const char* id_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var id_string = id_ptr ? UTF8ToString(id_ptr) : "";
  if (id_string.length === 0) img.removeAttribute("id");
  else img.id = id_string;
});

EM_JS(void, WebImageAddCssClassBridge, (int handle, const char* css_class_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var css_class = css_class_ptr ? UTF8ToString(css_class_ptr) : "";
  if (css_class.length > 0) img.classList.add(css_class);
});

EM_JS(void, WebImageRemoveCssClassBridge, (int handle, const char* css_class_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var css_class = css_class_ptr ? UTF8ToString(css_class_ptr) : "";
  if (css_class.length > 0) img.classList.remove(css_class);
});

EM_JS(void, WebImageSetStyleBridge, (int handle, const char* property_ptr, const char* value_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var property = property_ptr ? UTF8ToString(property_ptr) : "";
  var value = value_ptr ? UTF8ToString(value_ptr) : "";
  if (property.length === 0) return;

  img.style[property] = value;
});

EM_JS(void, WebImageClearStyleBridge, (int handle, const char* property_ptr), {
  var st = Module.__cse498WebImage;
  if (!st) return;

  var img = st.map.get(handle);
  if (!img) return;

  var property = property_ptr ? UTF8ToString(property_ptr) : "";
  if (property.length === 0) return;

  img.style[property] = "";
});

#else

int NextNativeWebImageHandle() {
  static int next_handle = 1;
  return next_handle++;
}

int WebImageCreateBridge() { return NextNativeWebImageHandle(); }
void WebImageDestroyBridge(int) {}
void WebImageDetachBridge(int) {}
void WebImageAttachBridge(int, const char*) {}
void WebImageSetSourceBridge(int, const char*) {}
void WebImageSetAltBridge(int, const char*) {}
void WebImageSetPositionBridge(int, double, double) {}
void WebImageSetSizeBridge(int, double, double) {}
void WebImageSetVisibleBridge(int, int) {}
void WebImageSetOpacityBridge(int, double) {}
void WebImageSetZIndexBridge(int, int) {}
void WebImageSetIdBridge(int, const char*) {}
void WebImageAddCssClassBridge(int, const char*) {}
void WebImageRemoveCssClassBridge(int, const char*) {}
void WebImageSetStyleBridge(int, const char*, const char*) {}
void WebImageClearStyleBridge(int, const char*) {}

#endif

bool ContainsString(const std::vector<std::string>& values,
                    const std::string& target) {
  return std::find(values.begin(), values.end(), target) != values.end();
}

// -----------------------------------------------------------------------------
// WebImage implementation
// -----------------------------------------------------------------------------

WebImage::WebImage() = default;

WebImage::WebImage(std::string src, std::string alt_text)
    : src_(std::move(src)), alt_text_(std::move(alt_text)) {}

WebImage::~WebImage() { Destroy(); }

WebImage::WebImage(WebImage&& other) noexcept { MoveFrom_(std::move(other)); }

WebImage& WebImage::operator=(WebImage&& other) noexcept {
  if (this == &other) {
    return *this;
  }

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

double WebImage::Clamp01_(double value) noexcept {
#if defined(__cpp_lib_clamp) && (__cpp_lib_clamp >= 201603L)
  return std::clamp(value, 0.0, 1.0);
#else
  return std::max(0.0, std::min(1.0, value));
#endif
}

void WebImage::EnsureCreated() {
  if (created_) {
    return;
  }

  handle_ = static_cast<std::int32_t>(WebImageCreateBridge());
  created_ = (handle_ != 0);

  if (!created_) {
    return;
  }

  SyncAllToDom_();
}

void WebImage::RemoveFromDom() {
  if (!created_) {
    return;
  }

  WebImageDetachBridge(handle_);
}

void WebImage::Destroy() {
  if (!created_) {
    return;
  }

  WebImageDestroyBridge(handle_);
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
  assert(width_px >= 0.0);
  assert(height_px >= 0.0);

  width_px_ = std::max(0.0, width_px);
  height_px_ = std::max(0.0, height_px);
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

const std::string& WebImage::GetParentElementId() const noexcept {
  return parent_id_;
}

void WebImage::SetElementId(std::string element_id) {
  element_id_ = std::move(element_id);
  SyncId_();
}

const std::string& WebImage::GetElementId() const noexcept { return element_id_; }

void WebImage::AddCssClass(const std::string& css_class) {
  if (css_class.empty()) {
    return;
  }

  if (ContainsString(classes_, css_class)) {
    return;
  }

  classes_.push_back(css_class);
  if (created_) {
    WebImageAddCssClassBridge(handle_, css_class.c_str());
  }
}

void WebImage::RemoveCssClass(const std::string& css_class) {
  if (css_class.empty()) {
    return;
  }

  std::vector<std::string>::iterator it =
      std::find(classes_.begin(), classes_.end(), css_class);
  if (it == classes_.end()) {
    return;
  }

  if (created_) {
    WebImageRemoveCssClassBridge(handle_, css_class.c_str());
  }
  classes_.erase(it);
}

bool WebImage::HasCssClass(const std::string& css_class) const {
  return ContainsString(classes_, css_class);
}

void WebImage::SetStyle(const std::string& property, const std::string& value) {
  if (property.empty()) {
    return;
  }

  styles_[property] = value;
  if (created_) {
    WebImageSetStyleBridge(handle_, property.c_str(), value.c_str());
  }
}

void WebImage::ClearStyle(const std::string& property) {
  if (property.empty()) {
    return;
  }

  styles_.erase(property);
  if (created_) {
    WebImageClearStyleBridge(handle_, property.c_str());
  }
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
  if (!created_) {
    return;
  }

  WebImageSetSourceBridge(handle_, src_.c_str());
}

void WebImage::SyncAlt_() {
  if (!created_) {
    return;
  }

  WebImageSetAltBridge(handle_, alt_text_.c_str());
}

void WebImage::SyncGeometry_() {
  if (!created_) {
    return;
  }

  WebImageSetPositionBridge(handle_, left_px_, top_px_);
  WebImageSetSizeBridge(handle_, width_px_, height_px_);
}

void WebImage::SyncVisibility_() {
  if (!created_) {
    return;
  }

  WebImageSetVisibleBridge(handle_, visible_ ? 1 : 0);
}

void WebImage::SyncOpacity_() {
  if (!created_) {
    return;
  }

  WebImageSetOpacityBridge(handle_, opacity_);
}

void WebImage::SyncZIndex_() {
  if (!created_) {
    return;
  }

  WebImageSetZIndexBridge(handle_, z_index_);
}

void WebImage::SyncParent_() {
  if (!created_) {
    return;
  }

  const char* parent_id_ptr = parent_id_.empty() ? nullptr : parent_id_.c_str();
  WebImageAttachBridge(handle_, parent_id_ptr);
}

void WebImage::SyncId_() {
  if (!created_) {
    return;
  }

  WebImageSetIdBridge(handle_, element_id_.c_str());
}

void WebImage::SyncClasses_() {
  if (!created_) {
    return;
  }

  for (const auto& css_class : classes_) {
    if (!css_class.empty()) {
      WebImageAddCssClassBridge(handle_, css_class.c_str());
    }
  }
}

void WebImage::SyncStyles_() {
  if (!created_) {
    return;
  }

  for (const auto& entry : styles_) {
    const std::string& property = entry.first;
    const std::string& value = entry.second;
    WebImageSetStyleBridge(handle_, property.c_str(), value.c_str());
  }
}

}  // namespace cse498
