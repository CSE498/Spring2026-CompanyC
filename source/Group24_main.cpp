// Group24_main.cpp
//
// Entry point and game loop for the Group 24 demo. Wires together WebCanvas
// (rendering), WebLayout (shell/controls), WebTextbox (HUD/log sidebar), and
// StubWorldAdapter (game logic). The exported C function Group24HandleAction
// is called by both JavaScript button/keyboard events and the C++ main().
//This file uses ChatGPT to help produce functionality, it is then reviewed.

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "tools/IWorldUiAdapter.hpp"
#include "tools/StubWorldAdapter.hpp"
#include "tools/WebCanvas.hpp"
#include "tools/WebTextbox.hpp"
#include "tools/WebLayout.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

constexpr int kCanvasWidthPx = 480;
constexpr int kCanvasHeightPx = 480;
constexpr int kGridSize = 10;
constexpr int kCellPx = kCanvasWidthPx / kGridSize;

// DOM host ids and shared font stack (match #group24_root typography).
constexpr const char* kHudHostElementId = "group24_hud_host";
constexpr const char* kLogHostElementId = "group24_log_host";
constexpr const char* kSidebarFontStack = "system-ui, Arial, sans-serif";

// Cell and grid rendering colors.
constexpr const char* kColorCellFallback  = "#e5e7eb";  // fill when no legend entry matched
constexpr const char* kColorCellText      = "#111827";  // glyph text on cells
constexpr const char* kColorCellEmpty     = "#f9fafb";  // background for blank cells
constexpr const char* kColorGridLine      = "#d1d5db";  // grid stroke
constexpr const char* kColorEntityText    = "#ffffff";  // glyph text on entities

std::unique_ptr<cse498::WebLayout> g_layout;
std::unique_ptr<cse498::WebCanvas> g_canvas;
std::unique_ptr<cse498::IWorldUiAdapter> g_world;
cse498::WebTextbox g_hud_text;
cse498::WebTextbox g_log_text;

enum ActionCode {
  kActionStart = 1,
  kActionReset = 2,
  kActionSave = 3,
  kActionLoad = 4,
  kActionUp = 5,
  kActionDown = 6,
  kActionLeft = 7,
  kActionRight = 8,
  kActionCollect = 9,
  kActionBuild = 10,
};

#ifdef __EMSCRIPTEN__
EM_JS(void, Group24EnsureUi, (), {
  if (!window.__group24_style_loaded) {
    var style = document.createElement('style');
    style.textContent = [
      '#group24_root{font-family:system-ui,Arial,sans-serif;padding:12px;color:#111827;}',
      '#group24_topbar{display:flex;gap:8px;align-items:center;margin-bottom:12px;flex-wrap:wrap;}',
      '#group24_main{display:flex;gap:12px;align-items:flex-start;}',
      '#group24_canvas_host{min-width:500px;}',
      '#group24_sidebar{width:320px;background:#f3f4f6;border:1px solid #d1d5db;border-radius:10px;padding:12px;}',
      '#group24_actions{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px;margin-bottom:12px;}',
      '#group24_actions button,#group24_topbar button,#group24_topbar select{padding:8px 10px;border:1px solid #9ca3af;border-radius:8px;background:white;}',
      '#group24_hud_host,#group24_log_host{white-space:pre-wrap;background:white;border:1px solid #d1d5db;border-radius:8px;padding:10px;margin-top:10px;overflow:visible;min-height:3em;}',
      '#group24_title{font-size:20px;font-weight:700;margin-right:8px;}',
      '#group24_canvas{border:1px solid #374151;border-radius:8px;background:#ffffff;}'
    ].join("");
    document.head.appendChild(style);
    window.__group24_style_loaded = true;
  }

  if (!window.__group24_keybound) {
    document.addEventListener('keydown', function(ev) {
      var key = ev.key.toLowerCase();
      var map = { 'w': 5, 'arrowup': 5, 's': 6, 'arrowdown': 6,
                  'a': 7, 'arrowleft': 7, 'd': 8, 'arrowright': 8,
                  'e': 9, 'b': 10, 'r': 2, 'enter': 1 };
      if (!(key in map)) return;
      ev.preventDefault();
      Module.ccall('Group24HandleAction', null, ['number'], [map[key]]);
    });
    window.__group24_keybound = true;
  }
});

