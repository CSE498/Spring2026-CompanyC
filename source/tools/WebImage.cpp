/**
 * @file WebImage.cpp
 * @author Sadwal Patel
 * @brief Implementation of the WebImage wrapper, including the Emscripten
 *        JavaScript bridge and native no-op stubs used for local testing.
 *
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 *
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this file.
 * The code was then reviewed and heavily edited by the author to ensure
 * correctness and suitability for the project.
 */

#include "WebImage.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
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

namespace {

template <typename ValueT, typename SyncFn, typename EqualFn = std::equal_to<ValueT>>
bool AssignIfChanged(ValueT& slot, ValueT next, SyncFn&& sync_fn,
                     EqualFn equal_fn = EqualFn{}) {
  if (equal_fn(slot, next)) {
    return false;
  }

  slot = std::move(next);
  std::forward<SyncFn>(sync_fn)();
  return true;
}

}  // namespace

bool ContainsString(const std::vector<std::string>& values,
                    const std::string& target) {
  return std::any_of(values.begin(), values.end(),
                     [&target](const std::string& value) {
                       return value == target;
                     });
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
  handle_ = std::exchange(other.handle_, 0);
  created_ = std::exchange(other.created_, false);

  src_ = std::move(other.src_);
  alt_text_ = std::move(other.alt_text_);
  left_px_ = std::exchange(other.left_px_, kDefaultLeftPx);
  top_px_ = std::exchange(other.top_px_, kDefaultTopPx);
  width_px_ = std::exchange(other.width_px_, kAutoSizePx);
  height_px_ = std::exchange(other.height_px_, kAutoSizePx);
  visible_ = std::exchange(other.visible_, true);
  opacity_ = std::exchange(other.opacity_, kDefaultOpacity);
  z_index_ = std::exchange(other.z_index_, kDefaultZIndex);

  parent_id_ = std::move(other.parent_id_);
  element_id_ = std::move(other.element_id_);
  classes_ = std::move(other.classes_);
  styles_ = std::move(other.styles_);
}

double WebImage::Clamp01_(double value) noexcept {
  return std::clamp(value, kMinOpacity, kMaxOpacity);
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
  WithCreatedHandle_([](std::int32_t handle) { WebImageDetachBridge(handle); });
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
  AssignIfChanged(src_, std::move(src), [this]() { SyncSource_(); });
}

const std::string& WebImage::GetSource() const noexcept { return src_; }

void WebImage::SetAltText(std::string alt_text) {
  AssignIfChanged(alt_text_, std::move(alt_text), [this]() { SyncAlt_(); });
}

const std::string& WebImage::GetAltText() const noexcept { return alt_text_; }

void WebImage::SetPositionPx(double left_px, double top_px) {
  if (left_px_ == left_px && top_px_ == top_px) {
    return;
  }

  left_px_ = left_px;
  top_px_ = top_px;
  SyncGeometry_();
}

double WebImage::GetLeftPx() const noexcept { return left_px_; }

double WebImage::GetTopPx() const noexcept { return top_px_; }

void WebImage::SetSizePx(double width_px, double height_px) {
  assert(width_px >= 0.0);
  assert(height_px >= 0.0);

  const double sanitized_width = std::max(kAutoSizePx, width_px);
  const double sanitized_height = std::max(kAutoSizePx, height_px);
  if (width_px_ == sanitized_width && height_px_ == sanitized_height) {
    return;
  }

  width_px_ = sanitized_width;
  height_px_ = sanitized_height;
  SyncGeometry_();
}

double WebImage::GetWidthPx() const noexcept { return width_px_; }

double WebImage::GetHeightPx() const noexcept { return height_px_; }

void WebImage::SetVisible(bool visible) {
  AssignIfChanged(visible_, visible, [this]() { SyncVisibility_(); });
}

bool WebImage::IsVisible() const noexcept { return visible_; }

void WebImage::SetOpacity(double opacity_0_to_1) {
  const double clamped_opacity = Clamp01_(opacity_0_to_1);
  AssignIfChanged(opacity_, clamped_opacity, [this]() { SyncOpacity_(); });
}

double WebImage::GetOpacity() const noexcept { return opacity_; }

