#include "WebCanvas.hpp"

#include <iostream>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

// Note : This class used ChatGPT in function development

namespace cse498 {

// Helpers/constants local to this translation unit.
namespace {

// Named constant to avoid magic numbers.
static constexpr double kTwoPi = 6.283185307179586;

// Helper to silence unused-parameter warnings in native builds.
template <typename... Ts>
inline void Unused(Ts&&...) {}

}  // namespace

WebCanvas::WebCanvas(const std::string& canvas_id, int width, int height)
    : id_(canvas_id), width_(width), height_(height) {
    if (id_.empty()) {
        throw std::invalid_argument("WebCanvas: canvas_id cannot be empty");
    }
    if (width_ <= 0 || height_ <= 0) {
        throw std::invalid_argument("WebCanvas: width and height must be positive");
    }

#ifdef __EMSCRIPTEN__
    InitDOM();
#else
    std::cout << "[WebCanvas] Created '" << id_ << "' size "
              << width_ << "x" << height_ << "\n";
#endif
}

#ifdef __EMSCRIPTEN__
void WebCanvas::InitDOM() {
    using emscripten::val;

    auto document = val::global("document");

    canvas_ = document.call<val>("getElementById", id_);
    if (canvas_.isNull() || canvas_.isUndefined()) {
        canvas_ = document.call<val>("createElement", std::string("canvas"));
        canvas_.set("id", id_);
        document["body"].call<void>("appendChild", canvas_);
    }

    canvas_.set("width", width_);
    canvas_.set("height", height_);

    ctx_ = canvas_.call<val>("getContext", std::string("2d"));
    if (ctx_.isNull() || ctx_.isUndefined()) {
        throw std::runtime_error("WebCanvas: failed to acquire 2D context");
    }
}

void WebCanvas::EnsureReady_() const {
    if (canvas_.isNull() || canvas_.isUndefined() ||
        ctx_.isNull() || ctx_.isUndefined()) {
        throw std::runtime_error("WebCanvas: canvas/context not initialized");
    }
}
#endif  // __EMSCRIPTEN__

bool WebCanvas::IsReady() const noexcept {
#ifdef __EMSCRIPTEN__
    return !(canvas_.isNull() || canvas_.isUndefined() ||
             ctx_.isNull() || ctx_.isUndefined());
#else
    return true;
#endif
}

void WebCanvas::AppendTo(const std::string& parent_id) {
    if (parent_id.empty()) {
        throw std::invalid_argument("WebCanvas::AppendTo: parent_id cannot be empty");
    }

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    using emscripten::val;

    auto document = val::global("document");
    auto parent = document.call<val>("getElementById", parent_id);

    if (parent.isNull() || parent.isUndefined()) {
        throw std::runtime_error("WebCanvas::AppendTo: parent not found");
    }

    parent.call<void>("appendChild", canvas_);
#else
    std::cout << "[WebCanvas] AppendTo " << parent_id << "\n";
#endif
}

void WebCanvas::Resize(int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0) {
        throw std::invalid_argument("WebCanvas::Resize: invalid size");
    }

    width_ = new_width;
    height_ = new_height;

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    canvas_.set("width", width_);
    canvas_.set("height", height_);
#else
    std::cout << "[WebCanvas] Resize to "
              << width_ << "x" << height_ << "\n";
#endif
}

void WebCanvas::Clear() {
#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.call<void>("clearRect", 0, 0, width_, height_);
#else
    std::cout << "[WebCanvas] Clear\n";
#endif
}

void WebCanvas::SetFillColor(const std::string& css) {
#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.set("fillStyle", css);
#else
    Unused(css);
#endif
}

void WebCanvas::SetStrokeColor(const std::string& css) {
#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.set("strokeStyle", css);
#else
    Unused(css);
#endif
}

void WebCanvas::SetLineWidth(float w) {
    if (w <= 0.f) {
        throw std::invalid_argument(
            "WebCanvas::SetLineWidth: line width must be positive");
    }

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.set("lineWidth", w);
#else
    Unused(w);
#endif
}

void WebCanvas::FillRect(float x, float y, float width, float height) {
    if (width <= 0.f || height <= 0.f) {
        throw std::invalid_argument("WebCanvas::FillRect: invalid rectangle");
    }

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.call<void>("fillRect", x, y, width, height);
#else
    Unused(x, y, width, height);
#endif
}

void WebCanvas::StrokeRect(float x, float y, float width, float height) {
    if (width <= 0.f || height <= 0.f) {
        throw std::invalid_argument("WebCanvas::StrokeRect: invalid rectangle");
    }

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.call<void>("strokeRect", x, y, width, height);
#else
    Unused(x, y, width, height);
#endif
}

void WebCanvas::DrawLine(float x1, float y1, float x2, float y2) {
#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.call<void>("beginPath");
    ctx_.call<void>("moveTo", x1, y1);
    ctx_.call<void>("lineTo", x2, y2);
    ctx_.call<void>("stroke");
#else
    Unused(x1, y1, x2, y2);
#endif
}

void WebCanvas::FillCircle(float cx, float cy, float r) {
    if (r <= 0.f) {
        throw std::invalid_argument("WebCanvas::FillCircle: invalid radius");
    }

#ifdef __EMSCRIPTEN__
    EnsureReady_();
    ctx_.call<void>("beginPath");
    ctx_.call<void>("arc", cx, cy, r, 0.0, kTwoPi);
    ctx_.call<void>("fill");
#else
    Unused(cx, cy, r);
#endif
}

}  // namespace cse498