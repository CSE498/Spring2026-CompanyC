/**
 * @file WebButton.cpp
 * @author Tess Gonda
 */

#include "WebButton.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace cse498 {

// Static member initialization
int WebButton::next_id = 0;

#ifdef __EMSCRIPTEN__
std::map<std::string, WebButton*> WebButton::button_registry;

// C++ function that JavaScript will call
void _WebButton_HandleClick(const std::string& button_id) {
    auto it = WebButton::button_registry.find(button_id);
    if (it != WebButton::button_registry.end()) {
        it->second->Click();
    }
}

// Bind the C++ function so JavaScript can call it
EMSCRIPTEN_BINDINGS(WebButton) {
    emscripten::function("_WebButton_HandleClick", &_WebButton_HandleClick);
}
#endif

//
// Constructors
//

WebButton::WebButton()
    : element_id("btn_" + std::to_string(next_id++))
    , label(DEFAULT_LABEL)
    , is_enabled(true)
    , is_pressed(false)
    , on_click_callback(nullptr)
    , width(DEFAULT_WIDTH)
    , height(DEFAULT_HEIGHT)
    , bg_color(DEFAULT_BG_COLOR)
    , text_color(DEFAULT_TEXT_COLOR)
    , border_color(DEFAULT_BORDER_COLOR)
    , border_width(DEFAULT_BORDER_WIDTH)
    , border_radius(DEFAULT_BORDER_RADIUS)
#ifdef __EMSCRIPTEN__
    , dom_element(emscripten::val::null())
#endif
{
#ifdef __EMSCRIPTEN__
    CreateDOMElement();
#else
    std::cout << "[WebButton] Created: " << element_id << " - '" << label << "'" << std::endl;
#endif
}

WebButton::WebButton(const std::string& label)
    : element_id("btn_" + std::to_string(next_id++))
    , label(label)
    , is_enabled(true)
    , is_pressed(false)
    , on_click_callback(nullptr)
    , width(DEFAULT_WIDTH)
    , height(DEFAULT_HEIGHT)
    , bg_color(DEFAULT_BG_COLOR)
    , text_color(DEFAULT_TEXT_COLOR)
    , border_color(DEFAULT_BORDER_COLOR)
    , border_width(DEFAULT_BORDER_WIDTH)
    , border_radius(DEFAULT_BORDER_RADIUS)
#ifdef __EMSCRIPTEN__
    , dom_element(emscripten::val::null())
#endif
{
#ifdef __EMSCRIPTEN__
    CreateDOMElement();
#else
    std::cout << "[WebButton] Created: " << element_id << " - '" << label << "'" << std::endl;
#endif
}

WebButton::WebButton(const std::string& label, std::function<void()> onClick)
    : element_id("btn_" + std::to_string(next_id++))
    , label(label)
    , is_enabled(true)
    , is_pressed(false)
    , on_click_callback(std::move(onClick))
    , width(DEFAULT_WIDTH)
    , height(DEFAULT_HEIGHT)
    , bg_color(DEFAULT_BG_COLOR)
    , text_color(DEFAULT_TEXT_COLOR)
    , border_color(DEFAULT_BORDER_COLOR)
    , border_width(DEFAULT_BORDER_WIDTH)
    , border_radius(DEFAULT_BORDER_RADIUS)
#ifdef __EMSCRIPTEN__
    , dom_element(emscripten::val::null())
#endif
{
#ifdef __EMSCRIPTEN__
    CreateDOMElement();
    AttachEventListener();
#else
    std::cout << "[WebButton] Created: " << element_id << " - '" << label << "' with callback" << std::endl;
#endif
}

WebButton::~WebButton() {
#ifdef __EMSCRIPTEN__
    button_registry.erase(element_id);
    if (!dom_element.isNull()) {
        dom_element.call<void>("remove");
    }
#else
    std::cout << "[WebButton] Destroyed: " << element_id << std::endl;
#endif
}

// Move constructor
WebButton::WebButton(WebButton&& other) noexcept
    : element_id(std::move(other.element_id))
    , label(std::move(other.label))
    , is_enabled(other.is_enabled)
    , is_pressed(other.is_pressed)
    , on_click_callback(std::move(other.on_click_callback))
    , width(other.width)
    , height(other.height)
    , bg_color(std::move(other.bg_color))
    , text_color(std::move(other.text_color))
    , border_color(std::move(other.border_color))
    , border_width(other.border_width)
    , border_radius(other.border_radius)
#ifdef __EMSCRIPTEN__
    , dom_element(std::move(other.dom_element))