void WebImage::SetZIndex(int z_index) {
  AssignIfChanged(z_index_, z_index, [this]() { SyncZIndex_(); });
}

int WebImage::GetZIndex() const noexcept { return z_index_; }

void WebImage::SetParentElementId(std::string parent_id) {
  AssignIfChanged(parent_id_, std::move(parent_id), [this]() { SyncParent_(); });
}

const std::string& WebImage::GetParentElementId() const noexcept {
  return parent_id_;
}

void WebImage::SetElementId(std::string element_id) {
  AssignIfChanged(element_id_, std::move(element_id), [this]() { SyncId_(); });
}

const std::string& WebImage::GetElementId() const noexcept { return element_id_; }

void WebImage::AddCssClass(const std::string& css_class) {
  if (css_class.empty() || ContainsString(classes_, css_class)) {
    return;
  }

  classes_.push_back(css_class);
  WithCreatedHandle_([&css_class](std::int32_t handle) {
    WebImageAddCssClassBridge(handle, css_class.c_str());
  });
}

void WebImage::RemoveCssClass(const std::string& css_class) {
  if (css_class.empty()) {
    return;
  }

  const auto class_it =
      std::find_if(classes_.begin(), classes_.end(),
                   [&css_class](const std::string& existing_class) {
                     return existing_class == css_class;
                   });
  if (class_it == classes_.end()) {
    return;
  }

  WithCreatedHandle_([&css_class](std::int32_t handle) {
    WebImageRemoveCssClassBridge(handle, css_class.c_str());
  });
  classes_.erase(class_it);
}

bool WebImage::HasCssClass(const std::string& css_class) const {
  return ContainsString(classes_, css_class);
}

void WebImage::SetStyle(const std::string& property, const std::string& value) {
  if (property.empty()) {
    return;
  }

  const auto style_it = styles_.find(property);
  if (style_it != styles_.end() && style_it->second == value) {
    return;
  }

  styles_[property] = value;
  WithCreatedHandle_([&property, &value](std::int32_t handle) {
    WebImageSetStyleBridge(handle, property.c_str(), value.c_str());
  });
}

void WebImage::ClearStyle(const std::string& property) {
  if (property.empty()) {
    return;
  }

  const auto erased_count = styles_.erase(property);
  if (erased_count == 0) {
    return;
  }

  WithCreatedHandle_([&property](std::int32_t handle) {
    WebImageClearStyleBridge(handle, property.c_str());
  });
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
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetSourceBridge(handle, src_.c_str());
  });
}

void WebImage::SyncAlt_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetAltBridge(handle, alt_text_.c_str());
  });
}

void WebImage::SyncGeometry_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetPositionBridge(handle, left_px_, top_px_);
    WebImageSetSizeBridge(handle, width_px_, height_px_);
  });
}

void WebImage::SyncVisibility_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetVisibleBridge(handle, visible_ ? 1 : 0);
  });
}

void WebImage::SyncOpacity_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetOpacityBridge(handle, opacity_);
  });
}

void WebImage::SyncZIndex_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetZIndexBridge(handle, z_index_);
  });
}

void WebImage::SyncParent_() {
  const char* parent_id_ptr = parent_id_.empty() ? nullptr : parent_id_.c_str();
  WithCreatedHandle_([parent_id_ptr](std::int32_t handle) {
    WebImageAttachBridge(handle, parent_id_ptr);
  });
}

void WebImage::SyncId_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    WebImageSetIdBridge(handle, element_id_.c_str());
  });
}

void WebImage::SyncClasses_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    std::for_each(classes_.begin(), classes_.end(),
                  [handle](const std::string& css_class) {
                    if (!css_class.empty()) {
                      WebImageAddCssClassBridge(handle, css_class.c_str());
                    }
                  });
  });
}

void WebImage::SyncStyles_() {
  WithCreatedHandle_([this](std::int32_t handle) {
    std::for_each(styles_.begin(), styles_.end(),
                  [handle](const std::pair<const std::string, std::string>& entry) {
                    WebImageSetStyleBridge(handle, entry.first.c_str(),
                                           entry.second.c_str());
                  });
  });
}

}  // namespace cse498
