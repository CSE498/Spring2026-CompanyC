/**
 * @file WebCanvas.cpp
 * @author Sadwal Patel, Naod Ghebredngl, Tess Gonda
 * @brief Declaration of the WebCanvas wrapper used to manage an HTML canvas
 *        element for Group 24's browser-based interface.
 *
 * This class provides reusable drawing and interaction functionality for the
 * module, including primitive rendering, grid/cell helpers, entity drawing,
 * coordinate mapping, and click handling.
 *
 * Copyright (c) 2026 Sadwal Patel, Naod Ghebredngl 
 * SPDX-License-Identifier: MIT
 *
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this
 * file. The code was then reviewed and heavily edited by the author to ensure
 * correctness and suitability for the project.
 */


#include "WebCanvas.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#endif

namespace cse498 {
namespace {

static constexpr double kTwoPi = 6.28318530717958647692;

template <typename... Ts>
inline void Unused(Ts&&...) {}

}  // namespace

#ifdef __EMSCRIPTEN__
EM_JS(void, CSE498_RegisterCanvasClickHandlerJs,
      (const char* element_id, std::uintptr_t self_ptr), {
        var id = UTF8ToString(element_id);
        var canvas = document.getElementById(id);
        if (!canvas) return;

        if (!Module.__cse498CanvasClicks) {
          Module.__cse498CanvasClicks = {};
        }
        if (Module.__cse498CanvasClicks[id]) {
          return;
        }
        Module.__cse498CanvasClicks[id] = true;

        canvas.addEventListener('click', function(evt) {
          var rect = canvas.getBoundingClientRect();
          var x = Math.floor(evt.clientX - rect.left);
          var y = Math.floor(evt.clientY - rect.top);
          Module.ccall('CSE498_WebCanvasHandleClickBridge',
                       null,
                       ['number', 'number', 'number'],
                       [self_ptr, x, y]);
        });
      });

extern "C" {
EMSCRIPTEN_KEEPALIVE void CSE498_WebCanvasHandleClickBridge(
    std::uintptr_t self_ptr, int pixel_x, int pixel_y) {
  if (self_ptr == 0) return;
  reinterpret_cast<WebCanvas*>(self_ptr)->HandleClick(pixel_x, pixel_y);
}
}
#endif

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

  val document = val::global("document");
  canvas_ = document.call<val>("getElementById", id_);
  if (canvas_.isNull() || canvas_.isUndefined()) {
    canvas_ = document.call<val>("createElement", std::string("canvas"));
    canvas_.set("id", id_);
    document["body"].call<void>("appendChild", canvas_);
  }

  canvas_.set("width", width_);
  canvas_.set("height", height_);
  canvas_["style"].set("display", std::string("block"));

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

void WebCanvas::RegisterClickBridge_() {
  if (click_bridge_registered_) return;
  EnsureReady_();
  CSE498_RegisterCanvasClickHandlerJs(id_.c_str(),
                                      reinterpret_cast<std::uintptr_t>(this));
  click_bridge_registered_ = true;
}
#endif

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