#endif
{
#ifdef __EMSCRIPTEN__
    // Update registry to point to new location
    button_registry[element_id] = this;
    other.dom_element = emscripten::val::null();
#else
    std::cout << "[WebButton] Moved: " << element_id << std::endl;
#endif
}

// Move assignment
WebButton& WebButton::operator=(WebButton&& other) noexcept {
    if (this != &other) {
#ifdef __EMSCRIPTEN__
        // Remove the old ID from the registry before overwriting it
        button_registry.erase(element_id);
#endif
        element_id = std::move(other.element_id);
        label = std::move(other.label);
        is_enabled = other.is_enabled;
        is_pressed = other.is_pressed;
        on_click_callback = std::move(other.on_click_callback);
        width = other.width;
        height = other.height;
        bg_color = std::move(other.bg_color);
        text_color = std::move(other.text_color);
        border_color = std::move(other.border_color);
        border_width = other.border_width;
        border_radius = other.border_radius;

#ifdef __EMSCRIPTEN__
        dom_element = std::move(other.dom_element);
        button_registry[element_id] = this;
        other.dom_element = emscripten::val::null();
#else
        std::cout << "[WebButton] Move-assigned: " << element_id << std::endl;
#endif
    }
    return *this;
}

#ifdef __EMSCRIPTEN__
void WebButton::CreateDOMElement() {
    auto document = emscripten::val::global("document");
    dom_element = document.call<emscripten::val>("createElement", std::string("button"));

    dom_element.set("id", element_id);
    dom_element.set("innerHTML", label);

    button_registry[element_id] = this;

    UpdateDOMStyle();
}

void WebButton::UpdateDOMStyle() {
    if (dom_element.isNull()) return;

    auto style = dom_element["style"];
    style.set("width", std::to_string(width) + "px");
    style.set("height", std::to_string(height) + "px");
    style.set("backgroundColor", bg_color);
    style.set("color", text_color);
    style.set("borderColor", border_color);
    style.set("borderWidth", std::to_string(border_width) + "px");
    style.set("borderRadius", std::to_string(border_radius) + "px");
    style.set("borderStyle", "solid");
    style.set("cursor", "pointer");
    style.set("fontFamily", "Arial, sans-serif");
    style.set("fontSize", "14px");

    if (!is_enabled) {
        style.set("opacity", "0.5");
        style.set("cursor", "not-allowed");
    } else {
        style.set("opacity", "1.0");
    }

    if (is_pressed) {
        style.set("boxShadow", "inset 0 2px 4px rgba(0,0,0,0.3)");
    } else {
        style.set("boxShadow", "");
    }
}

void WebButton::AttachEventListener() {
    if (dom_element.isNull()) return;

    // Use onclick property assignment so repeated calls replace the handler
    // rather than stacking multiple listeners via addEventListener
    EM_ASM({
        var btn = document.getElementById(UTF8ToString($0));
        if (btn) {
            btn.onclick = function() {
                Module._WebButton_HandleClick(UTF8ToString($0));
            };
        }
    }, element_id.c_str());
}
#endif

//
// Label Management
//

void WebButton::SetLabel(const std::string& new_label) {
    label = new_label;
#ifdef __EMSCRIPTEN__
    if (!dom_element.isNull()) {
        dom_element.set("innerHTML", label);
    }
#else
    std::cout << "[WebButton] " << element_id << " label changed to: '" << label << "'" << std::endl;
#endif
}

const std::string& WebButton::GetLabel() const {
    return label;
}

//
// Event Handling
//

void WebButton::SetOnClick(std::function<void()> callback) {
    on_click_callback = std::move(callback);
#ifdef __EMSCRIPTEN__
    AttachEventListener();
#else
    std::cout << "[WebButton] " << element_id << " callback set" << std::endl;
#endif
}

void WebButton::Click() {
    if (!is_enabled) {
#ifndef __EMSCRIPTEN__
        std::cout << "[WebButton] " << element_id << " click ignored (disabled)" << std::endl;
#endif
        return;
    }

#ifndef __EMSCRIPTEN__
    std::cout << "[WebButton] " << element_id << " CLICKED!" << std::endl;
#endif

    if (on_click_callback) {
        try {
            on_click_callback();
        } catch (const std::exception& e) {
            std::cerr << "[WebButton] Exception in callback: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[WebButton] Unknown exception in callback" << std::endl;
        }
    }
}

//
// State Management
//

void WebButton::SetEnabled(bool enabled) {
    is_enabled = enabled;
#ifdef __EMSCRIPTEN__
    if (!dom_element.isNull()) {
        dom_element.set("disabled", !enabled);
        UpdateDOMStyle();
    }
#else
    std::cout << "[WebButton] " << element_id << " "
              << (enabled ? "enabled" : "disabled") << std::endl;
#endif
}

