#include "tools/StubWorldAdapter.hpp"

#include <sstream>

namespace cse498 {

StubWorldAdapter::StubWorldAdapter() {
  Reset();
}

std::string StubWorldAdapter::GetWorldName() const {
  return "Stub World";
}

UiMode StubWorldAdapter::GetMode() const {
  return mode_;
}

int StubWorldAdapter::GetWidth() const {
  return kWidth;
}

int StubWorldAdapter::GetHeight() const {
  return kHeight;
}

std::vector<LegendEntry> StubWorldAdapter::GetLegend() const {
  return {
      {kGrass, "grass", "Grass", "#8fd17f", ".", false},
      {kTree, "tree", "Tree", "#3f8f3f", "T", true},
      {kStone, "stone", "Stone", "#9ca3af", "S", true},
      {kWheat, "wheat", "Wheat", "#f4d35e", "W", false},
      {kWall, "wall", "Wall", "#374151", "#", true},
      {kBuilt, "built", "Built", "#8b5cf6", "B", true},
  };
}

std::vector<RenderableCell> StubWorldAdapter::GetRenderableCells() const {
  std::vector<RenderableCell> cells;
  cells.reserve(static_cast<std::size_t>(kWidth * kHeight));

  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      RenderableCell cell;
      cell.x = x;
      cell.y = y;
      cell.legend_id = CellAt_(x, y);
      cell.visible = true;
      cell.selected = (x == selected_x_ && y == selected_y_);
      cells.push_back(cell);
    }
  }

  return cells;
}

std::vector<RenderableEntity> StubWorldAdapter::GetEntities() const {
  std::vector<RenderableEntity> entities;

  RenderableEntity player;
  player.id = 1;
  player.x = player_x_;
  player.y = player_y_;
  player.label = "Player";
  player.fill_css = mode_ == UiMode::Setup ? "#2563eb" : "#dc2626";
  player.glyph = "P";
  player.visible = true;
  entities.push_back(player);

  return entities;
}

std::vector<UiAction> StubWorldAdapter::GetAvailableActions() const {
  const bool running = (mode_ == UiMode::Live);
  return {
      {"start", "Start", "Enter", mode_ == UiMode::Setup},
      {"reset", "Reset", "R", true},
      {"save", "Save", "", true},
      {"load", "Load", "", true},
      {"up", "Up", "W", running},
      {"down", "Down", "S", running},
      {"left", "Left", "A", running},
      {"right", "Right", "D", running},
      {"collect", "Collect", "E", running},
      {"build", "Build", "B", running},
  };
}

void StubWorldAdapter::SubmitAction(const std::string& action_id) {
  if (action_id == "start") {
    if (mode_ == UiMode::Setup) {
      mode_ = UiMode::Live;
      SetStatus_("Simulation started.");
    } else {
      SetStatus_("Already running.");
    }
    return;
  }

  if (action_id == "reset") {
    Reset();
    return;
  }

  if (action_id == "save") {
    SaveWorld();
    return;
  }

  if (action_id == "load") {
    LoadWorld();
    return;
  }

  if (mode_ != UiMode::Live) {
    SetStatus_("Press Start before taking actions.");
    return;
  }

  int next_x = player_x_;
  int next_y = player_y_;

  if (action_id == "up") {
    --next_y;
  } else if (action_id == "down") {
    ++next_y;
  } else if (action_id == "left") {
    --next_x;
  } else if (action_id == "right") {
    ++next_x;
  } else if (action_id == "collect") {
    const bool collected_here = TryCollectAt_(player_x_, player_y_);
    const bool collected_selected = !collected_here && TryCollectAt_(selected_x_, selected_y_);
    if (collected_here || collected_selected) {
      Tick();
    } else {
      SetStatus_("Nothing collectible on the player or selected tile.");
    }
    return;
  } else if (action_id == "build") {
    if (resources_["wood"] <= 0) {
      SetStatus_("Need at least 1 wood to build.");
      return;
    }
    if (!InBounds_(selected_x_, selected_y_)) {
      SetStatus_("Select a valid tile first.");
      return;
    }
    if (selected_x_ == player_x_ && selected_y_ == player_y_) {
      SetStatus_("Cannot build on top of the player.");
      return;
    }
    if (CellAt_(selected_x_, selected_y_) != kGrass) {
      SetStatus_("Can only build on grass in the stub world.");
      return;
    }
    CellAt_(selected_x_, selected_y_) = kBuilt;
    --resources_["wood"];
    Tick();
    SetStatus_("Built a structure on the selected tile.");
    return;
  } else {
    SetStatus_("Unknown action: " + action_id);
    return;
  }

  if (!IsWalkable_(next_x, next_y)) {
    SetStatus_("Blocked movement.");
    return;
  }

  player_x_ = next_x;
  player_y_ = next_y;
  selected_x_ = player_x_;
  selected_y_ = player_y_;
  Tick();
  SetStatus_("Moved " + action_id + ".");
}

