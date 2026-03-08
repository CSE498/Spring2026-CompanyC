#pragma once
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {

/**
 * @class WebCanvas
 * @brief Manages an HTML5 canvas element through a C++ interface.
 *
 * WebCanvas provides functionality for creating, initializing, and drawing
 * on an HTML5 <canvas> element when compiled with Emscripten. When compiled
 * natively (without Emscripten), drawing operations are stubbed out so the
 * class can still be compiled and tested.
 *
 * Typical usage:
 *   WebCanvas canvas("main_canvas", 800, 600);
 *   canvas.Clear();
 *   canvas.SetFillColor("#ff0000");
 *   canvas.FillRect(50, 50, 100, 100);
 *
 * The class performs argument validation and throws:
 *   - std::invalid_argument for programmer errors (e.g., invalid dimensions)
 *   - std::runtime_error for runtime DOM/context failures
 */
class WebCanvas {
public:
    WebCanvas(const std::string& canvas_id, int width, int height);
    ~WebCanvas() = default;

    // Basic info
    const std::string& GetElementID() const noexcept { return id_; }
    int GetWidth() const noexcept { return width_; }
    int GetHeight() const noexcept { return height_; }

    // Canvas lifecycle / layout
    void AppendTo(const std::string& parent_id);
    void Resize(int new_width, int new_height);
    bool IsReady() const noexcept;

    // Drawing state
    void Clear();
    void SetFillColor(const std::string& css);
    void SetStrokeColor(const std::string& css);
    void SetLineWidth(float w);

    // Drawing primitives
    void FillRect(float x, float y, float width, float height);
    void StrokeRect(float x, float y, float width, float height);
    void DrawLine(float x1, float y1, float x2, float y2);
    void FillCircle(float cx, float cy, float r);

private:
    std::string id_;
    int width_;
    int height_;

#ifdef __EMSCRIPTEN__
    emscripten::val canvas_;
    emscripten::val ctx_;

    void InitDOM();
    void EnsureReady_() const;
#endif
};

}  // namespace cse498