#pragma once

#include <string>

namespace cse498 {

/// Queues a modal dialog in the browser (Emscripten). The message body uses
/// WebTextbox and the dismiss control uses WebButton on a minimal DOM shell.
/// Each message is shown one at a time; the next appears after dismissal.
/// On native builds this is a no-op so tests compile without a browser.
void EnqueueWebPopup(const std::string& message);

}  // namespace cse498