bool WebButton::IsEnabled() const {
    return is_enabled;
}

void WebButton::SetPressed(bool pressed) {
    is_pressed = pressed;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " "
              << (pressed ? "pressed" : "unpressed") << std::endl;
#endif
}

bool WebButton::IsPressed() const {
    return is_pressed;
}

void WebButton::TogglePressed() {
    SetPressed(!is_pressed);
}

//
// Styling (Size)
//

void WebButton::SetSize(int w, int h) {
    if (w <= 0) throw std::invalid_argument("Width must be positive");
    if (h <= 0) throw std::invalid_argument("Height must be positive");

    width = w;
    height = h;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " size set to: "
              << width << "x" << height << std::endl;
#endif
}

void WebButton::SetWidth(int w) {
    if (w <= 0) throw std::invalid_argument("Width must be positive");
    width = w;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " width set to: " << width << std::endl;
#endif
}

void WebButton::SetHeight(int h) {
    if (h <= 0) throw std::invalid_argument("Height must be positive");
    height = h;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " height set to: " << height << std::endl;
#endif
}

std::pair<int, int> WebButton::GetSize() const {
    return {width, height};
}

//
// Styling (Colors)
//

void WebButton::SetBackgroundColor(const std::string& color) {
    bg_color = color;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " background color: " << color << std::endl;
#endif
}

void WebButton::SetTextColor(const std::string& color) {
    text_color = color;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " text color: " << color << std::endl;
#endif
}

void WebButton::SetBorderColor(const std::string& color) {
    border_color = color;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " border color: " << color << std::endl;
#endif
}

const std::string& WebButton::GetBackgroundColor() const {
    return bg_color;
}

//
// Styling (Borders)
//

void WebButton::SetBorderWidth(int w) {
    if (w < 0) throw std::invalid_argument("Border width cannot be negative");
    border_width = w;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " border width: " << border_width << std::endl;
#endif
}

void WebButton::SetBorderRadius(int r) {
    if (r < 0) throw std::invalid_argument("Border radius cannot be negative");
    border_radius = r;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " border radius: " << border_radius << std::endl;
#endif
}

void WebButton::SetBorder(int w, const std::string& color, int r) {
    SetBorderWidth(w);
    SetBorderColor(color);
    SetBorderRadius(r);
}

//
// DOM Interaction
//

void WebButton::AppendTo(const std::string& parent_id) {
    if (parent_id.empty()) {
        throw std::invalid_argument("Parent ID cannot be empty");
    }

#ifdef __EMSCRIPTEN__
    auto document = emscripten::val::global("document");
    auto parent = document.call<emscripten::val>("getElementById", parent_id);

    if (parent.isNull() || parent.isUndefined()) {
        throw std::runtime_error("Parent element not found: " + parent_id);
    }

    parent.call<void>("appendChild", dom_element);
#else
    std::cout << "[WebButton] " << element_id << " appended to: " << parent_id << std::endl;
#endif
}

void WebButton::Remove() {
#ifdef __EMSCRIPTEN__
    if (!dom_element.isNull()) {
        dom_element.call<void>("remove");
    }
#else
    std::cout << "[WebButton] " << element_id << " removed from DOM" << std::endl;
#endif
}

const std::string& WebButton::GetElementID() const {
    return element_id;
}

//
// Utility
//

bool WebButton::HasCallback() const {
    return on_click_callback != nullptr;
}

void WebButton::ResetStyle() {
    width         = DEFAULT_WIDTH;
    height        = DEFAULT_HEIGHT;
    bg_color      = DEFAULT_BG_COLOR;
    text_color    = DEFAULT_TEXT_COLOR;
    border_color  = DEFAULT_BORDER_COLOR;
    border_width  = DEFAULT_BORDER_WIDTH;
    border_radius = DEFAULT_BORDER_RADIUS;
#ifdef __EMSCRIPTEN__
    UpdateDOMStyle();
#else
    std::cout << "[WebButton] " << element_id << " style reset to defaults" << std::endl;
#endif
}

std::string WebButton::ToString() const {
    std::ostringstream oss;
    oss << "WebButton{id='" << element_id
        << "', label='" << label
        << "', enabled=" << (is_enabled ? "true" : "false")
        << ", pressed=" << (is_pressed ? "true" : "false")
        << ", size=" << width << "x" << height
        << ", hasCallback=" << (HasCallback() ? "true" : "false")
        << "}";
    return oss.str();
}

} // namespace cse498
