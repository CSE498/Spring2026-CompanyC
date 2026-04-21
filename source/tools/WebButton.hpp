#pragma once

/**
 * @file WebButton.hpp
 * @author Tess Gonda
 *
 * WebButton wraps an HTML <button> element (under Emscripten) or produces
 * console output stubs (native builds) so the same C++ code works in both
 * environments.
 *
 * Basic usage:
 * @code
 *   cse498::WebButton btn("Click Me", []() {
 *       std::cout << "Button clicked!\n";
 *   });
 *   btn.SetSize(120, 40);
 *   btn.SetBackgroundColor("#4f46e5");
 *   btn.AppendTo("my-container");   // attaches to <div id="my-container">
 * @endcode
 */

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#endif

namespace cse498 {

class WebButton {
private:
  // Default style values — defined first so member initializers below can use them
  static constexpr int         DEFAULT_WIDTH         = 100;
  static constexpr int         DEFAULT_HEIGHT        = 30;
  static constexpr const char* DEFAULT_LABEL         = "Button";
  static constexpr const char* DEFAULT_BG_COLOR      = "#f0f0f0";
  static constexpr const char* DEFAULT_TEXT_COLOR    = "#000000";
  static constexpr const char* DEFAULT_BORDER_COLOR  = "#cccccc";
  static constexpr int         DEFAULT_BORDER_WIDTH  = 1;
  static constexpr int         DEFAULT_BORDER_RADIUS = 4;

  std::string element_id_;                                     ///< Unique identifier for this button
  std::string label_            = DEFAULT_LABEL;               ///< Button text content
  bool        is_enabled_       = true;                        ///< Whether button accepts clicks
  bool        is_pressed_       = false;                       ///< Toggle state
  std::function<void()> on_click_callback_;                    ///< Function to call on click

  int         width_         = DEFAULT_WIDTH;                  ///< Button width (px)
  int         height_        = DEFAULT_HEIGHT;                 ///< Button height (px)
  std::string bg_color_      = DEFAULT_BG_COLOR;               ///< Background color
  std::string text_color_    = DEFAULT_TEXT_COLOR;             ///< Text color
  std::string border_color_  = DEFAULT_BORDER_COLOR;           ///< Border color
  int         border_width_  = DEFAULT_BORDER_WIDTH;           ///< Border width (px)
  int         border_radius_ = DEFAULT_BORDER_RADIUS;          ///< Border radius for rounded corners

  static inline int    next_id_      = 0;                      ///< Counter for generating unique IDs
  static inline size_t freeze_count_ = 0;                      ///< Freeze nesting depth for batched DOM updates

#ifdef __EMSCRIPTEN__
  emscripten::val dom_element_;

  // Global registry mapping button IDs to instances (for JS callbacks)
  static inline std::unordered_map<std::string, WebButton*> button_registry_;

  void CleanupDOM();           ///< Remove from registry and DOM
  void CreateDOMElement();     ///< Create the <button> element
  void UpdateDOMStyle();       ///< Apply current style to DOM
  void AttachEventListener();  ///< Wire up click handler
#endif

public:
  /**
   * @brief Constructor with optional custom label
   * @param label The text to display on the button (defaults to "Button")
   */
  explicit WebButton(const std::string& label = DEFAULT_LABEL);

  /**
   * @brief Constructor with label and any callable click handler
   * @tparam Callable Any callable type compatible with void() - lambda, function pointer, or functor
   * @param label The text to display on the button
   * @param onClick Callable to invoke when button is clicked
   */
  template<typename Callable>
  WebButton(const std::string& label, Callable&& onClick)
      : WebButton(label)
  {
    SetOnClick(std::forward<Callable>(onClick));
  }

  /**
   * @brief Destructor
   */
  ~WebButton();

  // Delete copy constructor and copy assignment to prevent copying
  WebButton(const WebButton&) = delete;
  WebButton& operator=(const WebButton&) = delete;

  // Enable move constructor and move assignment
  WebButton(WebButton&& other) noexcept;
  WebButton& operator=(WebButton&& other) noexcept;

  /**
   * @brief Suspend DOM updates until UnfreezeDOM() is called; supports nesting
   */
  void FreezeDOM() { ++freeze_count_; }

  /**
   * @brief Resume DOM updates; flushes a pending style update on the outermost unfreeze
   */
  void UnfreezeDOM() {
    if (freeze_count_ > 0 && --freeze_count_ == 0) {
#ifdef __EMSCRIPTEN__
      UpdateDOMStyle();
#endif
    }
  }

  /**
   * @brief Set the button's display text
   * @param label New text for the button
   */
  void SetLabel(const std::string& label);