EM_JS(void, Group24PopulateUiControls, (), {
  var actions = document.getElementById('group24_actions');
  var sidebar = document.getElementById('group24_sidebar');

  if (!actions || !sidebar) return;

  // Hook topbar buttons created by WebLayout.
  [1, 2, 3, 4].forEach(function(code) {
    var btn = document.getElementById('g24btn-' + code);
    if (!btn || btn.__group24_bound) return;
    btn.addEventListener('click', function() {
      Module.ccall('Group24HandleAction', null, ['number'], [code]);
    });
    btn.__group24_bound = true;
  });

  // Add sidebar label if missing.
  if (!document.getElementById('group24_actions_label')) {
    var labelWrap = document.createElement('div');
    labelWrap.id = 'group24_actions_label';
    labelWrap.innerHTML = '<strong>Available Actions</strong>';
    sidebar.insertBefore(labelWrap, actions);
  }

  // Fill action buttons only if missing.
  if (!document.getElementById('g24btn-5')) {
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
      btn.id = 'g24btn-' + code;
      btn.textContent = text;
      btn.addEventListener('click', function() {
        Module.ccall('Group24HandleAction', null, ['number'], [code]);
      });
      actions.appendChild(btn);
    });
  }
});

EM_JS(void, Group24SetModeTick, (const char* text_ptr), {
  var el = document.getElementById('group24_mode_tick');
  if (!el) return;
  el.textContent = text_ptr ? UTF8ToString(text_ptr) : "";
});

EM_JS(void, Group24SetActionEnabled, (int code, int enabled), {
  var btn = document.getElementById('g24btn-' + code);
  if (!btn) return;
  btn.disabled = !enabled;
});
#else
void Group24EnsureUi() {}
void Group24PopulateUiControls() {}
void Group24SetModeTick(const char*) {}
void Group24SetActionEnabled(int, int) {}
#endif

std::string ActionIdForCode(const int code) {
  switch (code) {
    case kActionStart: return "start";
    case kActionReset: return "reset";
    case kActionSave: return "save";
    case kActionLoad: return "load";
    case kActionUp: return "up";
    case kActionDown: return "down";
    case kActionLeft: return "left";
    case kActionRight: return "right";
    case kActionCollect: return "collect";
    case kActionBuild: return "build";
    default: return std::string();
  }
}

int CodeForActionId(const std::string& action_id) {
  if (action_id == "start") return kActionStart;
  if (action_id == "reset") return kActionReset;
  if (action_id == "save") return kActionSave;
  if (action_id == "load") return kActionLoad;
  if (action_id == "up") return kActionUp;
  if (action_id == "down") return kActionDown;
  if (action_id == "left") return kActionLeft;
  if (action_id == "right") return kActionRight;
  if (action_id == "collect") return kActionCollect;
  if (action_id == "build") return kActionBuild;
  return 0;
}

