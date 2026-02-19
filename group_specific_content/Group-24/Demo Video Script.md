## Part 1

## Part 2
Thank you Sadwal. Hi, I am Prijam. I’m going to give a quick overview of our WebTextbox class.
WebTextbox is a small C++ wrapper that manages a text block in the browser as an HTML div, and it’s designed to compile to WebAssembly with Emscripten. The core idea is you configure everything from C++ using a clean interface, and when you call EnsureCreated(), it creates the DOM element and syncs all stored state to it. It’s also safe to use in native builds, because we provide native stubs so the same code can be unit-tested without a browser.
Functionality-wise, it supports setting and updating the text, font family, size, weight, style, and line height. You can change text color and background color, text alignment, and layout like position and size. We also added text decoration, word-wrap behavior, and padding. For visibility, there’s SetVisible and SetOpacity, where opacity is clamped safely to the valid range. You can optionally set an element ID and parent container ID to attach the textbox to a specific DOM node.
For lifecycle, Destroy() fully deletes the element, and RemoveFromDom() detaches it without destroying the handle so it can be re-attached later.
Testing-wise, we have 22 unit tests that cover defaults, setters/getters, edge cases, lifecycle behavior, move semantics, and safety checks like double-destroy and destructor cleanup, and all tests pass.
Now, let’s move towards the WebButton class.
