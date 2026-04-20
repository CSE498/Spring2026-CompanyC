#pragma once

#include <string>

namespace cse498 {

/// Queues a modal dialog in the browser (Emscripten). Each message is shown
/// one at a time; the next appears after the user dismisses the current dialog.
/// On native builds this is a no-op so tests compile without a browser.
void EnqueueWebPopup(const std::string& message);

}  // namespace cse498
