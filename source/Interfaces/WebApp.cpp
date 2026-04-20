// WebApp.cpp
//
// Implementation of the WebApp application class.  Contains the UI shell,
// Emscripten/JavaScript bridge code, canvas/HUD rendering, and the exported
// HandleAction entry point.
//
// See Interfaces/WebApp.hpp for the class interface and usage notes.

#include "Interfaces/WebApp.hpp"
#include "tools/WebAssetKeys.hpp"

#include <emscripten.h>

#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


namespace {

// ---------------------------------------------------------------------------
// Layout and rendering constants
// ---------------------------------------------------------------------------

constexpr int kCanvasWidthPx  = 480;
constexpr int kCanvasHeightPx = 480;

// DOM host ids and shared font stack (match #cse498_root typography).
constexpr const char* kHudHostElementId = "cse498_hud_host";
constexpr const char* kLogHostElementId = "cse498_log_host";
constexpr const char* kSidebarFontStack = "system-ui, Arial, sans-serif";

// Cell and grid rendering colors.
constexpr const char* kColorCellFallback = "#e5e7eb";  // fill when no legend entry matched
constexpr const char* kColorCellText     = "#111827";  // glyph text on cells
constexpr const char* kColorCellEmpty    = "#f9fafb";  // background for blank cells
constexpr const char* kColorGridLine     = "#d1d5db";  // grid stroke
constexpr const char* kColorEntityText   = "#ffffff";  // glyph text on entities

// ---------------------------------------------------------------------------
// JavaScript bridge functions (Emscripten)
// ---------------------------------------------------------------------------

EM_JS(void, WebAppEnsureUi, (), {
  if (!window.__cse498_style_loaded) {
    var style = document.createElement('style');
    style.textContent = [
      '#cse498_root{font-family:system-ui,Arial,sans-serif;padding:12px;color:#111827;}',
      '#cse498_topbar{display:flex;gap:8px;align-items:center;margin-bottom:12px;flex-wrap:wrap;}',
      '#cse498_main{display:flex;gap:12px;align-items:flex-start;}',
      '#cse498_canvas_host{min-width:500px;}',
      '#cse498_sidebar{width:320px;background:#f3f4f6;border:1px solid #d1d5db;border-radius:10px;padding:12px;}',
      '#cse498_actions{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px;margin-bottom:12px;}',
      '#cse498_actions button,#cse498_topbar button,#cse498_topbar select{padding:8px 10px;border:1px solid #9ca3af;border-radius:8px;background:white;}',
      '#cse498_hud_host,#cse498_log_host{white-space:pre-wrap;background:white;border:1px solid #d1d5db;border-radius:8px;padding:10px;margin-top:10px;overflow:visible;min-height:3em;}',
      '#cse498_title{font-size:20px;font-weight:700;margin-right:8px;}',
      '#cse498_canvas{border:1px solid #374151;border-radius:8px;background:#ffffff;}'
    ].join("");
    document.head.appendChild(style);
    window.__cse498_style_loaded = true;
  }

  if (!window.__cse498_keybound) {
    document.addEventListener('keydown', function(ev) {
      var key = ev.key.toLowerCase();
      // Codes must match the ActionCode enum in WebApp.hpp.
      var map = { 'w': 5, 'arrowup': 5, 's': 6, 'arrowdown': 6,
                  'a': 7, 'arrowleft': 7, 'd': 8, 'arrowright': 8,
                  'e': 9, 'b': 10, 'r': 2, 'enter': 1 };
      if (!(key in map)) return;
      ev.preventDefault();
      Module.ccall('HandleAction', null, ['number'], [map[key]]);
    });
    window.__cse498_keybound = true;
  }
});

EM_JS(void, WebAppPopulateUiControls, (), {
  var actions = document.getElementById('cse498_actions');
  var sidebar = document.getElementById('cse498_sidebar');

  if (!actions || !sidebar) return;

  // Hook topbar buttons created by WebLayout.
  // Codes 1-4 must match the ActionCode enum in WebApp.hpp.
  [1, 2, 3, 4].forEach(function(code) {
    var btn = document.getElementById('cse498btn-' + code);
    if (!btn || btn.__cse498_bound) return;
    btn.addEventListener('click', function() {
      Module.ccall('HandleAction', null, ['number'], [code]);
    });
    btn.__cse498_bound = true;
  });

  // Add sidebar label if missing.
  if (!document.getElementById('cse498_actions_label')) {
    var labelWrap = document.createElement('div');
    labelWrap.id = 'cse498_actions_label';
    labelWrap.innerHTML = '<strong>Available Actions</strong>';
    sidebar.insertBefore(labelWrap, actions);
  }

  // Fill action buttons only if missing.
  // Codes must match the ActionCode enum in WebApp.hpp.
  if (!document.getElementById('cse498btn-5')) {
    var actionButtons = [
      [5, 'Up'],
      [6, 'Down'],
      [7, 'Left'],
      [8, 'Right'],
      [9, 'Collect'],
      [10, 'Build']
    ];

    actionButtons.forEach(function(entry) {
      var code = entry[0];
      var text = entry[1];
      var btn = document.createElement('button');
      btn.id = 'cse498btn-' + code;
      btn.textContent = text;
      btn.addEventListener('click', function() {
        Module.ccall('HandleAction', null, ['number'], [code]);
      });
      actions.appendChild(btn);
    });
  }
});

EM_JS(void, WebAppSetModeTick, (const char* text_ptr), {
  var el = document.getElementById('cse498_mode_tick');
  if (!el) return;
  el.textContent = text_ptr ? UTF8ToString(text_ptr) : "";
});

EM_JS(void, WebAppSetActionEnabled, (int code, int enabled), {
  var btn = document.getElementById('cse498btn-' + code);
  if (!btn) return;
  btn.disabled = !enabled;
});

// Read a URL query parameter.  Returns a malloc'd C string or null.
EM_JS(char*, GetUrlParam_impl, (const char* name), {
  var params = new URLSearchParams(window.location.search);
  var val = params.get(UTF8ToString(name));
  if (!val) return 0;
  var len = lengthBytesUTF8(val) + 1;
  var ptr = _malloc(len);
  stringToUTF8(val, ptr, len);
  return ptr;
});

}  // namespace

