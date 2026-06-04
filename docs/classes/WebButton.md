[Directory](/DIRECTORY.md)
# WebButton
Documented By: Abigail MacKersie

Developed By: Tess Gonda

## **0** Introduction
This class serves as an interface between the HTML element of a button and the C++ backend for Emscripten development.

File Location: /source/tools/WebButton.hpp

## **1** Structural Elements

### **1.1** Member Variables

#### **1.1.1** Static Member Variables
These member variables outline the default values for the various elements of the HTML button.
- DEFAULT_WIDTH (constexpr int)
- DEFAULT_HEIGHT (constexpr int)
- DEFAULT_LABEL (constexpr const char*)
- DEFAULT_BG_COLOR (constexpr const char*)
- DEFAULT_TEXT_COLOR (constexpr const char*)
- DEFAULT_BORDER_COLOR (constexpr const char*)
- DEFAULT_BORDER_WIDTH (constexpr int)
- DEFAULT_BORDER_RADIUS (constexpr int)

These member variables are static to optimize compile and runtime.
- next_id_ (inline int) - Counter for generating unique IDs
- freeze_count_ (inline size_t) - Nesting depth counter for batched DOM updates; style is only flushed when this reaches zero
- button_registry_ (std::unordered_map<std::string, WebButton*>) - Emscripten only

#### **1.1.2** Private Member Variables
- element_id_ (std::string) - Unique ID
- label_ (std::string) - Actual label text on button
- is_enabled_ (bool) - Does the button accept clicks?
- is_pressed_ (bool) - State bool: clicked or not clicked?
- width_ (int) - Actual width of button
- height_ (int) - Actual height of button
- bg_color_ (std::string) - Actual background color
- text_color_ (std::string) - Actual text color
- border_color_ (std::string) - Actual border color
- border_width_ (int) - Actual border width
- border_radius_ (int) - Actual border radius for rounded borders
- dom_element_ (emscripten::val) - Handle to the underlying HTML <button> DOM element; Emscripten only

## **2** Functions

### **2.1** Private Functions
- void CleanupDOM() - Removes the button from the DOM and erases it from button_registry_; called by the destructor and move assignment operator; Emscripten only
- void CreateDOMElement() - Create the button element in the DOM
- void UpdateDOMStyle() - Apply current button style to the DOM element
- void AttachEventListener() - Attaches an event listener to the button to trigger events on click

### **2.2** Public Functions
Constructors & Destructors
- explicit WebButton(const std::string& label = DEFAULT_LABEL) - Constructor with optional label input
- ~WebButton() - Default destructor
- WebButton Copy Constructor & Assignment DELETED
- WebButton Move Constructor & Assignment ENABLED

DOM Freeze/Unfreeze
- void FreezeDOM() - Suspends DOM style updates; supports nesting
- void UnfreezeDOM() - Resumes DOM style updates; flushes a single UpdateDOMStyle() call when the outermost freeze is released

Getters & Setters for Member Variables
- element_id_ Getter Available (GetElementID)
- label_ Getter & Setter Available (GetLabel, SetLabel)
- is_enabled_ Getter & Setter Available (IsEnabled, SetEnabled)
- is_pressed_ Getter & Setter Available (IsPressed, SetPressed)
- width_ & height_ Composite Getter & Setter Available (GetSize, SetSize)
- width_ Setter Available (SetWidth)
- height_ Setter Available (SetHeight)
- bg_color_ Getter & Setter Available (GetBackgroundColor, SetBackgroundColor)
- text_color_ Getter & Setter Available (GetTextColor, SetTextColor)
- border_color_ Getter & Setter Available (GetBorderColor, SetBorderColor)
- border_width_ & border_radius_ Composite Setter (SetBorder) - uses FreezeDOM/UnfreezeDOM internally
- border_width_ Getter & Setter Available (GetBorderWidth, SetBorderWidth)
- border_radius_ Getter & Setter Available (GetBorderRadius, SetBorderRadius)

Miscellaneous
- void Click() - Trigger button click
- void TogglePressed() - Toggle pressed state
- void AppendTo(const std::string& parent_id) - Append button to target parent element
- void Remove() - Remove element from the DOM
- [[nodiscard]] bool HasCallback() const - Check if button has a callback registered
- void ResetStyle() - Reset button styling to the default values
- [[nodiscard]] std::string ToString() - Create string of button status for debugging
- friend void WebButtonHandleClick(const std::string& button_id) - Friend function for JavaScript callback

### **2.3** Templated Functions
- template<typename Callable> / void SetOnClick(Callable&& callable) - Set the click handler callable, works with all callables compatible with void

## **3** Dependencies
Header File
- Standard Library functional
- Standard Library iostream
- Standard Library string
- Standard Library unordered_map
- Emscripten (bind.h, val.h)

C++ File
- Standard Library cassert
- Standard Library sstream