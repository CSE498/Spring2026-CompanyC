#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Agents/PacingAgent.hpp"
#include "Worlds/StubWorld.hpp"
#include "core/WorldPosition.hpp"
#include "tools/WebInterface.hpp"

namespace {

using cse498::WebInterface;
using cse498::WorldPosition;
using cse498::StubWorld;

void Fail(const std::string& message) {
  std::cerr << "FAIL: " << message << std::endl;
  std::exit(1);
}

void ExpectTrue(bool condition, const std::string& message) {
  if (!condition) {
    Fail(message);
  }
}

template <typename T, typename U>
void ExpectEq(const T& actual, const U& expected, const std::string& message) {
  if (!(actual == expected)) {
    std::cerr << "FAIL: " << message
              << "\n  expected: " << expected
              << "\n  actual:   " << actual << std::endl;
    std::exit(1);
  }
}

WebInterface& AddPlayer(StubWorld& world,
                        const std::string& name = "Player",
                        WorldPosition pos = {1, 1}) {
  auto& ui = world.AddAgent<WebInterface>(name);
  ui.SetSymbol('P');
  ui.SetLocation(pos);
  return ui;
}

void TestStatusModeTickAndWorldName() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.Notify("Welcome to the world", "status");
  ui.Notify("My Test World", "world_name");
  ui.Notify("live", "mode_change");
  ui.Notify("", "tick");
  ui.Notify("", "tick");
  ui.Notify("", "tick");

  const auto hud = ui.GetHudState();
  ExpectEq(hud.status_message, std::string("Welcome to the world"),
           "status message should be stored in HUD state");
  ExpectEq(hud.world_name, std::string("My Test World"),
           "world name should be stored in HUD state");
  ExpectEq(hud.mode, std::string("Live"),
           "mode_change=live should set HUD mode to Live");
  ExpectEq(hud.tick, 3, "three tick notifications should produce tick count 3");

  ui.Notify("", "tick_reset");
  ExpectEq(ui.GetHudState().tick, 0, "tick_reset should clear the tick counter");

  ui.Notify("setup", "mode_change");
  ExpectEq(ui.GetHudState().mode, std::string("Setup"),
           "mode_change=setup should set HUD mode to Setup");
}

void TestResourceNotifyAndResourceClear() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.Notify("wood:3", "resource");
  ui.Notify("stone:7", "resource");
  ui.Notify("wheat:11", "resource");

  auto hud = ui.GetHudState();
  ExpectEq(hud.resources.size(), static_cast<std::size_t>(3),
           "resource notifications should populate 3 entries");
  ExpectEq(hud.resources.at("wood"), 3,
           "wood resource count should be parsed correctly");
  ExpectEq(hud.resources.at("stone"), 7,
           "stone resource count should be parsed correctly");
  ExpectEq(hud.resources.at("wheat"), 11,
           "wheat resource count should be parsed correctly");

  ui.Notify("", "resource_clear");
  hud = ui.GetHudState();
  ExpectTrue(hud.resources.empty(),
             "resource_clear should empty the stored resource map");
}

void TestPanelClearPanelLineAndPanelText() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.Notify("", "panel_clear");
  ExpectEq(ui.GetPanelText(), std::string(),
           "panel_clear should start with an empty panel");

  ui.Notify("Wood: 3", "panel_line");
  ExpectEq(ui.GetPanelText(), std::string("Wood: 3"),
           "first panel_line should set the panel text");

  ui.Notify("Stone: 2", "panel_line");
  ExpectEq(ui.GetPanelText(), std::string("Wood: 3\nStone: 2"),
           "second panel_line should append with a newline");

  ui.Notify("Type: Wheat\nHarvestable: Yes", "panel_text");
  ExpectEq(ui.GetPanelText(), std::string("Type: Wheat\nHarvestable: Yes"),
           "panel_text should replace the whole panel at once");

  ui.Notify("", "panel_clear");
  ExpectEq(ui.GetPanelText(), std::string(),
           "panel_clear should clear a previously populated panel");
}

void TestPanelTextDoesNotDestroyResources() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.Notify("wood:5", "resource");
  ui.Notify("stone:9", "resource");
  ui.Notify("Wood: 5\nStone: 9", "panel_text");

  const auto hud = ui.GetHudState();

  ExpectEq(hud.resources.at("wood"), 5,
            "wood resource value should survive panel_text updates");
  ExpectEq(hud.resources.at("stone"), 9,
            "stone resource value should survive panel_text updates");
  ExpectTrue(hud.resources.size() >= 2,
            "resource map should still contain at least the resources added in this test");
  ExpectEq(ui.GetPanelText(), std::string("Wood: 5\nStone: 9"),
           "panel_text should remain available separately from resources");
}

void TestUnknownActionSetsStatusAndDoesNotQueueAction() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.SubmitAction("definitely_not_a_real_action");

  const auto hud = ui.GetHudState();
  ExpectEq(hud.status_message,
           std::string("Unknown action: definitely_not_a_real_action"),
           "unknown action should update status message");

  const std::size_t selected = ui.SelectAction(world.GetGrid());
  ExpectEq(selected, static_cast<std::size_t>(0),
           "unknown action should not queue a pending action");
}

void TestSelectedCellRoundTripAndRenderableSelection() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ui.SelectCell(2, 1);

  const auto selected = ui.GetSelectedCell();
  ExpectEq(selected.first, 2, "selected x should round-trip");
  ExpectEq(selected.second, 1, "selected y should round-trip");

  const auto cells = ui.GetRenderableCells();
  const int expected_count = ui.GetGridWidth() * ui.GetGridHeight();
  ExpectEq(static_cast<int>(cells.size()), expected_count,
           "renderable cell count should match grid area");

  int selected_count = 0;
  for (const auto& cell : cells) {
    if (cell.selected) {
      ++selected_count;
      ExpectEq(cell.x, 2, "selected renderable cell x should match selected_x_");
      ExpectEq(cell.y, 1, "selected renderable cell y should match selected_y_");
    }
  }

  ExpectEq(selected_count, 1,
           "exactly one renderable cell should be marked selected");

  const auto hud = ui.GetHudState();
  ExpectTrue(hud.selected_cell.find("(2, 1)") != std::string::npos,
             "HUD selected cell string should include selected coordinates");
}

