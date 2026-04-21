/**
 * @file WebButton.cpp
 * @author Tess Gonda
 */

#include "WebButton.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

namespace cse498 {

#ifdef __EMSCRIPTEN__
// C++ function that JavaScript will call
void WebButtonHandleClick(const std::string& button_id) {
  auto it = WebButton::button_registry_.find(button_id);
  if (it != WebButton::button_registry_.end()) {
    it->second->Click();
  }
}

// Bind the C++ function so JavaScript can call it
EMSCRIPTEN_BINDINGS(WebButton) {
  emscripten::function("WebButtonHandleClick", &WebButtonHandleClick);
}
#endif

//
// Constructors
//

WebButton::WebButton(const std::string& label)
    : element_id_("btn_" + std::to_string(next_id_++))
    , label_(label)
#ifdef __EMSCRIPTEN__
    , dom_element_(emscripten::val::null())
#endif
{
#ifdef __EMSCRIPTEN__
  CreateDOMElement();
#else
  std::cout << "[WebButton] Initialized (not yet in DOM): " << element_id_ << " - '" << label_ << "'" << std::endl;
#endif
}

WebButton::~WebButton() {
#ifdef __EMSCRIPTEN__
  CleanupDOM();
#else
  std::cout << "[WebButton] Destroyed: " << element_id_ << std::endl;
#endif
}

// Move constructor
WebButton::WebButton(WebButton&& other) noexcept
    : element_id_(std::move(other.element_id_))
    , label_(std::move(other.label_))
    , is_enabled_(other.is_enabled_)
    , is_pressed_(other.is_pressed_)
    , on_click_callback_(std::move(other.on_click_callback_))
    , width_(other.width_)
    , height_(other.height_)
    , bg_color_(std::move(other.bg_color_))
    , text_color_(std::move(other.text_color_))
    , border_color_(std::move(other.border_color_))
    , border_width_(other.border_width_)
    , border_radius_(other.border_radius_)
#ifdef __EMSCRIPTEN__
    , dom_element_(std::move(other.dom_element_))
#endif
{
#ifdef __EMSCRIPTEN__
  // Update registry to point to new location
  button_registry_[element_id_] = this;
  other.dom_element_ = emscripten::val::null();
#else
  std::cout << "[WebButton] Moved: " << element_id_ << std::endl;
#endif
  other.element_id_.clear();
}

// Move assignment
WebButton& WebButton::operator=(WebButton&& other) noexcept {
  if (this != &other) {
#ifdef __EMSCRIPTEN__
    // Clean up current DOM element and registry entry before overwriting
    CleanupDOM();
#endif
    element_id_        = std::move(other.element_id_);
    label_             = std::move(other.label_);
    is_enabled_        = other.is_enabled_;
    is_pressed_        = other.is_pressed_;
    on_click_callback_ = std::move(other.on_click_callback_);
    width_             = other.width_;
    height_            = other.height_;
    bg_color_          = std::move(other.bg_color_);
    text_color_        = std::move(other.text_color_);
    border_color_      = std::move(other.border_color_);
    border_width_      = other.border_width_;
    border_radius_     = other.border_radius_;

#ifdef __EMSCRIPTEN__
    dom_element_ = std::move(other.dom_element_);
    button_registry_[element_id_] = this;
    other.dom_element_ = emscripten::val::null();
#else
    std::cout << "[WebButton] Move-assigned: " << element_id_ << std::endl;
#endif
    other.element_id_.clear();
  }
  return *this;
}

#ifdef __EMSCRIPTEN__
void WebButton::CleanupDOM() {
  if (!element_id_.empty()) {
    button_registry_.erase(element_id_);
  }
  if (!dom_element_.isNull()) {
    dom_element_.call<void>("remove");
  }
}

void WebButton::CreateDOMElement() {
  auto document = emscripten::val::global("document");
  dom_element_ = document.call<emscripten::val>("createElement", std::string("button"));

  dom_element_.set("id", element_id_);
  dom_element_.set("textContent", label_);

  button_registry_[element_id_] = this;

  UpdateDOMStyle();
}

void WebButton::UpdateDOMStyle() {
  if (dom_element_.isNull()) return;

  auto style = dom_element_["style"];
  style.set("width", std::to_string(width_) + "px");
  style.set("height", std::to_string(height_) + "px");
  style.set("backgroundColor", bg_color_);
  style.set("color", text_color_);
  style.set("borderColor", border_color_);
  style.set("borderWidth", std::to_string(border_width_) + "px");
  style.set("borderRadius", std::to_string(border_radius_) + "px");
  style.set("borderStyle", "solid");
  style.set("cursor", "pointer");
  style.set("fontFamily", "Arial, sans-serif");
  style.set("fontSize", "14px");

  if (!is_enabled_) {
    style.set("opacity", "0.5");
    style.set("cursor", "not-allowed");
  } else {
    style.set("opacity", "1.0");
  }

  if (is_pressed_) {
    style.set("boxShadow", "inset 0 2px 4px rgba(0,0,0,0.3)");
  } else {
    style.set("boxShadow", "");
  }
}

void WebButton::AttachEventListener() {
  if (dom_element_.isNull()) return;

  // Use onclick property assignment so repeated calls replace the handler
  // instead of adding multiple listeners.
  // btn_id is captured by value in the closure so it remains valid when the
  // click fires later (avoids relying on the pointer $0 being stable).
  EM_ASM({
    var btn = document.getElementById(UTF8ToString($0));
    if (btn) {
      var btn_id = UTF8ToString($0);
      btn.onclick = function() {
        Module.WebButtonHandleClick(btn_id);
      };
    }
  }, element_id_.c_str());
}
#endif

//
// Label Management
//

void WebButton::SetLabel(const std::string& new_label) {
  label_ = new_label;
#ifdef __EMSCRIPTEN__
  if (!dom_element_.isNull()) {
    dom_element_.set("textContent", label_);
  }
#else
  std::cout << "[WebButton] " << element_id_ << " label changed to: '" << label_ << "'" << std::endl;
#endif
}

//
// Event Handling
//

void WebButton::Click() {
  if (!is_enabled_) {
#ifndef __EMSCRIPTEN__
    std::cout << "[WebButton] " << element_id_ << " click ignored (disabled)" << std::endl;
#endif
    return;
  }

#ifndef __EMSCRIPTEN__
  std::cout << "[WebButton] " << element_id_ << " CLICKED!" << std::endl;
#endif

  if (on_click_callback_) {
    on_click_callback_();
  }
}

//
// State Management
//

void WebButton::SetEnabled(bool enabled) {
  is_enabled_ = enabled;
#ifdef __EMSCRIPTEN__
  if (!dom_element_.isNull()) {
    dom_element_.set("disabled", !enabled);
    if (freeze_count_ == 0) UpdateDOMStyle();
  }
#else
  std::cout << "[WebButton] " << element_id_ << " "
            << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

void WebButton::SetPressed(bool pressed) {
  is_pressed_ = pressed;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " "
            << (pressed ? "pressed" : "unpressed") << std::endl;
#endif
}

//
// Styling (Size)
//

void WebButton::SetSize(int w, int h) {
  assert(w > 0 && "Width must be positive");
  assert(h > 0 && "Height must be positive");

  width_  = w;
  height_ = h;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " size set to: "
            << width_ << "x" << height_ << std::endl;
#endif
}

void WebButton::SetWidth(int w) {
  assert(w > 0 && "Width must be positive");
  width_ = w;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " width set to: " << width_ << std::endl;
#endif
}

void WebButton::SetHeight(int h) {
  assert(h > 0 && "Height must be positive");
  height_ = h;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " height set to: " << height_ << std::endl;
#endif
}

//
// Styling (Colors)
//

void WebButton::SetBackgroundColor(const std::string& color) {
  bg_color_ = color;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " background color: " << color << std::endl;
#endif
}

void WebButton::SetTextColor(const std::string& color) {
  text_color_ = color;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " text color: " << color << std::endl;
#endif
}

void WebButton::SetBorderColor(const std::string& color) {
  border_color_ = color;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " border color: " << color << std::endl;
#endif
}

//
// Styling (Borders)
//

void WebButton::SetBorderWidth(int w) {
  assert(w >= 0 && "Border width cannot be negative");
  border_width_ = w;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " border width: " << border_width_ << std::endl;
#endif
}

void WebButton::SetBorderRadius(int r) {
  assert(r >= 0 && "Border radius cannot be negative");
  border_radius_ = r;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " border radius: " << border_radius_ << std::endl;
#endif
}

void WebButton::SetBorder(int w, const std::string& color, int r) {
  FreezeDOM();
  SetBorderWidth(w);
  SetBorderColor(color);
  SetBorderRadius(r);
  UnfreezeDOM();
}

//
// DOM Interaction
//

void WebButton::AppendTo(const std::string& parent_id) {
  assert(!parent_id.empty() && "Parent ID cannot be empty");

#ifdef __EMSCRIPTEN__
  auto document = emscripten::val::global("document");
  auto parent = document.call<emscripten::val>("getElementById", parent_id);

  assert(!parent.isNull() && !parent.isUndefined() && "Parent element not found");

  parent.call<void>("appendChild", dom_element_);
#else
  std::cout << "[WebButton] " << element_id_ << " appended to: " << parent_id << std::endl;
#endif
}

void WebButton::Remove() {
#ifdef __EMSCRIPTEN__
  if (!dom_element_.isNull()) {
    dom_element_.call<void>("remove");
  }
#else
  std::cout << "[WebButton] " << element_id_ << " removed from DOM" << std::endl;
#endif
}

//
// Utility
//

void WebButton::ResetStyle() {
  width_         = DEFAULT_WIDTH;
  height_        = DEFAULT_HEIGHT;
  bg_color_      = DEFAULT_BG_COLOR;
  text_color_    = DEFAULT_TEXT_COLOR;
  border_color_  = DEFAULT_BORDER_COLOR;
  border_width_  = DEFAULT_BORDER_WIDTH;
  border_radius_ = DEFAULT_BORDER_RADIUS;
#ifdef __EMSCRIPTEN__
  if (freeze_count_ == 0) UpdateDOMStyle();
#else
  std::cout << "[WebButton] " << element_id_ << " style reset to defaults" << std::endl;
#endif
}

std::string WebButton::ToString() const {
  std::ostringstream oss;
  oss << "WebButton{id='" << element_id_
      << "', label='" << label_
      << "', enabled=" << (is_enabled_ ? "true" : "false")
      << ", pressed=" << (is_pressed_ ? "true" : "false")
      << ", size=" << width_ << "x" << height_
      << ", hasCallback=" << (HasCallback() ? "true" : "false")
      << "}";
  return oss.str();
}

} // namespace cse498
