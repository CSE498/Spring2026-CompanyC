#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "tools/IWorldUiAdapter.hpp"
#include "tools/StubWorldAdapter.hpp"
#include "tools/WebCanvas.hpp"
#include "tools/WebTextbox.hpp"

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
  if (document.getElementById('group24_root')) return;

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
  ].join('');
  document.head.appendChild(style);

  var root = document.createElement('div');
  root.id = 'group24_root';
  root.innerHTML = [
    '<div id="group24_topbar">',
      '<span id="group24_title">Group 24 Stub Demo</span>',
      '<label for="group24_world_select">World:</label>',
      '<select id="group24_world_select"><option value="stub">Stub World</option></select>',
      '<button id="g24btn-1">Start</button>',
      '<button id="g24btn-2">Reset</button>',
      '<button id="g24btn-3">Save</button>',
      '<button id="g24btn-4">Load</button>',
      '<span id="group24_mode_tick"></span>',
    '</div>',
    '<div id="group24_main">',
      '<div id="group24_canvas_host"></div>',
      '<div id="group24_sidebar">',
        '<div><strong>Available Actions</strong></div>',
        '<div id="group24_actions">',
          '<button id="g24btn-5">Up</button>',
          '<button id="g24btn-6">Down</button>',
          '<button id="g24btn-7">Left</button>',
          '<button id="g24btn-8">Right</button>',
          '<button id="g24btn-9">Collect</button>',
          '<button id="g24btn-10">Build</button>',
        '</div>',
        '<div id="group24_hud_host"></div>',
        '<div id="group24_log_host"></div>',
      '</div>',
    '</div>'
  ].join('');

  document.body.innerHTML = '';
  document.body.appendChild(root);

  for (var code = 1; code <= 10; ++code) {
    (function(c) {
      var btn = document.getElementById('g24btn-' + c);
      if (!btn) return;
      btn.addEventListener('click', function() {
        Module.ccall('Group24HandleAction', null, ['number'], [c]);
      });
    })(code);
  }

  document.addEventListener('keydown', function(ev) {
    var key = ev.key.toLowerCase();
    var map = { 'w': 5, 'arrowup': 5, 's': 6, 'arrowdown': 6,
                'a': 7, 'arrowleft': 7, 'd': 8, 'arrowright': 8,
                'e': 9, 'b': 10, 'r': 2, 'enter': 1 };
    if (!(key in map)) return;
    ev.preventDefault();
    Module.ccall('Group24HandleAction', null, ['number'], [map[key]]);
  });
});

EM_JS(void, Group24SetModeTick, (const char* text_ptr), {
  var el = document.getElementById('group24_mode_tick');
  if (!el) return;
  el.textContent = text_ptr ? UTF8ToString(text_ptr) : '';
});

EM_JS(void, Group24SetActionEnabled, (int code, int enabled), {
  var btn = document.getElementById('g24btn-' + code);
  if (!btn) return;
  btn.disabled = !enabled;
});
#else
void Group24EnsureUi() {}
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

  for (const auto& cell : g_world->GetRenderableCells()) {
    const auto it = legend_by_id.find(cell.legend_id);
    const cse498::LegendEntry* entry = (it != legend_by_id.end()) ? &it->second : nullptr;
    const std::string fill = entry ? entry->fill_css : "#e5e7eb";
    const std::string glyph = entry ? entry->glyph : "?";
    g_canvas->DrawCell(cell.x, cell.y, kCellPx, kCellPx, fill, glyph, "#111827");
    if (cell.selected) {
      g_canvas->SetStrokeColor("#ef4444");
      g_canvas->SetLineWidth(3.0f);
      g_canvas->StrokeRect(static_cast<float>(cell.x * kCellPx + 1),
                           static_cast<float>(cell.y * kCellPx + 1),
                           static_cast<float>(kCellPx - 2),
                           static_cast<float>(kCellPx - 2));
    }
  }

  for (const auto& entity : g_world->GetEntities()) {
    const float center_x = static_cast<float>(entity.x * kCellPx + kCellPx / 2);
    const float center_y = static_cast<float>(entity.y * kCellPx + kCellPx / 2);
    g_canvas->DrawEntity(center_x, center_y, 14.0f, entity.fill_css,
                         entity.glyph, "#ffffff");
  }

  for (int i = 0; i <= kGridSize; ++i) {
    g_canvas->SetStrokeColor("#d1d5db");
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
