#pragma once
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {

class WebCanvas {
public:
    WebCanvas(const std::string& canvas_id, int width, int height);
    ~WebCanvas() = default;

    // Basic info
    const std::string& GetElementID() const noexcept { return id_; }
    int getWidth() const noexcept { return width_; }
    int getHeight() const noexcept { return height_; }

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
    void FillRect(float x, float y, float w, float h);
    void StrokeRect(float x, float y, float w, float h);
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

} // namespace cse498
