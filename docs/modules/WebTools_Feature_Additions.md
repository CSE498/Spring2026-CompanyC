# Web Tools Feature Additions
Documented By: Prijam Khanal

Developed By: Prijam Khanal

## **0** Introduction
This document summarizes the Group24 web feature additions centered on `WebTextbox`, `WebPopup`, and `AutoTickPolicy`, plus the integration path through `WebInterface` and `WebApp`. The goal of these additions is to improve UI composition, popup behavior, and auto-tick safety in the web runtime.

## **1** WebTextbox Additions

### **1.1** New/Refined Public API
- `SetStyle(const StyleSnapshot&)` provides a group-contract alias for `ApplySnapshot(...)`.
- `AppendLine(T&&)` appends a newline-separated row, handling text and arithmetic values.
- `Clear()`, `Create()`, `Show()`, and `Hide()` provide clearer aliases for existing operations.
- `ApplyFlowLayoutStyles()` switches textbox CSS to flow layout usage (for sidebars/panels).

### **1.2** Behavior Notes
- `AppendLine(...)` inserts `\n` only when existing content is non-empty.
- `ApplyFlowLayoutStyles()` is intended to be called after creation when embedding in non-overlay containers.
- Flow layout styling includes relative positioning, full width, border-box sizing, and a minimum height for multiline text stability.

## **2** WebPopup Additions

### **2.1** Data Model
- `WebPopupOptions` defines popup behavior with:
  - `show_ok_button`
  - `auto_dismiss`
  - `auto_dismiss_ms`
- `WebPopupRequest` wraps popup `message` plus `options`.

### **2.2** Runtime Behavior
- `EnqueueWebPopup(...)` is the public entry point.
- Under Emscripten builds, requests are pushed into an internal FIFO queue and shown one at a time.
- Popup shell is created with a modal overlay and cleaned up on dismiss.
- Dismiss modes supported:
  - OK button click
  - Timed auto-dismiss
  - Combined OK + timed mode
- Escape key and overlay-click dismissal are supported while visible.
- Options are normalized so an invalid "no button and no timer" combination still yields a dismissible popup.
- Timer delay resolution behavior:
  - no timer flag -> no timer
  - timer enabled with no value -> default delay
  - non-positive delay -> timer disabled

### **2.3** Platform Constraint
- On non-Emscripten (native) builds, popup rendering is a no-op by design.

## **3** AutoTickPolicy Additions

### **3.1** Decision Types
- `AutoTickStartBlocker` identifies start blockers (currently popup visibility).
- `AutoTickStartDecision` carries:
  - `should_start`
  - `blocker`
  - `retry_delay_ms`

### **3.2** Decision Helpers
- `DecideAutoTickStart(bool popup_visible, int popup_retry_ms = 100)` blocks auto-tick start while popup overlay is present and returns retry delay.
- `ClampAutoTickIntervalMs(int ms, int min_ms = 50)` enforces a lower bound for safe interval configuration.

## **4** Integration Flow (WebInterface -> WebApp -> Popup Runtime)

### **4.1** Message to Popup Queue
- `WebInterface::Notify(...)` routes:
  - `"popup"` -> immediate popup request with default options
  - `"popup_timed"` -> timed popup request parsed from `ms|text` format
  - `"popup_timed_ok"` -> timed popup with visible OK button
- `WebInterface::TakePendingPopups()` returns and clears queued requests.

### **4.2** WebApp Dispatch
- `WebApp` drains `TakePendingPopups()` and forwards each request through `EnqueueWebPopup(message, options)`.

### **4.3** Auto-Tick Safety Around Popups
- `WebApp` uses `DecideAutoTickStart(...)` before starting looped auto-tick.
- If blocked, `retry_delay_ms` is used to reschedule the start attempt.
- `SetAutoTickMs(...)` applies `ClampAutoTickIntervalMs(...)` before restart.

## **5** Testing and Validation
- `tests/web/WebTextboxTest.cpp` validates textbox text/style APIs and flow-layout related behavior.
- `tests/tools/WebPopupTest.cpp` validates popup option defaults, request modeling, queue semantics via interface paths, timed parsing behavior, and native no-op safety.
- `tests/tools/AutoTickPolicyTest.cpp` validates start-gating decisions and interval clamping edge cases.

## **6** Known Constraints / Notes
- Popup UI construction uses Emscripten/DOM bridge code and therefore applies only to web builds.
- Timed popup parsing in interface flow expects optional `ms|text` format; malformed or edge values are normalized by current logic.
