# Group 24 — Web Interface module (WebTextbox) spec

## Role

The interface module lets a human see world state and feedback (HUD, action log) and send actions through the existing UI shell (`Group24_main.cpp`). **WebTextbox** is the C++ type used for independent text regions in the browser (HTML `div` elements via Emscripten).

## Public API (group contract)

| Method | Behavior |
|--------|----------|
| `Create()` | Create/attach the DOM node (`EnsureCreated`). |
| `SetText(...)` | Replace all text. |
| `AppendLine(...)` | Append one line; inserts `\n` between lines when content already exists. |
| `Clear()` | Clear text (`ClearText`). |
| `SetStyle(snapshot)` | Apply a `StyleSnapshot` (`ApplySnapshot`). |
| `Show()` / `Hide()` | Visibility (`SetVisible`). |

Additional methods (`SetFontSize`, `SetTextColor`, `CaptureStyle`, etc.) support styling and testing.

## Inputs and outputs

- **Inputs:** Text and style updates from the driver. For the stub demo, strings are built from `HudState` via `IWorldUiAdapter::GetHudState()` (world name, mode, tick, selected cell, resources, status message).
- **Outputs:** Rendered text in one or more `WebTextbox` instances (e.g. HUD panel and message log).

## Integration points

- **Driver:** [`source/Group24_main.cpp`](../../source/Group24_main.cpp) — constructs `WebTextbox` objects, attaches them to host divs (`group24_hud_host`, `group24_log_host`), and updates them in `RenderWorld()`.
- **World data:** [`source/tools/IWorldUiAdapter.hpp`](../../source/tools/IWorldUiAdapter.hpp) — `HudState` supplies strings; **do not** change this interface without team agreement.

## Fallback / stub behavior

- **Save / Load:** When underlying hooks are stubs, status text may still report adapter messages (e.g. from `StubWorldAdapter`). If a feature is not wired, the UI should show a clear status line when the team adds that messaging to `HudState.status_message` or a dedicated field.

## Build and test

- **Unit tests (native):** `bash tests/run_webtextbox_tests.sh`
- **Browser demo:** `bash demos/web/group24_build_emscripten.sh`, then serve the repository root and open `group24_demo.html`.

## Advanced C++ (course rubric)

Implemented in `WebTextbox` with comments in source: templates (`SetText` / `AppendText` / `AppendLine`), `constexpr` opacity clamp, value semantics (`StyleSnapshot`), `std::ranges::for_each` in `SyncAll_`, and lambdas for batch style push.