  val document = val::global("document");
  val parent = document.call<val>("getElementById", parent_id);
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
  std::cout << "[WebCanvas] Resize to " << width_ << "x" << height_ << "\n";
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

void WebCanvas::SetLineWidth(float width) {
  if (width <= 0.0f) {
    throw std::invalid_argument(
        "WebCanvas::SetLineWidth: line width must be positive");
  }

#ifdef __EMSCRIPTEN__
  EnsureReady_();
  ctx_.set("lineWidth", width);
#else
  Unused(width);
#endif
}

void WebCanvas::SetFont(const std::string& css_font) {
  if (css_font.empty()) {
    throw std::invalid_argument("WebCanvas::SetFont: font cannot be empty");
  }

#ifdef __EMSCRIPTEN__
  EnsureReady_();
  ctx_.set("font", css_font);
#else
  Unused(css_font);
#endif
}

void WebCanvas::FillRect(float x, float y, float width, float height) {
  if (width <= 0.0f || height <= 0.0f) {
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
  if (width <= 0.0f || height <= 0.0f) {
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

void WebCanvas::FillCircle(float cx, float cy, float radius) {
  if (radius <= 0.0f) {
    throw std::invalid_argument("WebCanvas::FillCircle: invalid radius");
  }

#ifdef __EMSCRIPTEN__
  EnsureReady_();
  ctx_.call<void>("beginPath");
  ctx_.call<void>("arc", cx, cy, radius, 0.0, kTwoPi);
  ctx_.call<void>("fill");
#else
  Unused(cx, cy, radius);
#endif
}

void WebCanvas::DrawText(float x, float y, const std::string& text) {
  if (text.empty()) {
    return;
  }

#ifdef __EMSCRIPTEN__
  EnsureReady_();
  ctx_.call<void>("fillText", text, x, y);
#else
  Unused(x, y, text);
#endif
}

void WebCanvas::DrawCell(int col, int row, int cell_width, int cell_height,
                         const std::string& fill_css,
                         const std::string& glyph,
                         const std::string& glyph_css) {
  if (col < 0 || row < 0) {
    throw std::invalid_argument("WebCanvas::DrawCell: row/col cannot be negative");
  }
  if (cell_width <= 0 || cell_height <= 0) {
    throw std::invalid_argument("WebCanvas::DrawCell: invalid cell size");
  }

  const float x = static_cast<float>(col * cell_width);
  const float y = static_cast<float>(row * cell_height);
  SetFillColor(fill_css);
  FillRect(x, y, static_cast<float>(cell_width), static_cast<float>(cell_height));

#ifdef __EMSCRIPTEN__
  if (!glyph.empty()) {
    EnsureReady_();
    ctx_.set("fillStyle", glyph_css);
    ctx_.set("font",
             std::to_string(std::max(12, cell_height - 6)) + "px sans-serif");
    ctx_.set("textAlign", std::string("center"));
    ctx_.set("textBaseline", std::string("middle"));
    ctx_.call<void>("fillText", glyph,
                    x + static_cast<float>(cell_width) * 0.5f,
                    y + static_cast<float>(cell_height) * 0.5f);
  }
#else
  Unused(glyph_css);
  Unused(glyph);
#endif
}

void WebCanvas::DrawEntity(float center_x, float center_y, float radius,
                           const std::string& fill_css,
                           const std::string& glyph,
                           const std::string& glyph_css) {
  if (radius <= 0.0f) {
    throw std::invalid_argument("WebCanvas::DrawEntity: invalid radius");
  }

  SetFillColor(fill_css);
  FillCircle(center_x, center_y, radius);

#ifdef __EMSCRIPTEN__
  if (!glyph.empty()) {
    EnsureReady_();
    ctx_.set("fillStyle", glyph_css);
    ctx_.set("font",
             std::to_string(std::max(12, static_cast<int>(radius * 1.5f))) +
                 "px sans-serif");
    ctx_.set("textAlign", std::string("center"));
    ctx_.set("textBaseline", std::string("middle"));
    ctx_.call<void>("fillText", glyph, center_x, center_y);
  }
#else
  Unused(glyph_css);
  Unused(glyph);
#endif
}

//
// Image drawing helpers
//

#ifdef __EMSCRIPTEN__

EM_JS(void, CSE498_PreloadImageJs, (const char* url), {
  if (!window.__cse498Images) window.__cse498Images = {};
  var src = UTF8ToString(url);
  if (window.__cse498Images[src]) return;
  var img = new Image();
  img.onload = function() {
    // Trigger a redraw (action code 0 = no-op redraw) once the image is ready.
    if (Module && Module.ccall) {
      Module.ccall('HandleAction', null, ['number'], [0]);
    }
  };
  img.src = src;
  window.__cse498Images[src] = img;
});

EM_JS(int, CSE498_IsImageReady, (const char* url), {
  var src = UTF8ToString(url);
  var img = window.__cse498Images && window.__cse498Images[src];
  return (img && img.complete && img.naturalWidth > 0) ? 1 : 0;
});

EM_JS(void, CSE498_DrawImageJs,
      (const char* canvas_id,
       const char* url, float x, float y, float w, float h), {
  var canvas = document.getElementById(UTF8ToString(canvas_id));
  if (!canvas) return;
  var ctx = canvas.getContext('2d');
  var src = UTF8ToString(url);
  var img = window.__cse498Images && window.__cse498Images[src];
  if (img && img.complete && img.naturalWidth > 0) {
    ctx.drawImage(img, x, y, w, h);
  }
});

#endif  // __EMSCRIPTEN__

/*static*/ void WebCanvas::PreloadImage(const std::string& url) {
  if (url.empty()) return;
#ifdef __EMSCRIPTEN__
  CSE498_PreloadImageJs(url.c_str());
#else
  Unused(url);
#endif
}

void WebCanvas::DrawImage(const std::string& url,
                          float x, float y, float w, float h,
                          const std::string& fallback_fill_css) {
  if (w <= 0.0f || h <= 0.0f) {
    throw std::invalid_argument("WebCanvas::DrawImage: width and height must be positive");
  }
#ifdef __EMSCRIPTEN__
  EnsureReady_();
  if (!url.empty() && CSE498_IsImageReady(url.c_str())) {
    CSE498_DrawImageJs(id_.c_str(), url.c_str(), x, y, w, h);
  } else {
    SetFillColor(fallback_fill_css);
    FillRect(x, y, w, h);
  }
#else
  Unused(url, x, y, w, h, fallback_fill_css);
#endif
}

void WebCanvas::DrawCellImage(int col, int row,
                              int cell_width, int cell_height,
                              const std::string& url,
                              const std::string& fallback_fill_css,
                              const std::string& fallback_glyph) {
  if (col < 0 || row < 0 || cell_width <= 0 || cell_height <= 0) return;
#ifdef __EMSCRIPTEN__
  if (!url.empty() && CSE498_IsImageReady(url.c_str())) {
    const float x = static_cast<float>(col * cell_width);
    const float y = static_cast<float>(row * cell_height);
    const float w = static_cast<float>(cell_width);
    const float h = static_cast<float>(cell_height);
    CSE498_DrawImageJs(id_.c_str(), url.c_str(), x, y, w, h);
    return;
  }
#else
  Unused(url);
#endif
  // Fallback: color fill + glyph (same as DrawCell).
  DrawCell(col, row, cell_width, cell_height,
           fallback_fill_css, fallback_glyph, "#000000");
}

void WebCanvas::Present() {
#ifdef __EMSCRIPTEN__
  EnsureReady_();
#else
  std::cout << "[WebCanvas] Present\n";
#endif
}

void WebCanvas::SetClickHandler(const std::function<void(int, int)>& handler) {
  click_handler_ = handler;
#ifdef __EMSCRIPTEN__
  if (click_handler_) {
    RegisterClickBridge_();
  }
#endif
}

std::pair<int, int> WebCanvas::PixelToCell(int pixel_x, int pixel_y,
                                           int cell_width,
                                           int cell_height) const {
  if (cell_width <= 0 || cell_height <= 0) {
    throw std::invalid_argument("WebCanvas::PixelToCell: invalid cell size");
  }
  if (pixel_x < 0 || pixel_y < 0) {
    return std::make_pair(-1, -1);
  }
  return std::make_pair(pixel_x / cell_width, pixel_y / cell_height);
}

void WebCanvas::HandleClick(int pixel_x, int pixel_y) {
  if (click_handler_) {
    click_handler_(pixel_x, pixel_y);
  }
}

void WebCanvas::DrawGrid(int cols, int rows, int cell_width, int cell_height,
                         const std::function<void(int, int)>& draw_cell_fn) {
  if (cols <= 0 || rows <= 0) {
    throw std::invalid_argument("WebCanvas::DrawGrid: cols/rows must be positive");
  }
  if (cell_width <= 0 || cell_height <= 0) {
    throw std::invalid_argument("WebCanvas::DrawGrid: invalid cell size");
  }
  if (!draw_cell_fn) {
    throw std::invalid_argument("WebCanvas::DrawGrid: draw_cell_fn must be valid");
  }

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      draw_cell_fn(col, row);
    }
  }
}

void WebCanvas::HighlightCell(int col, int row, int cell_width, int cell_height,
                              const std::string& stroke_css,
                              float line_width) {
  if (col < 0 || row < 0) {
    throw std::invalid_argument(
        "WebCanvas::HighlightCell: row/col cannot be negative");
  }
  if (cell_width <= 0 || cell_height <= 0) {
    throw std::invalid_argument(
        "WebCanvas::HighlightCell: invalid cell size");
  }
  if (line_width <= 0.0f) {
    throw std::invalid_argument(
        "WebCanvas::HighlightCell: line width must be positive");
  }

  const float x = static_cast<float>(col * cell_width);
  const float y = static_cast<float>(row * cell_height);

  SetStrokeColor(stroke_css);
  SetLineWidth(line_width);
  StrokeRect(x, y, static_cast<float>(cell_width),
             static_cast<float>(cell_height));
}

}  // namespace cse498
