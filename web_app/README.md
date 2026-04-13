## Author - Prijam Khanal
## Citation - ChatGPT LLM (OpenAI) was used to plan the structure of this file and help generate parts of this file (especially the command lines and codes). The document was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.

# Company C Web Interface

Landing page for the Web Interface module. Uses WebTextbox, WebButton, WebImage, and WebCanvas. Displays "Company C" with a
decorative canvas background, logo placeholder, subtitle, and interactive
Explore button.

## Setup Emscripten (one-time)

If `em++` is not in your PATH, install Emscripten using the emsdk:

```bash
# Clone the emsdk
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk

# Install and activate the latest SDK
./emsdk install latest
./emsdk activate latest

# Add to your shell (run this in every new terminal, or add to ~/.zshrc / ~/.bashrc)
source ./emsdk_env.sh
```

To make it permanent, add this line to your `~/.zshrc` (or `~/.bashrc`):

```bash
source ~/emsdk/emsdk_env.sh
```

Then open a new terminal. Verify with: `em++ --version`.

## Build

From the repository root:

```bash
bash web_app/build.sh
```

This compiles `main.cpp`, `AppUI.cpp`, and all web tool sources (WebTextbox,
WebButton, WebImage, WebCanvas) into `app.js` and `app.wasm` in
this folder.

## Run

Serve the app with any static HTTP server. For example:

```bash
cd web_app && python3 -m http.server 8080
```

Then open [http://localhost:8080/](http://localhost:8080/) in your browser.

Alternatively, from the repo root:

```bash
python3 -m http.server 8080
```

Then open [http://localhost:8080/web_app/](http://localhost:8080/web_app/).

## Structure

- `main.cpp` - Entry point; exports `RunWebInterface` for Emscripten
- `AppUI.hpp` / `AppUI.cpp` - UI composition using all web classes
- `index.html` - Page structure with `#canvas-bg` and `#content` containers

## After Changes

Re-run the build script, then refresh the browser.
