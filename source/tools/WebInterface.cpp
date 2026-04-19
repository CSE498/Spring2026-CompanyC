#include "tools/WebInterface.hpp"

#include <string>

#include "core/AgentBase.hpp"
#include "core/WorldBase.hpp"
#include "core/WorldGrid.hpp"

namespace cse498 {

WebInterface::WebInterface(size_t id, const std::string& name,
                           const WorldBase& in_world)
    : InterfaceBase(id, name, in_world) {}

size_t WebInterface::SelectAction(WorldGrid& /*grid*/) {
  const size_t result = pending_action_;
  pending_action_ = 0;
  return result;
}

void WebInterface::Notify(const std::string& message,
                          const std::string& msg_type) {
  if (msg_type == "status") {
    status_message_ = message;
  } else if (msg_type == "mode_change") {
    if      (message == "live")  mode_ = UiMode::Live;
    else if (message == "setup") mode_ = UiMode::Setup;
  } else if (msg_type == "tick") {
    ++tick_;
  } else if (msg_type == "tick_reset") {
    tick_ = 0;
  } else if (msg_type == "resource") {
    // Expected format: "key:value"  (e.g., "wood:3")
    const auto colon = message.find(':');
    if (colon != std::string::npos) {
      resources_[message.substr(0, colon)] =
          std::stoi(message.substr(colon + 1));
    }
  } else if (msg_type == "world_name") {
    world_name_ = message;
  }
}

void WebInterface::SubmitAction(const std::string& action_id) {
  const size_t id = GetActionID(action_id);
  if (id == 0) {
    status_message_ = "Unknown action: " + action_id;
    return;
  }
  pending_action_ = id;
}

void WebInterface::SelectCell(int x, int y) {
  selected_x_ = x;
  selected_y_ = y;
}

std::pair<int, int> WebInterface::GetSelectedCell() const {
  return {selected_x_, selected_y_};
}

void WebInterface::SetCellVisual(const std::string& cell_type_name,
                                 std::string fill_css, std::string glyph) {
  cell_visuals_[cell_type_name] =
      CellVisual{std::move(fill_css), std::move(glyph)};
}

void WebInterface::RegisterActionMeta(const std::string& action_id,
                                      ActionMeta meta) {
  action_metas_[action_id] = std::move(meta);
}

std::vector<LegendEntry> WebInterface::GetLegend() const {
  const WorldGrid& grid   = world.GetGrid();
  const auto&      ctypes = grid.GetCellTypes();

  std::vector<LegendEntry> legend;
  legend.reserve(ctypes.size());

  for (size_t i = 0; i < ctypes.size(); ++i) {
    const CellType& ct  = ctypes[i];
    const auto      vis = cell_visuals_.find(ct.name);

    LegendEntry entry;
    entry.id    = static_cast<int>(i);
    entry.key   = ct.name;
    entry.label = ct.desc.empty() ? ct.name : ct.desc;

    if (vis != cell_visuals_.end()) {
      entry.fill_css = vis->second.fill_css;
      entry.glyph    = vis->second.glyph;
    } else {
      // Fallback: grey fill, cell-type symbol (or "?" if none assigned).
      entry.fill_css = "#e5e7eb";
      entry.glyph    = (ct.symbol != '\0') ? std::string(1, ct.symbol) : "?";
    }

    entry.blocks_movement = false;
    legend.push_back(std::move(entry));
  }
  return legend;
}

std::vector<RenderableCell> WebInterface::GetRenderableCells() const {
  const WorldGrid& grid = world.GetGrid();
  const int        w    = static_cast<int>(grid.GetWidth());
  const int        h    = static_cast<int>(grid.GetHeight());

  std::vector<RenderableCell> cells;
  cells.reserve(static_cast<size_t>(w * h));

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      RenderableCell cell;
      cell.x         = x;
      cell.y         = y;
      cell.legend_id = static_cast<int>(
          grid[static_cast<size_t>(x), static_cast<size_t>(y)]);
      cell.visible   = true;
      cell.selected  = (x == selected_x_ && y == selected_y_);
      cells.push_back(cell);
    }
  }
  return cells;
}

std::vector<RenderableEntity> WebInterface::GetEntities() const {
  const auto             agent_ids = world.GetKnownAgents(*this);
  std::vector<RenderableEntity> entities;
  entities.reserve(agent_ids.size());

  for (const size_t id : agent_ids) {
    const AgentBase& agent = world.GetAgent(id);
    if (!agent.GetLocation().IsPosition()) continue;

    const WorldPosition& pos = agent.GetLocation().AsWorldPosition();
    RenderableEntity ent;
    ent.id       = static_cast<int>(id);
    ent.x        = static_cast<int>(pos.CellX());
    ent.y        = static_cast<int>(pos.CellY());
    ent.label    = agent.GetName();
    ent.fill_css = agent.IsInterface() ? "#2563eb" : "#dc2626";
    ent.glyph    = std::string(1, agent.GetSymbol());
    ent.visible  = true;
    entities.push_back(std::move(ent));
  }
  return entities;
}

std::vector<UiAction> WebInterface::GetAvailableActions() const {
  const bool live = (mode_ == UiMode::Live);
  std::vector<UiAction> actions;
  actions.reserve(action_map.size());

  for (const auto& kv : action_map) {
    const std::string& name    = kv.first;
    const auto         meta_it = action_metas_.find(name);

    UiAction action;
    action.id     = name;
    action.label  = (meta_it != action_metas_.end()) ? meta_it->second.label  : name;
    action.hotkey = (meta_it != action_metas_.end()) ? meta_it->second.hotkey : "";

    const bool req_live =
        (meta_it != action_metas_.end()) ? meta_it->second.requires_live : true;
    action.enabled = !req_live || live;

    actions.push_back(std::move(action));
  }
  return actions;
}

HudState WebInterface::GetHudState() const {
  const WorldGrid& grid = world.GetGrid();

  // Format the selected-cell string with the cell type name if in range.
  std::string sel_str =
      "(" + std::to_string(selected_x_) + ", " +
      std::to_string(selected_y_) + ")";
  if (grid.IsValid(static_cast<double>(selected_x_),
                   static_cast<double>(selected_y_))) {
    const size_t type_id =
        grid[static_cast<size_t>(selected_x_),
             static_cast<size_t>(selected_y_)];
    sel_str += " " + grid.GetCellTypeName(type_id);
  }

  HudState hud;
  hud.world_name     = world_name_;
  hud.mode           = (mode_ == UiMode::Setup) ? "Setup" :
                       (mode_ == UiMode::Live)  ? "Live"  : "Observe";
  hud.status_message = status_message_;
  hud.selected_cell  = sel_str;
  hud.tick           = tick_;
  hud.resources      = resources_;
  hud.world_score    = world.GetWorldScoreDisplay();
  return hud;
}

int WebInterface::GetGridWidth() const {
  return static_cast<int>(world.GetGrid().GetWidth());
}

int WebInterface::GetGridHeight() const {
  return static_cast<int>(world.GetGrid().GetHeight());
}

}  // namespace cse498