void StubWorldAdapter::Tick() {
  ++tick_;
}

HudState StubWorldAdapter::GetHudState() const {
  HudState hud;
  hud.world_name = GetWorldName();
  hud.mode = (mode_ == UiMode::Setup) ? "Setup" :
             (mode_ == UiMode::Live) ? "Live" : "Observe";
  hud.status_message = status_message_;
  hud.selected_cell = FormatSelectedCell_();
  hud.tick = tick_;
  hud.resources = resources_;
  return hud;
}

void StubWorldAdapter::SaveWorld() {
  SetStatus_("Save stub called. Database hook not connected yet.");
}

void StubWorldAdapter::LoadWorld() {
  SetStatus_("Load stub called. Database hook not connected yet.");
}

void StubWorldAdapter::Reset() {
  grid_.assign(static_cast<std::size_t>(kWidth * kHeight), kGrass);
  InitializeWorld_();
  player_x_ = 1;
  player_y_ = 1;
  selected_x_ = 1;
  selected_y_ = 1;
  mode_ = UiMode::Setup;
  tick_ = 0;
  resources_.clear();
  resources_["wood"] = 1;
  resources_["stone"] = 0;
  resources_["wheat"] = 0;
  SetStatus_("World reset. Press Start to enter live mode.");
}

void StubWorldAdapter::SelectCell(int x, int y) {
  if (!InBounds_(x, y)) {
    return;
  }
  selected_x_ = x;
  selected_y_ = y;
  SetStatus_("Selected tile (" + std::to_string(x) + ", " +
             std::to_string(y) + ").");
}

void StubWorldAdapter::InitializeWorld_() {
  for (int x = 0; x < kWidth; ++x) {
    CellAt_(x, 0) = kWall;
    CellAt_(x, kHeight - 1) = kWall;
  }
  for (int y = 0; y < kHeight; ++y) {
    CellAt_(0, y) = kWall;
    CellAt_(kWidth - 1, y) = kWall;
  }

  CellAt_(3, 2) = kTree;
  CellAt_(6, 3) = kTree;
  CellAt_(4, 5) = kStone;
  CellAt_(7, 6) = kWheat;
  CellAt_(2, 7) = kWheat;
}

bool StubWorldAdapter::InBounds_(int x, int y) const {
  return x >= 0 && x < kWidth && y >= 0 && y < kHeight;
}

bool StubWorldAdapter::IsWalkable_(int x, int y) const {
  if (!InBounds_(x, y)) {
    return false;
  }
  const int cell = CellAt_(x, y);
  return cell != kWall && cell != kTree && cell != kStone && cell != kBuilt;
}

int StubWorldAdapter::CellAt_(int x, int y) const {
  return grid_[static_cast<std::size_t>(y * kWidth + x)];
}

int& StubWorldAdapter::CellAt_(int x, int y) {
  return grid_[static_cast<std::size_t>(y * kWidth + x)];
}

bool StubWorldAdapter::TryCollectAt_(int x, int y) {
  if (!InBounds_(x, y)) {
    return false;
  }

  int& cell = CellAt_(x, y);
  if (cell == kTree) {
    cell = kGrass;
    ++resources_["wood"];
    SetStatus_("Collected wood.");
    return true;
  }
  if (cell == kStone) {
    cell = kGrass;
    ++resources_["stone"];
    SetStatus_("Collected stone.");
    return true;
  }
  if (cell == kWheat) {
    cell = kGrass;
    ++resources_["wheat"];
    SetStatus_("Collected wheat.");
    return true;
  }
  return false;
}

std::string StubWorldAdapter::FormatSelectedCell_() const {
  std::ostringstream out;
  out << "(" << selected_x_ << ", " << selected_y_ << ")";

  const int cell = CellAt_(selected_x_, selected_y_);
  switch (cell) {
    case kGrass:
      out << " Grass";
      break;
    case kTree:
      out << " Tree";
      break;
    case kStone:
      out << " Stone";
      break;
    case kWheat:
      out << " Wheat";
      break;
    case kWall:
      out << " Wall";
      break;
    case kBuilt:
      out << " Built";
      break;
    default:
      out << " Unknown";
      break;
  }
  return out.str();
}

void StubWorldAdapter::SetStatus_(const std::string& message) {
  status_message_ = message;
}

}  // namespace cse498