void TestLegendUsesCustomVisualAndFallbacks() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  const auto& cell_types = world.GetGrid().GetCellTypes();
  ExpectTrue(!cell_types.empty(),
             "StubWorld should expose at least one cell type for legend testing");

  ui.SetCellVisual(cell_types[0].name, "#123456", "X");

  const auto legend = ui.GetLegend();
  ExpectEq(legend.size(), cell_types.size(),
           "legend entry count should match cell type count");

  ExpectEq(legend[0].id, 0, "first legend entry should have id 0");
  ExpectEq(legend[0].key, cell_types[0].name,
           "first legend key should match first cell type name");
  ExpectEq(legend[0].fill_css, std::string("#123456"),
           "custom cell visual fill should be used in the legend");
  ExpectEq(legend[0].glyph, std::string("X"),
           "custom cell visual glyph should be used in the legend");

  if (cell_types.size() > 1) {
    const std::string fallback_glyph =
        (cell_types[1].symbol != '\0') ? std::string(1, cell_types[1].symbol) : "?";

    ExpectEq(legend[1].key, cell_types[1].name,
             "fallback legend key should still match cell type name");
    ExpectEq(legend[1].glyph, fallback_glyph,
             "fallback legend glyph should come from the cell symbol or '?'");
    ExpectEq(legend[1].blocks_movement, false,
             "legend currently reports blocks_movement as false");
  }
}

void TestEntityVisualIsAppliedToPlayerEntity() {
  StubWorld world;
  auto& ui = AddPlayer(world, "Player", WorldPosition{1, 1});
  ui.SetEntityVisual('P', "assets/player.png");

  const auto entities = ui.GetEntities();
  ExpectTrue(!entities.empty(), "player interface should appear in renderable entities");

  bool found_player = false;
  for (const auto& entity : entities) {
    if (entity.label == "Player") {
      found_player = true;
      ExpectEq(entity.x, 1, "player entity x should match player location");
      ExpectEq(entity.y, 1, "player entity y should match player location");
      ExpectEq(entity.glyph, std::string("P"),
               "player glyph should reflect player symbol");
      ExpectEq(entity.image_url, std::string("assets/player.png"),
               "entity visual mapping should set the image_url");
      ExpectEq(entity.fill_css, std::string("#2563eb"),
               "interface entities should render with interface fill color");
    }
  }

  ExpectTrue(found_player, "player entity should be found by label in GetEntities()");
}

void TestGridDimensionsMirrorWorldGrid() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  ExpectEq(ui.GetGridWidth(),
           static_cast<int>(world.GetGrid().GetWidth()),
           "GetGridWidth should mirror world.GetGrid().GetWidth()");
  ExpectEq(ui.GetGridHeight(),
           static_cast<int>(world.GetGrid().GetHeight()),
           "GetGridHeight should mirror world.GetGrid().GetHeight()");
}

void TestAvailableActionsMetadataIfWorldExposesActions() {
  StubWorld world;
  auto& ui = AddPlayer(world);

  auto actions = ui.GetAvailableActions();

  // Some worlds may not expose actions in StubWorld. If yours does not,
  // swap StubWorld to MazeWorld or InteractionHeavyWorld and keep the rest
  // of this test exactly the same.
  if (actions.empty()) {
    std::cout << "[INFO] Skipping action metadata test because this world "
                 "did not expose any actions.\n";
    return;
  }

  const std::string action_id = actions.front().id;
  ui.RegisterActionMeta(action_id, WebInterface::ActionMeta{"Move Test", "W", true});

  ui.Notify("setup", "mode_change");
  actions = ui.GetAvailableActions();

  bool found_setup_action = false;
  for (const auto& action : actions) {
    if (action.id == action_id) {
      found_setup_action = true;
      ExpectEq(action.label, std::string("Move Test"),
               "action label should reflect registered metadata");
      ExpectEq(action.hotkey, std::string("W"),
               "action hotkey should reflect registered metadata");
      ExpectEq(action.enabled, false,
               "requires_live=true action should be disabled in Setup mode");
    }
  }
  ExpectTrue(found_setup_action,
             "registered action id should still be present in setup mode");

  ui.Notify("live", "mode_change");
  actions = ui.GetAvailableActions();

  bool found_live_action = false;
  for (const auto& action : actions) {
    if (action.id == action_id) {
      found_live_action = true;
      ExpectEq(action.enabled, true,
               "requires_live=true action should be enabled in Live mode");
    }
  }
  ExpectTrue(found_live_action,
             "registered action id should still be present in live mode");
}

}  // namespace

int main() {
  TestStatusModeTickAndWorldName();
  TestResourceNotifyAndResourceClear();
  TestPanelClearPanelLineAndPanelText();
  TestPanelTextDoesNotDestroyResources();
  TestUnknownActionSetsStatusAndDoesNotQueueAction();
  TestSelectedCellRoundTripAndRenderableSelection();
  TestLegendUsesCustomVisualAndFallbacks();
  TestEntityVisualIsAppliedToPlayerEntity();
  TestGridDimensionsMirrorWorldGrid();
  TestAvailableActionsMetadataIfWorldExposesActions();

  std::cout << "WebInterfaceFeatureTest passed.\n";
  return 0;
}