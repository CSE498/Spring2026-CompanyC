/**
 * @file WebButton.hpp
 * @author Tess Gonda
 * 
 * This class provides a simple interface for creating and managing interactive buttons
 * in a web-based UI.
 */

#ifndef WEBBUTTON_HPP
#define WEBBUTTON_HPP

#include <string>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <map>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

class WebButton {
private:
    std::string element_id;                     ///< Unique identifier for this button
    std::string label;                          ///< Button text content
    bool is_enabled;                            ///< Whether button accepts clicks
    bool is_pressed;                            ///< Toggle state
    std::function<void()> on_click_callback;    ///< Function to call on click
    
    int width;                       ///< Button width (px)
    int height;                      ///< Button height (px)
    std::string bg_color;            ///< Background color
    std::string text_color;          ///< Text color
    std::string border_color;        ///< Border color
    int border_width;                ///< Border width (px)
    int border_radius;               ///< Border radius for rounded corners
    
    static int next_id;              ///< Counter for generating unique IDs
    
#ifdef __EMSCRIPTEN__
    emscripten::val dom_element;
    
    // Global registry to map button IDs to instances (for callbacks)
    static std::map<std::string, WebButton*> button_registry;
    
    void CreateDOMElement();         ///< Create the <button> element
    void UpdateDOMStyle();           ///< Apply current style to DOM
    void AttachEventListener();      ///< Wire up click handler
#endif
    
public:
    /**
     * @brief Default constructor
     */
    WebButton();
    
    /**
     * @brief Constructor with custom label
     * @param label The text to display on the button
     */
    WebButton(const std::string& label);
    
    /**
     * @brief Constructor with label and click callback
     * @param label The text to display on the button
     * @param onClick Function to call when button is clicked
     */
    WebButton(const std::string& label, std::function<void()> onClick);
    
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
     * @brief Set the button's display text
     * @param label New text for the button
     */
    void SetLabel(const std::string& label);
    
    /**
     * @brief Get the current button label
     * @return Current button text
     */
    std::string GetLabel() const;

    /**
     * @brief Set the function to call when button is clicked
     * @param callback Function with signature void()
     */
    void SetOnClick(std::function<void()> callback);
    
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
    bool IsEnabled() const;
    
    /**
     * @brief Set the pressed state (for toggle buttons)
     * @param pressed True for pressed/active, false for normal
     */
    void SetPressed(bool pressed);
    
    /**
     * @brief Check if button is in pressed state
     * @return True if pressed, false otherwise
     */
    bool IsPressed() const;
    
    /**
     * @brief Toggle the pressed state
     */
    void TogglePressed();
    
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
    std::pair<int, int> GetSize() const;
    
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
    std::string GetBackgroundColor() const;
    
    /**
     * @brief Set border width
     * @param width Border width in pixels (must be non-negative)
     */
    void SetBorderWidth(int width);
    
    /**
     * @brief Set border radius for rounded corners
     * @param radius Corner radius in pixels (must be non-negative)
     */
    void SetBorderRadius(int radius);
    
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
    std::string GetElementID() const;
    
    /**
     * @brief Check if button has a click callback registered
     * @return True if callback exists, false otherwise
     */
    bool HasCallback() const;
    
    /**
     * @brief Reset button to default styling
     */
    void ResetStyle();
    
    /**
     * @brief Get string representation of button for debugging
     * @return String describing button state
     */
    std::string ToString() const;
    
#ifdef __EMSCRIPTEN__
    // Friend function for JavaScript callback
    friend void _WebButton_HandleClick(const std::string& button_id);
#endif
};

#endif // WEBBUTTON_HPP