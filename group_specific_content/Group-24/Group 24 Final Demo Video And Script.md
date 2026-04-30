# Group 24 Final Demo Video Overview – Script

**Video Link**: [Final Demo Video](https://youtu.be/O0JACvWOjW8)

## Tess - WebButton Improvements
I've made a few key upgrades to WebButton. The first is template callable support: the constructor and SetOnClick now accept any callable: lambdas, function pointers, or functors. You're no longer locked into std:: function at the call site.

I also added FreezeDOM and UnfreezeDiM for batching DOM updates. When you call SetBorder, instead of pushing three separate style updates to the DOM, it freezes, makes all three changes, then flushes once on unfreeze. This is per-instance, so multiple buttons don't interfere with each other.

Finally, I added move semantics so buttons can safely live in containers like std:: vector without double-destruction bugs. Copy is still deleted. The test suite now has 15 test cases covering all of this - template callables, freeze batching, move semantics, and unique IDs. That's everything new for WebButton.

## Prijam - WebTextBox Improvements
To improve my class, I focused on making WebTextbox easier to use, more consistent with our group API, and safer for real UI layout in the demo.

In WebTextbox.hpp, I added cleaner alias methods like Create, Clear, Show, Hide, and SetStyle, so calling code reads better without changing core behavior. I also added AppendLine, which appends text line-by-line and automatically inserts newlines, including support for numeric values.

In WebTextbox.cpp, I added ApplyFlowLayoutStyles. This is important because the textbox default style is overlay-oriented, but our sidebar and stacked panels need flow layout behavior. So this method switches key CSS styles like relative positioning, full width, border-box sizing, and a minimum height, with a named constant to avoid magic strings.

For testing, in WebTextboxTest.cpp, I expanded coverage with new tests for AppendLine, the new alias API contract, and flow layout application. I also updated style snapshot usage to call SetStyle instead of ApplySnapshot, matching the improved public API.

Overall, this commit made WebTextbox cleaner to integrate, clearer to read, and better validated through tests.

## Naod Overall Changes

## SadWal Overall Changes

## Tess Overall Changes
I added sprite rendering to the module. On the WebCanvas side, l added three new methods: PreloadImage starts loading an image into the browser cache in the background. DrawImage draws it into any rectangle, and DrawCellImage scales it to fill one grid cell. Both fall back to the existing color-fill-and-glyph path if the image isn't loaded yet, so the game never shows a blank cell during load.

I threaded the image URL through the data model - RenderableEntity got one new image_url field. WebInterface stores a glyph-to-URL map and fills it in GetEntities. At the WebApp level, world authors just call RegisterEntityVisual with a glyph character and a path, and RegisterCellBackground for tiles that need a background layer under the sprite.

The last feature is SetPlayerVisible. Group 7's world has an invisible camera agent - the player controls a viewport but shouldn't appear as a dot on screen. One flag in WebInterface, one check in GetEntities, and one call in the world setup is all it takes.

## Prijam - Overall Changes
I added two feature areas: AutoTickPolicy and WebPopup, and I backed both with tests.
First, in AutoTickPolicy.hpp, I added a small decision layer for starting auto-tick safely.

DecideAutoTickStart checks whether a popup is visible, and if it is, it blocks auto-tick and returns a retry delay.
I also added ClampAutoTickIntervalMs, which enforces a minimum tick interval so timing stays safe and predictable.
Second, in WebPopup.hpp and WebPopup.cpp, I added a structured popup system with queueing and options.


I introduced WebPopupOptions and WebPopupRequest, then implemented EnqueueWebPopup so popups can be shown with three behaviors: OK-button dismiss, timed auto-dismiss, or both.

On the web path, I create a modal overlay, build the message using WebTextbox, create an optional OK button with WebButton, support Escape and overlay-click dismissal, and process popups in FIFO order.
I also normalize invalid option combinations and handle default timer behavior safely.
For validation, I added all necessary tests in AutoTickPolicyTest.cpp and WebPopupTest.cpp, covering defaults, edge cases, parsing behavior, queue behavior, and expected popup handling paths.
Overall, these additions make auto-tick safer and popup interactions more robust and user-friendly.

I also made sure the dropdown button at the home page correctly navigates to different worlds.

## Abigail - Overall Changes

## Tess - Web Interface Demo
This is Group 24's web interface module - a C++ library that lets any Company C world render itself in the browser via Emscripten, with no JavaScript written by the world author. Everything you see here is generated entirely from C++.

When you hit Start, the simulation runs. The grid is drawn by WebCanvas, which handles sprite rendering for cells and agents - falling back to colored glyphs if an image isn't loaded yet. The HUD on the right updates every tick, showing the world name, current mode, tick count, and status messages.

Action buttons are built with WebButton and wired up automatically - each one has a label, a hotkey, and knows whether it should be disabled in Setup mode versus Live mode. Clicking a cell highlights it and shows its type in the HUD.

The dropdown lets you swap between worlds - maze, interaction, dynamic, sokoban - all loading into the same Ul shell. Save and load are wired through the sync layer. The analysis panel tracks action frequency across the session.

Popup messages can be timed or require a dismissal, and the viewport supports zoom. The whole thing is designed so other groups drop in one header, register their visuals, and get a working browser interface.
