# Group 24: Web Interface Video Overview – Script

**Video Link**: [Web Interface Video Overview](https://youtu.be/FjQu19C_hbk?si=GpEcDpIjnlHe1V0A)

## WebImage Class - Sadwal Patel
Hi, I'm Sadwal Patel from Group 24 Web Interface. In this project I implemented the WebImage class for the 
Web Interface module.

The goal of WebImage is to give us a simple C++ interface to control an HTML element when we compile with 
Emscripten, while still being safe and testable in a normal native C++ build.

On screen I’m going to open source/tools/WebImage.h. WebImage stores the image state—like the image source 
path, alt text, position and size in pixels, visibility, opacity, z-index, a parent element ID, and an 
optional DOM element ID. It also supports CSS classes and arbitrary inline style key/value pairs.

The key idea is lazy creation. You can set all properties first, and then call EnsureCreated() when you 
actually want the DOM element to exist. After creation, any updates synchronize to the DOM when running under 
Emscripten. I also implemented RemoveFromDom() versus Destroy() so we can detach the element from the page or 
fully delete it and reset the internal handle.

I made the class safe for project integration by supporting move construction and move assignment. That 
prevents double-destruction bugs if WebImage objects get moved around in containers.

Next, I’ll show the unit tests in tests/WebImage.cpp. These tests run with strict flags including -Werror, 
and they cover defaults, setters, clamping behavior like opacity, CSS class operations, lifecycle 
create/destroy/recreate, and move semantics.

Finally, I’ll show the browser smoke demo in source/tools/webimage_smoke_demo.cpp and the generated 
webimage_demo.html. When I run it in the browser, you can see the image render and update—proving the 
Emscripten/DOM integration works.

In the full system, other modules can use WebImage for things like avatars, icons, sprites, or UI graphics by 
setting a parent container, positioning, and updating properties during the simulation loop. That’s everything 
I contributed—thanks. Now I’ll hand over to Prijam for WebTextbox.

## WebTextbox Class - Prijam Khanal
Thank you Sadwal. Hi, I am Prijam. I’m going to give a quick overview of our WebTextbox class.

WebTextbox is a small C++ wrapper that manages a text block in the browser as an HTML div, and it’s designed to 
compile to WebAssembly with Emscripten. The core idea is you configure everything from C++ using a clean interface, 
and when you call EnsureCreated(), it creates the DOM element and syncs all stored state to it. It’s also safe to 
use in native builds, because we provide native stubs so the same code can be unit-tested without a browser.

Functionality-wise, it supports setting and updating the text, font family, size, weight, style, and line height. 
You can change text color and background color, text alignment, and layout like position and size. We also added 
text decoration, word-wrap behavior, and padding. For visibility, there’s SetVisible and SetOpacity, where opacity 
is clamped safely to the valid range. You can optionally set an element ID and parent container ID to attach the 
textbox to a specific DOM node.

For lifecycle, Destroy() fully deletes the element, and RemoveFromDom() detaches it without destroying the handle 
so it can be re-attached later.

Testing-wise, we have 22 unit tests that cover defaults, setters/getters, edge cases, lifecycle behavior, move 
semantics, and safety checks like double-destroy and destructor cleanup, and all tests pass.

Now, let’s move towards the WebButton class.

## WebButton Class - Tess Gonda
Thanks, Prijam! I'm Tess Gonda, and I'll be walking you through the WebButton class I developed for our 
Web Interface module. This class provides the foundation for interactive buttons in our C++ web applications.

The WebButton class creates and manages HTML button elements directly from C++ code. For basic operations, 
you can create buttons with custom labels, enable or disable them, and manage toggle states for things like 
pressed buttons or active filters. For event handling, I implemented a callback system using std::function. 
You can attach any function to a button click, and the class handles all the JavaScript event listener wiring 
automatically. It also catches and logs exceptions in callbacks to prevent crashes. For styling, I provided 
full control over appearance, like dimensions, colors, borders, and border radius. The class maintains all 
styling states internally and applies it to the DOM automatically. I also included a ResetStyle() method to 
restore default styling.

WebButton uses a static ID counter to ensure that every button has a unique identifier. The class automatically 
manages a global registry mapping button IDs to C++ instances, which is important for handling JavaScript 
callbacks. I implemented comprehensive error handling: throwing std::invalid_argument for programmer errors 
like negative dimensions, while handling runtime issues like missing parent elements.

The class has 15 Catch2 test cases covering 70 assertions, and all tests pass! WebButton is now (hopefully) ready 
for use by our team and the other groups in Company C.

That's all for the WebButton class, now onto WebCanvas!

## WebCanvas Class - Naod Ghebredngl
Thanks, Tess. I'm Naod, and I'll be explaining the WebCanvas class I implemented for our Web Interface module.

WebCanvas provides a C++ interface for managing an HTML5 canvas, allowing other parts of our project to perform 
drawing operations directly from C++ without writing JavaScript.

The constructor validates the canvas ID and dimensions, stores them internally, and when compiled with Emscripten 
it initializes the DOM canvas and grabs the 2D rendering context. That logic lives in InitDOM, which either finds 
an existing canvas by ID or creates one and attaches it to the page

I also built in runtime safety checks with EnsureReady, so drawing calls fail clearly if the canvas or context 
wasn't initialized properly.

On top of that, the class exposes a small set of drawing primitives — Clear, FillRect, StrokeRect, DrawLine, and 
FillCircle, these will be the functions that will be continued to be built upon.

That's it for the WebCanvas class, thank you.

## WebLayout Class - Abigail MacKersie
Hello, this is Abigail MacKersie, and I will be describing our last class, WebLayout. It's currently in a very 
rudimentary state due to several issues with getting Emscripten to read from and write to HTML code, but the 
vision for this class is to create a way to interface directly with the HTML DOM solely through C++. In the future, 
it will hopefully be able to insert, edit, and delete elements without the user ever having to touch the actual 
HTML file for a seamless C++ module development process.

Nested inside the class are a few structs and enum classes that will help with organization of the HTML elements as 
nodes, a struct that I created to hold all of the necessary information for an HTML element such as the tag's name 
and any classes or attributes held within it. These nodes will be organized in a tree-like structure of unique 
pointers, with each node having a unique pointer to their parent node (or nullptr, if it is the root) and a vector 
of child nodes.

There are also a few utility functions designed to pull the metadata of the HTML elements, throw errors if a 
pointer to a node is not initialized correctly, and to convert a string to full lowercase for consistency. 

There are no current test cases to confirm that this works with 100% certainty pushed to the code base (as many 
attempts to get Emscripten to recognize the HTML file have failed), but they will be added as soon as they are 
completed.

Thank you for watching, and I hope that this helped with understanding our code.