// Clears the canvas, draws all world cells and entities via the adapter, then
// updates the HUD textbox and action button enabled states.
void RenderWorld() {
  if (!g_canvas || !g_world) {
    return;
  }

  const std::vector<cse498::LegendEntry> legend = g_world->GetLegend();
  std::map<int, cse498::LegendEntry> legend_by_id;
  for (const auto& entry : legend) {
    legend_by_id[entry.id] = entry;
  }

  g_canvas->Clear();

  g_canvas->DrawGrid(kGridSize, kGridSize, kCellPx, kCellPx,
    [&](int col, int row) {

      // Find matching world cell
      for (const auto& cell : g_world->GetRenderableCells()) {
        if (cell.x == col && cell.y == row) {

          const auto it = legend_by_id.find(cell.legend_id);
          const cse498::LegendEntry* entry =
              (it != legend_by_id.end()) ? &it->second : nullptr;

          const std::string fill = entry ? entry->fill_css : kColorCellFallback;
          const std::string glyph = entry ? entry->glyph : "?";

          g_canvas->DrawCell(col, row, kCellPx, kCellPx,
                             fill, glyph, kColorCellText);

          if (cell.selected) {
            g_canvas->HighlightCell(col, row, kCellPx, kCellPx);
          }

          return;
        }
      }

      // Default empty cell
      g_canvas->DrawCell(col, row, kCellPx, kCellPx,
                         kColorCellEmpty, "", kColorCellText);
  });

  for (const auto& entity : g_world->GetEntities()) {
    const float center_x = static_cast<float>(entity.x * kCellPx + kCellPx / 2);
    const float center_y = static_cast<float>(entity.y * kCellPx + kCellPx / 2);
    g_canvas->DrawEntity(center_x, center_y, 14.0f, entity.fill_css,
                         entity.glyph, kColorEntityText);
  }

  for (int i = 0; i <= kGridSize; ++i) {
    g_canvas->SetStrokeColor(kColorGridLine);
    g_canvas->SetLineWidth(1.0f);
    const float p = static_cast<float>(i * kCellPx);
    g_canvas->DrawLine(p, 0.0f, p, static_cast<float>(kCanvasHeightPx));
    g_canvas->DrawLine(0.0f, p, static_cast<float>(kCanvasWidthPx), p);
  }

  g_canvas->Present();

  const cse498::HudState hud = g_world->GetHudState();
  std::ostringstream hud_text;
  hud_text << "World: " << hud.world_name << "\n"
           << "Mode: " << hud.mode << "\n"
           << "Tick: " << hud.tick << "\n"
           << "Selected: " << hud.selected_cell << "\n\n"
           << "Resources\n";
  for (const auto& entry : hud.resources) {
    hud_text << "- " << entry.first << ": " << entry.second << "\n";
  }

  g_hud_text.SetText(hud_text.str());
  g_log_text.SetText(hud.status_message);

  std::ostringstream mode_tick;
  mode_tick << "Mode: " << hud.mode << " | Tick: " << hud.tick;
  Group24SetModeTick(mode_tick.str().c_str());

  for (const auto& action : g_world->GetAvailableActions()) {
    const int code = CodeForActionId(action.id);
    if (code != 0) {
      Group24SetActionEnabled(code, action.enabled ? 1 : 0);
    }
  }
}

void InitializeDemo() {
  Group24EnsureUi();

  // Topbar controls now created in C++ instead of JS
  g_layout = std::make_unique<cse498::WebLayout>();
  g_layout->createGroup24Shell();
  Group24PopulateUiControls();

  g_world.reset(new cse498::StubWorldAdapter());
  g_canvas.reset(new cse498::WebCanvas("group24_canvas", kCanvasWidthPx,
                                       kCanvasHeightPx));
  g_canvas->AppendTo("group24_canvas_host");
  g_canvas->SetClickHandler([](int pixel_x, int pixel_y) {
    const std::pair<int, int> cell = g_canvas->PixelToCell(pixel_x, pixel_y,
                                                           kCellPx, kCellPx);
    g_world->SelectCell(cell.first, cell.second);
    RenderWorld();
  });

  g_hud_text.SetParentId(kHudHostElementId);
  g_log_text.SetParentId(kLogHostElementId);
  g_hud_text.SetWordWrap("break-word");
  g_log_text.SetWordWrap("break-word");
  g_hud_text.Create();
  g_log_text.Create();
  g_hud_text.ApplyFlowLayoutStyles();
  g_log_text.ApplyFlowLayoutStyles();
  g_hud_text.SetFontFamily(kSidebarFontStack);
  g_log_text.SetFontFamily(kSidebarFontStack);

  RenderWorld();
}

}  // namespace

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void Group24HandleAction(int action_code) {
  if (!g_world) {
    InitializeDemo();
  }
  const std::string action_id = ActionIdForCode(action_code);
  if (action_id.empty()) {
    return;
  }
  g_world->SubmitAction(action_id);
  RenderWorld();
}
}

int main() {
  InitializeDemo();
  return 0;
}