std::string GetUrlParam(const std::string& name, const std::string& default_value) {
  char* raw = GetUrlParam_impl(name.c_str());
  std::string value = raw ? raw : default_value;
  std::free(raw);
  return value;
}

// ---------------------------------------------------------------------------
// WebApp method definitions
// ---------------------------------------------------------------------------

WebApp::WebApp() {
  WebAppEnsureUi();

  layout_ = std::make_unique<cse498::WebLayout>();
  layout_->createShell();
  WebAppPopulateUiControls();

  canvas_ = std::make_unique<cse498::WebCanvas>("cse498_canvas",
                                                kCanvasWidthPx, kCanvasHeightPx);
  canvas_->AppendTo("cse498_canvas_host");
  canvas_->SetClickHandler([this](int pixel_x, int pixel_y) {
    if (!interface_) return;
    const int cell_size = std::min(kCanvasWidthPx  / interface_->GetGridWidth(),
                                   kCanvasHeightPx / interface_->GetGridHeight());
    const auto cell  = canvas_->PixelToCell(pixel_x, pixel_y, cell_size, cell_size);
    interface_->SelectCell(cell.first, cell.second);
    RenderWorld();
  });

  hud_text_ = std::make_unique<cse498::WebTextbox>();
  log_text_ = std::make_unique<cse498::WebTextbox>();
  hud_text_->SetParentId(kHudHostElementId);
  log_text_->SetParentId(kLogHostElementId);
  hud_text_->SetWordWrap("break-word");
  log_text_->SetWordWrap("break-word");
  hud_text_->Create();
  log_text_->Create();
  hud_text_->ApplyFlowLayoutStyles();
  log_text_->ApplyFlowLayoutStyles();
  hud_text_->SetFontFamily(kSidebarFontStack);
  log_text_->SetFontFamily(kSidebarFontStack);
}

void WebApp::SetCellVisual(const std::string& cell_type_name,
                            std::string fill_css, std::string glyph) {
  if (interface_) {
    interface_->SetCellVisual(cell_type_name, std::move(fill_css), std::move(glyph));
  }
}

void WebApp::RegisterActionMeta(const std::string& action_id,
                                 cse498::WebInterface::ActionMeta meta) {
  if (interface_) {
    interface_->RegisterActionMeta(action_id, std::move(meta));
  }
}

void WebApp::RegisterEntityVisual(char glyph, std::string image_url) {
  if (interface_) {
    cse498::WebCanvas::PreloadImage(image_url);
    interface_->SetEntityVisual(glyph, std::move(image_url));
  }
}

void WebApp::RegisterCellBackground(const std::string& cell_type_name,
                                     std::string bg_image_url) {
  cse498::WebCanvas::PreloadImage(bg_image_url);
  cell_backgrounds_[cell_type_name] = std::move(bg_image_url);
}

void WebApp::Render() {
  if (!interface_) return;
  legend_by_id_.clear();
  for (const auto& entry : interface_->GetLegend()) {
    legend_by_id_[entry.id] = entry;
    // Preload the sprite for this cell type.
    cse498::WebCanvas::PreloadImage("assets/" + entry.key + ".png");
  }
  RenderWorld();
}

