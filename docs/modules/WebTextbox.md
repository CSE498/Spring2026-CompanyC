# WebTextbox
Documented By: Prijam Khanal

Developed By: Prijam Khanal

## **0** Introduction
`WebTextbox` is a web-UI utility class that manages a text DOM element (`div`) from C++ code. It stores style/content state in C++, and synchronizes that state to the browser when running under Emscripten. On native builds, it uses stubs so behavior can still be unit tested without a browser.

The class is intended for HUD/status text, panel text, and modal/popup message content where code wants direct C++ control over text, typography, layout, and visibility.

## **1** Structural Elements

### **1.1** Member Variables

#### **1.1.1** Static Member Variables
- `kDefaultFontSize` (`int`) - default text size.
- `kDefaultLineHeight` (`double`) - default line-height multiplier.
- `kDefaultOpacity` (`double`) - default element opacity.
- `kMinOpacity` (`double`) - minimum allowed opacity.
- `kMaxOpacity` (`double`) - maximum allowed opacity.

#### **1.1.2** Private Member Variables
- `handle_` (`int32_t`) - underlying runtime handle/identifier.
- `created_` (`bool`) - whether the DOM element is currently created.
- `text_` (`std::string`) - current text content.
- Style state: `font_family_`, `font_size_px_`, `font_weight_`, `font_style_`, `line_height_`.
- Visual state: `text_color_`, `bg_color_`, `text_decoration_`, `word_wrap_`, `text_align_`, `padding_`.
- Layout state: `left_px_`, `top_px_`, `width_px_`, `height_px_`.
- Visibility state: `visible_`, `opacity_`.
- DOM identity: `element_id_`, `parent_id_`.

### **1.2** Structs

#### **1.2.1** StyleSnapshot
`StyleSnapshot` is a value-type copy of all user-facing textbox state (content + styling + identity), without the live DOM handle.

**1.2.1.1** Member Variables
- Includes text, typography, color, decoration/wrap/alignment, padding, position/size, visibility/opacity, and DOM IDs.

**1.2.1.2** Functions
- Captured with `CaptureStyle()`.
- Applied with `ApplySnapshot(...)` or alias `SetStyle(...)`.

## **2** Functions

### **2.1** Static Functions
- `ClampOpacityCompileTime(double)` - compile-time-friendly clamp helper for opacity bounds.

### **2.2** Private Functions
- `SyncAll_()` - pushes all stored state to DOM in one pass.
- `PushStyle_(prop, value)` - updates a single CSS property.
- `PushText_()` - updates textbox content.
- `PushParent_()` - attaches/re-attaches to parent container.
- `PushElementId_()` - updates element ID in DOM.
- `MoveFrom_(WebTextbox&&)` - move-state helper.
- `ClampOpacity_(double)` - runtime opacity clamp.

### **2.3** Public Functions (Grouped)

#### **2.3.1** Content and Snapshot
- `SetText(...)`, `GetText()`, `AppendText(...)`, `AppendLine(...)`, `ClearText()`, `Clear()`.
- `CaptureStyle()`, `ApplySnapshot(...)`, `SetStyle(...)`.

#### **2.3.2** Typography and Color
- Font: `SetFontFamily`, `SetFontSize`, `SetFontWeight`, `SetFontStyle`, `SetLineHeight` (and getters).
- Color: `SetTextColor`, `SetBackgroundColor` (and getters).

#### **2.3.3** Text Formatting and Layout
- Formatting: `SetTextDecoration`, `SetWordWrap`, `SetTextAlignment` (and getters).
- Layout: `SetPosition`, `SetSize`, `SetPadding` (and getters).
- Flow layout helper: `ApplyFlowLayoutStyles()` for panel/sidebar contexts.

#### **2.3.4** Visibility and Lifecycle
- Visibility: `SetVisible`, `IsVisible`, `SetOpacity`, `GetOpacity`, `Show()`, `Hide()`.
- Identity: `SetElementId`, `SetParentId` (and getters).
- Lifecycle: `EnsureCreated`, `Create()`, `RemoveFromDom`, `Destroy`, `IsCreated`, `GetHandle`.

## **3** Behavior Notes
- State-first design: setters update C++ state and push to DOM when created.
- Lifecycle-safe usage:
  - You can configure styles/text before creation.
  - `EnsureCreated()` / `Create()` creates and syncs current state.
  - `Destroy()` removes and resets created state safely.
- `AppendLine(...)` appends newline-separated rows and supports both string-like and arithmetic values.
- `ApplyFlowLayoutStyles()` is for non-overlay containers; it switches key CSS properties so text works in stacked/flow layouts.
- Copy operations are deleted to avoid two objects managing one DOM node; move operations are supported.

## **4** Test Documentation

### **4.1** Primary Test File
- `tests/web/WebTextboxTest.cpp`

### **4.2** Coverage Areas
- Default construction state and constants.
- Content operations (`SetText`, `AppendText`, `AppendLine`, `ClearText`).
- Typography, colors, decorations, wrapping, alignment, layout, padding.
- Visibility and opacity clamp behavior.
- DOM identity and lifecycle (`EnsureCreated`, `RemoveFromDom`, `Destroy`, double-destroy safety).
- Move semantics and self-move safety.
- Snapshot capture/apply value semantics.
- Group-contract alias API (`Create`, `Clear`, `Show`, `Hide`, `SetStyle`).
- `ApplyFlowLayoutStyles()` call-path and state stability.

### **4.3** Running Tests
- Intended command from repository root:
  - `bash tests/run_webtextbox_tests.sh`
- Manual command equivalent:
  - `g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -O0 -g -I./source ./tests/web/WebTextboxTest.cpp ./source/tools/WebTextbox.cpp -o webtextbox_tests && ./webtextbox_tests`

## **5** Dependencies
- C++ standard library: strings, type traits, utility helpers.
- Emscripten path for browser DOM integration.
- Native stubs for non-web unit-testing.

## **6** Known Issues / Notes
- Test script path currently references `tests/tools/WebTextboxTest.cpp`, while test source header and current location use `tests/web/WebTextboxTest.cpp`. Keep this aligned when running tests.