  /**
   * @brief Get the current button label
   * @return Current button text
   */
  [[nodiscard]] const std::string& GetLabel() const { return label_; }

  /**
   * @brief Set the click handler, accepts any callable compatible with void()
   * @tparam Callable Lambda, function pointer, or functor with signature void()
   * @param callable The callable to invoke on click
   */
  template<typename Callable>
  void SetOnClick(Callable&& callable) {
    on_click_callback_ = std::forward<Callable>(callable);
#ifdef __EMSCRIPTEN__
    AttachEventListener();
#else
    std::cout << "[WebButton] " << element_id_ << " callback set" << std::endl;
#endif
  }

  /**
   * @brief Trigger a button click programmatically
   */
  void Click();

  /**
   * @brief Enable or disable the button
   * @param enabled True to enable, false to disable
   */
  void SetEnabled(bool enabled);

  /**
   * @brief Check if button is currently enabled
   * @return True if enabled, false if disabled
   */
  [[nodiscard]] bool IsEnabled() const { return is_enabled_; }

  /**
   * @brief Set the pressed state (for toggle buttons)
   * @param pressed True for pressed/active, false for normal
   */
  void SetPressed(bool pressed);

  /**
   * @brief Check if button is in pressed state
   * @return True if pressed, false otherwise
   */
  [[nodiscard]] bool IsPressed() const { return is_pressed_; }

  /**
   * @brief Toggle the pressed state
   */
  void TogglePressed() { SetPressed(!is_pressed_); }

  /**
   * @brief Set button dimensions
   * @param width Width in pixels (must be positive)
   * @param height Height in pixels (must be positive)
   */
  void SetSize(int width, int height);

  /**
   * @brief Set button width
   * @param width Width in pixels (must be positive)
   */
  void SetWidth(int width);

  /**
   * @brief Set button height
   * @param height Height in pixels (must be positive)
   */
  void SetHeight(int height);

  /**
   * @brief Get current button dimensions
   * @return Pair of (width, height) in pixels
   */
  [[nodiscard]] std::pair<int, int> GetSize() const { return {width_, height_}; }

  /**
   * @brief Set background color
   * @param color CSS color string
   */
  void SetBackgroundColor(const std::string& color);

  /**
   * @brief Set text color
   * @param color CSS color string
   */
  void SetTextColor(const std::string& color);

  /**
   * @brief Set border color
   * @param color CSS color string
   */
  void SetBorderColor(const std::string& color);

  /**
   * @brief Get current background color
   * @return CSS color string
   */
  [[nodiscard]] const std::string& GetBackgroundColor() const { return bg_color_; }

  /**
   * @brief Get current text color
   * @return CSS color string
   */
  [[nodiscard]] const std::string& GetTextColor() const { return text_color_; }

  /**
   * @brief Get current border color
   * @return CSS color string
   */
  [[nodiscard]] const std::string& GetBorderColor() const { return border_color_; }

  /**
   * @brief Set border width
   * @param width Border width in pixels (must be non-negative)
   */
  void SetBorderWidth(int width);

  /**
   * @brief Get current border width
   * @return Border width in pixels
   */
  [[nodiscard]] int GetBorderWidth() const { return border_width_; }

  /**
   * @brief Set border radius for rounded corners
   * @param radius Corner radius in pixels (must be non-negative)
   */
  void SetBorderRadius(int radius);

  /**
   * @brief Get current border radius
   * @return Corner radius in pixels
   */
  [[nodiscard]] int GetBorderRadius() const { return border_radius_; }

  /**
   * @brief Set all border properties at once
   * @param width Border width (px)
   * @param color Border color (CSS string)
   * @param radius Corner radius (px)
   */
  void SetBorder(int width, const std::string& color, int radius = 0);

  /**
   * @brief Append button to a parent element
   * @param parent_id ID of the parent HTML element
   */
  void AppendTo(const std::string& parent_id);

  /**
   * @brief Remove button from the DOM
   */
  void Remove();

  /**
   * @brief Get the unique element ID
   * @return Button's DOM element ID
   */
  [[nodiscard]] const std::string& GetElementID() const { return element_id_; }

  /**
   * @brief Check if button has a click callback registered
   * @return True if callback exists, false otherwise
   */
  [[nodiscard]] bool HasCallback() const { return on_click_callback_ != nullptr; }

  /**
   * @brief Reset button to default styling
   */
  void ResetStyle();

  /**
   * @brief Get string representation of button for debugging
   * @return String describing button state
   */
  [[nodiscard]] std::string ToString() const;

#ifdef __EMSCRIPTEN__
  // Friend function for JavaScript callback
  friend void WebButtonHandleClick(const std::string& button_id);
#endif
};

} // namespace cse498