void WebApp::HandleAction(int action_code) {
  if (!interface_ || !world_) return;

  // Code 0 is a no-op redraw (used by the image preloader onload callback).
  if (action_code == 0) {
    RenderWorld();
    return;
  }

  const std::string action_id = ActionIdForCode(action_code);
  if (action_id.empty()) return;

  const bool is_move_action = IsMovementAction(action_code);
  std::pair<int, int> start_cell{0, 0};
  std::pair<int, int> attempted_cell{0, 0};
  const bool had_start_cell =
      is_move_action && TryGetPlayerCell(start_cell);
  if (had_start_cell) {
    attempted_cell = GetAttemptedMoveCell(action_code, start_cell);
  }

  // Submit the action to the interface (stores it as pending), then advance
  // the world by one turn (world calls SelectAction → DoAction on the agent).
  interface_->SubmitAction(action_id);
  world_->RunAgents();

  if (had_start_cell) {
    std::pair<int, int> end_cell{0, 0};
    if (TryGetPlayerCell(end_cell) && end_cell != start_cell) {
      interface_->SelectCell(end_cell.first, end_cell.second);
    } else {
      interface_->SelectCell(attempted_cell.first, attempted_cell.second);
    }
  }

  RenderWorld();
}

std::string WebApp::ActionIdForCode(int code) {
  switch (code) {
    case kActionStart:   return "start";
    case kActionReset:   return "reset";
    case kActionSave:    return "save";
    case kActionLoad:    return "load";
    case kActionUp:      return "up";
    case kActionDown:    return "down";
    case kActionLeft:    return "left";
    case kActionRight:   return "right";
    case kActionCollect: return "collect";
    case kActionBuild:   return "build";
    default:             return std::string();
  }
}

int WebApp::CodeForActionId(const std::string& action_id) {
  if (action_id == "start")   return kActionStart;
  if (action_id == "reset")   return kActionReset;
  if (action_id == "save")    return kActionSave;
  if (action_id == "load")    return kActionLoad;
  if (action_id == "up")      return kActionUp;
  if (action_id == "down")    return kActionDown;
  if (action_id == "left")    return kActionLeft;
  if (action_id == "right")   return kActionRight;
  if (action_id == "collect") return kActionCollect;
  if (action_id == "build")   return kActionBuild;
  return 0;
}

bool WebApp::IsMovementAction(int action_code) const {
  switch (action_code) {
    case kActionUp:
    case kActionDown:
    case kActionLeft:
    case kActionRight:
      return true;
    default:
      return false;
  }
}

bool WebApp::TryGetPlayerCell(std::pair<int, int>& out_cell) const {
  if (!interface_) return false;

  const auto& location = interface_->GetLocation();
  if (!location.IsPosition()) return false;

  const cse498::WorldPosition& pos = location.AsWorldPosition();
  out_cell = {static_cast<int>(pos.CellX()), static_cast<int>(pos.CellY())};
  return true;
}

std::pair<int, int> WebApp::GetAttemptedMoveCell(
    int action_code, const std::pair<int, int>& from_cell) const {
  switch (action_code) {
    case kActionUp:
      return {from_cell.first, from_cell.second - 1};
    case kActionDown:
      return {from_cell.first, from_cell.second + 1};
    case kActionLeft:
      return {from_cell.first - 1, from_cell.second};
    case kActionRight:
      return {from_cell.first + 1, from_cell.second};
    default:
      return from_cell;
  }
}

