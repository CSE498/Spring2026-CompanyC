#include "Worlds/StubWorld.hpp"

#include <string>

#include "core/AgentBase.hpp"
#include "core/WorldGrid.hpp"
#include "core/WorldPosition.hpp"
#include "tools/WebInterface.hpp"  // needed for dynamic_cast in COLLECT and BUILD

namespace cse498 {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

StubWorld::StubWorld() {
  // Cell type ID 0 ("Unknown") is reserved by WorldGrid's constructor.
  // IDs 1-6 are assigned in declaration order and stored for fast lookups.
  grass_id_ = main_grid.AddCellType("grass", "Grass", '.');
  tree_id_  = main_grid.AddCellType("tree",  "Tree",  'T');
  stone_id_ = main_grid.AddCellType("stone", "Stone", 'S');
  wheat_id_ = main_grid.AddCellType("wheat", "Wheat", 'W');
  wall_id_  = main_grid.AddCellType("wall",  "Wall",  '#');
  built_id_ = main_grid.AddCellType("built", "Built", 'B');

  InitializeGrid_();

  resources_["wood"]  = 1;
  resources_["stone"] = 0;
  resources_["wheat"] = 0;
}

// ---------------------------------------------------------------------------
// ConfigAgent — called by WorldBase::AddAgent before Initialize()
// ---------------------------------------------------------------------------

void StubWorld::ConfigAgent(AgentBase& agent) {
  agent.AddAction("start",   START);
  agent.AddAction("reset",   RESET);
  agent.AddAction("save",    SAVE);
  agent.AddAction("load",    LOAD);
  agent.AddAction("up",      MOVE_UP);
  agent.AddAction("down",    MOVE_DOWN);
  agent.AddAction("left",    MOVE_LEFT);
  agent.AddAction("right",   MOVE_RIGHT);
  agent.AddAction("collect", COLLECT);
  agent.AddAction("build",   BUILD);

  // Bootstrap the interface with world identity and initial state.
  agent.Notify("Stub World", "world_name");
  SendResources_(agent);
  agent.Notify("Press Start to begin.", "status");
}

// ---------------------------------------------------------------------------
// DoAction
// ---------------------------------------------------------------------------

int StubWorld::DoAction(AgentBase& agent, size_t action_id) {
  // --- Meta-actions: always available regardless of started_ ---
  switch (static_cast<ActionType>(action_id)) {

  case START: {
    if (started_) {
      agent.Notify("Already running.", "status");
      return 0;
    }
    started_ = true;
    agent.Notify("live", "mode_change");
    agent.Notify("Simulation started.", "status");
    return 1;
  }

  case RESET: {
    started_ = false;
    resources_["wood"]  = 1;
    resources_["stone"] = 0;
    resources_["wheat"] = 0;
    InitializeGrid_();
    agent.SetLocation(WorldPosition{1, 1});
    // Reset the selection cursor to match the player's new position.
    if (auto* wi = dynamic_cast<WebInterface*>(&agent)) {
      wi->SelectCell(1, 1);
    }
    agent.Notify("setup",       "mode_change");
    agent.Notify("tick_reset",  "tick_reset");
    SendResources_(agent);
    agent.Notify("World reset. Press Start to begin.", "status");
    return 1;
  }

  case SAVE: {
    agent.Notify("Save not yet implemented.", "status");
    return 0;
  }

  case LOAD: {
    agent.Notify("Load not yet implemented.", "status");
    return 0;
  }

  case REMAIN_STILL:
    return 1;

  default:
    break;
  }

  // --- Game actions: require the simulation to be running ---
  if (!started_) {
    agent.Notify("Press Start before taking actions.", "status");
    return 0;
  }

  WorldPosition cur_pos = agent.GetLocation().AsWorldPosition();

  switch (static_cast<ActionType>(action_id)) {

  case MOVE_UP:
  case MOVE_DOWN:
  case MOVE_LEFT:
  case MOVE_RIGHT: {
    WorldPosition   next;
    std::string     dir_label;
    switch (static_cast<ActionType>(action_id)) {
      case MOVE_UP:    next = cur_pos.Up();    dir_label = "up";    break;
      case MOVE_DOWN:  next = cur_pos.Down();  dir_label = "down";  break;
      case MOVE_LEFT:  next = cur_pos.Left();  dir_label = "left";  break;
      case MOVE_RIGHT: next = cur_pos.Right(); dir_label = "right"; break;
      default:         next = cur_pos;                              break;
    }

    if (!IsWalkable_(next)) {
      agent.Notify("Blocked movement.", "status");
      return 0;
    }

    agent.SetLocation(next);
    // Keep the selection cursor in sync with the player position.
    if (auto* wi = dynamic_cast<WebInterface*>(&agent)) {
      wi->SelectCell(static_cast<int>(next.CellX()),
                     static_cast<int>(next.CellY()));
    }
    agent.Notify("Moved " + dir_label + ".", "status");
    agent.Notify("", "tick");
    return 1;
  }

  case COLLECT: {
    const int px = static_cast<int>(cur_pos.CellX());
    const int py = static_cast<int>(cur_pos.CellY());

    // Try the player's current cell first, then the selected cell.
    bool collected = TryCollectAt_(agent, px, py);
    if (!collected) {
      if (auto* wi = dynamic_cast<WebInterface*>(&agent)) {
        auto [sx, sy] = wi->GetSelectedCell();
        collected = TryCollectAt_(agent, sx, sy);
      }
    }

    if (!collected) {
      agent.Notify("Nothing collectible here.", "status");
      return 0;
    }

    SendResources_(agent);
    agent.Notify("", "tick");
    return 1;
  }

  case BUILD: {
    if (resources_["wood"] <= 0) {
      agent.Notify("Need at least 1 wood to build.", "status");
      return 0;
    }

    auto* wi = dynamic_cast<WebInterface*>(&agent);
    if (!wi) {
      agent.Notify("Build requires a WebInterface agent.", "status");
      return 0;
    }

    auto [sx, sy] = wi->GetSelectedCell();

    if (sx == static_cast<int>(cur_pos.CellX()) &&
        sy == static_cast<int>(cur_pos.CellY())) {
      agent.Notify("Cannot build on top of the player.", "status");
      return 0;
    }

    if (!main_grid.IsValid(static_cast<double>(sx),
                           static_cast<double>(sy))) {
      agent.Notify("Select a valid tile first.", "status");
      return 0;
    }

    const size_t target_id =
        main_grid[static_cast<size_t>(sx), static_cast<size_t>(sy)];
    if (target_id != grass_id_) {
      agent.Notify("Can only build on grass.", "status");
      return 0;
    }

    main_grid[static_cast<size_t>(sx), static_cast<size_t>(sy)] = built_id_;
    --resources_["wood"];
    SendResources_(agent);
    agent.Notify("Built a structure on the selected tile.", "status");
    agent.Notify("", "tick");
    return 1;
  }

  default:
    agent.Notify("Unknown action.", "status");
    return 0;
  }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void StubWorld::InitializeGrid_() {
  // Load the grid from a character map.  Symbols must match the CellType
  // symbols assigned in the constructor (grass='.', tree='T', etc.).
  main_grid.Load(std::vector<std::string>{
    "##########",   // row 0
    "#........#",   // row 1
    "#..T.....#",   // row 2: tree at (3, 2)
    "#.....T..#",   // row 3: tree at (6, 3)
    "#........#",   // row 4
    "#...S....#",   // row 5: stone at (4, 5)
    "#......W.#",   // row 6: wheat at (7, 6)
    "#.W......#",   // row 7: wheat at (2, 7)
    "#........#",   // row 8
    "##########",   // row 9
  });
}

bool StubWorld::IsWalkable_(WorldPosition pos) const {
  if (!main_grid.IsValid(pos)) return false;
  const size_t cell = main_grid[pos];
  return cell != wall_id_  && cell != tree_id_ &&
         cell != stone_id_ && cell != built_id_;
}

bool StubWorld::TryCollectAt_(AgentBase& agent, int x, int y) {
  if (!main_grid.IsValid(static_cast<double>(x), static_cast<double>(y))) {
    return false;
  }

  size_t& cell =
      main_grid[static_cast<size_t>(x), static_cast<size_t>(y)];

  if (cell == tree_id_) {
    cell = grass_id_;
    ++resources_["wood"];
    agent.Notify("Collected wood.", "status");
    return true;
  }
  if (cell == stone_id_) {
    cell = grass_id_;
    ++resources_["stone"];
    agent.Notify("Collected stone.", "status");
    return true;
  }
  if (cell == wheat_id_) {
    cell = grass_id_;
    ++resources_["wheat"];
    agent.Notify("Collected wheat.", "status");
    return true;
  }
  return false;
}

void StubWorld::SendResources_(AgentBase& agent) const {
  for (const auto& [key, val] : resources_) {
    agent.Notify(key + ":" + std::to_string(val), "resource");
  }
}

}  // namespace cse498