// Clears the canvas, draws all world cells and entities via the interface,
// then updates the HUD textbox and action button enabled states.
void WebApp::RenderWorld() {
  if (!canvas_ || !interface_) return;

  // Grid dimensions come from the world so this works with any WorldBase world.
  const int grid_w = interface_->GetGridWidth();
  const int grid_h = interface_->GetGridHeight();
  const int cell_w = std::min(kCanvasWidthPx  / grid_w,
                              kCanvasHeightPx / grid_h);
  const int cell_h = cell_w;

  // Fetch all renderable cells once and build a position-indexed lookup so
  // each grid cell is resolved in O(1) rather than with a linear scan.
  const auto renderable_cells = interface_->GetRenderableCells();
  std::unordered_map<int, int> cell_pos_map;  // (y * grid_w + x) → index
  cell_pos_map.reserve(renderable_cells.size());
  for (int i = 0; i < static_cast<int>(renderable_cells.size()); ++i) {
    cell_pos_map[renderable_cells[i].y * grid_w + renderable_cells[i].x] = i;
  }

  canvas_->Clear();

  canvas_->DrawGrid(grid_w, grid_h, cell_w, cell_h,
    [&](int col, int row) {
      const auto it = cell_pos_map.find(row * grid_w + col);
      if (it != cell_pos_map.end()) {
        const auto& cell = renderable_cells[it->second];

        const auto legend_it = legend_by_id_.find(cell.legend_id);
        const cse498::LegendEntry* entry =
            (legend_it != legend_by_id_.end()) ? &legend_it->second : nullptr;

        const std::string fill  = entry ? entry->fill_css : kColorCellFallback;
        const std::string glyph = entry ? entry->glyph   : "?";

        // Draw background tile first if one is registered for this cell type.
        if (entry) {
          const auto bg_it = cell_backgrounds_.find(entry->key);
          if (bg_it != cell_backgrounds_.end()) {
            canvas_->DrawImage(bg_it->second,
                               static_cast<float>(col * cell_w),
                               static_cast<float>(row * cell_h),
                               static_cast<float>(cell_w),
                               static_cast<float>(cell_h),
                               fill);
          }
        }

        std::string image_url;
        if (entry) { image_url = "assets/" + entry->key + ".png"; }
        canvas_->DrawCellImage(col, row, cell_w, cell_h,
                               image_url, fill, glyph);

        if (cell.selected) {
          canvas_->HighlightCell(col, row, cell_w, cell_h);
        }
      } else {
        canvas_->DrawCell(col, row, cell_w, cell_h,
                          kColorCellEmpty, "", kColorCellText);
      }
  });

  const float entity_radius = 0.4f * static_cast<float>(std::min(cell_w, cell_h));
  for (const auto& entity : interface_->GetEntities()) {
    const float center_x =
        static_cast<float>(entity.x * cell_w + cell_w / 2);
    const float center_y =
        static_cast<float>(entity.y * cell_h + cell_h / 2);
    if (!entity.image_url.empty()) {
      // Draw sprite scaled to the full cell, aligned to the cell top-left.
      const float img_x = static_cast<float>(entity.x * cell_w);
      const float img_y = static_cast<float>(entity.y * cell_h);
      canvas_->DrawImage(entity.image_url, img_x, img_y,
                         static_cast<float>(cell_w),
                         static_cast<float>(cell_h),
                         entity.fill_css);
    } else {
      canvas_->DrawEntity(center_x, center_y, entity_radius, entity.fill_css,
                          entity.glyph, kColorEntityText);
    }
  }

  // Draw grid lines after cells so they appear on top.
  canvas_->SetStrokeColor(kColorGridLine);
  canvas_->SetLineWidth(1.0f);
  const float grid_px_w = static_cast<float>(grid_w * cell_w);
  const float grid_px_h = static_cast<float>(grid_h * cell_h);
  for (int i = 0; i <= grid_w; ++i) {
    const float p = static_cast<float>(i * cell_w);
    canvas_->DrawLine(p, 0.0f, p, grid_px_h);
  }
  for (int i = 0; i <= grid_h; ++i) {
    const float p = static_cast<float>(i * cell_h);
    canvas_->DrawLine(0.0f, p, grid_px_w, p);
  }

  canvas_->Present();

  const cse498::HudState hud = interface_->GetHudState();
  const std::string& panel_text = interface_->GetPanelText();
  std::ostringstream hud_text;
  hud_text << "World: "    << hud.world_name    << "\n"
           << "Mode: "     << hud.mode          << "\n"
           << "Tick: "     << hud.tick          << "\n"
           << "Selected: " << hud.selected_cell << "\n\n"
           << "Resources\n";
  if (!panel_text.empty()) {
    hud_text << panel_text;
    if (panel_text.back() != '\n') {
      hud_text << "\n";
    }
  } else {
    for (const auto& entry : hud.resources) {
      hud_text << "- " << entry.first << ": " << entry.second << "\n";
    }
  }

  hud_text_->SetText(hud_text.str());
  log_text_->SetText(hud.status_message);

  std::ostringstream mode_tick;
  mode_tick << "Mode: " << hud.mode << " | Tick: " << hud.tick;
  WebAppSetModeTick(mode_tick.str().c_str());

  for (const auto& action : interface_->GetAvailableActions()) {
    const int code = CodeForActionId(action.id);
    if (code != 0) {
      WebAppSetActionEnabled(code, static_cast<int>(action.enabled));
    }
  }
}

// ---------------------------------------------------------------------------
// Global app instance and Emscripten C export
// ---------------------------------------------------------------------------

std::unique_ptr<WebApp> g_app;

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void HandleAction(int action_code) {
    if (g_app) {
      g_app->HandleAction(action_code);
    }
  }
}
