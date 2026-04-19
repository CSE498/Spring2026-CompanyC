// web_main.cpp
//
// World-specific entry point for the web demo.
//
// This file is the only place that names a concrete WorldBase subclass.
// To swap in a different world, replace the Initialize<...>() call and the
// SetCellVisual / RegisterActionMeta configuration below.

#include <emscripten.h>
#include <cstdlib>

#include "Interfaces/WebApp.hpp"
#include "Agents/PacingAgent.hpp"
#include "Worlds/InteractionHeavyWorld.hpp"
#include "Worlds/MazeWorld.hpp"
#include "Worlds/StubWorld.hpp"

using namespace cse498;

int main() {
  g_app = std::make_unique<WebApp>();

  std::string world = GetUrlParam("world");

  if (world == "interaction") {
    using world_t = cse498::InteractionHeavyWorld;
    using agent_t = cse498::PacingAgent;
    auto & world = g_app->Initialize<world_t>();
    world.AddAgent<agent_t>("Pacer 1").SetLocation(WorldPosition{3,1});
    world.AddAgent<agent_t>("Pacer 2").SetLocation(WorldPosition{6,1});
    world.AddAgent<agent_t>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    world.AddAgent<agent_t>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
  } else if (world == "maze") {
    using agent_t = cse498::PacingAgent;
    auto & world = g_app->Initialize<cse498::MazeWorld>();
    world.AddAgent<agent_t>("Pacer 1").SetLocation(WorldPosition{3,1});
    world.AddAgent<agent_t>("Pacer 2").SetLocation(WorldPosition{6,1});
    world.AddAgent<agent_t>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    world.AddAgent<agent_t>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
  } else if (world == "dynamic") {
    using agent_t = cse498::PacingAgent;
    // Uncomment when DynamicWorld is implemented.
    // auto& w = g_app->Initialize<cse498::DyanmicWorld>();
    // w.AddAgent<agent_t>("Pacer 1").SetLocation(WorldPosition{3, 1});
    // w.AddAgent<agent_t>("Pacer 2").SetLocation(WorldPosition{6, 1});
    // g_app->SetPlayerVisible(false);
    // g_app->EnableViewport(32);
  } else { // world == "stub"
    g_app->Initialize<cse498::StubWorld>();
  }

  g_app->SetCellVisual("grass",       "#8fd17f", ".");
  g_app->SetCellVisual("wall",        "#0c1523", "#");
  g_app->SetCellVisual("built",       "#8b5cf6", "B");
  g_app->SetCellVisual("diamond_ore", "#eae2fb", "D");
  g_app->SetCellVisual("exit",        "#a12989", "E");
  g_app->SetCellVisual("gold_ore",    "#e1e827", "G");
  g_app->SetCellVisual("iron_ore",    "#525252", "I");
  g_app->SetCellVisual("boulder",     "#6e4f08", "O");
  g_app->SetCellVisual("stone",       "#9ca3af", "S");
  g_app->SetCellVisual("tree",        "#3f8f3f", "T");
  g_app->SetCellVisual("wheat",       "#f4d35e", "W");

  // Background tiles drawn beneath resource/object cell sprites.
  // Group 7 (outdoor): grass under trees, wheat, stone, wood.
  g_app->RegisterCellBackground("tree",  "assets/grass.png");
  g_app->RegisterCellBackground("wheat", "assets/grass.png");
  g_app->RegisterCellBackground("stone", "assets/grass.png");
  g_app->RegisterCellBackground("wood",  "assets/grass.png");
  // Group 8 (dungeon): floor/dirt under ores and boulders.
  g_app->RegisterCellBackground("boulder",     "assets/floor.png");
  g_app->RegisterCellBackground("gold_ore",    "assets/floor.png");
  g_app->RegisterCellBackground("iron_ore",    "assets/floor.png");
  g_app->RegisterCellBackground("diamond_ore", "assets/floor.png");
  g_app->RegisterCellBackground("exit",        "assets/floor.png");

  // Entity sprites: maps the agent glyph symbol to an asset PNG.
  // 'P' is set by WebApp::Initialize for the human player.
  g_app->RegisterEntityVisual('P', "assets/player.png");
  // '*' is the default AgentBase symbol used by PacingAgent and collector agents.
  g_app->RegisterEntityVisual('*', "assets/agent.png");
  g_app->RegisterEntityVisual('E', "assets/enemy.png");
  g_app->RegisterEntityVisual('G', "assets/goblin.png");

  using Meta = cse498::WebInterface::ActionMeta;
  g_app->RegisterActionMeta("start",   Meta{"Start",   "Enter", false});
  g_app->RegisterActionMeta("reset",   Meta{"Reset",   "R",     false});
  g_app->RegisterActionMeta("save",    Meta{"Save",    "",      false});
  g_app->RegisterActionMeta("load",    Meta{"Load",    "",      false});
  g_app->RegisterActionMeta("up",      Meta{"Up",      "W",     true});
  g_app->RegisterActionMeta("down",    Meta{"Down",    "S",     true});
  g_app->RegisterActionMeta("left",    Meta{"Left",    "A",     true});
  g_app->RegisterActionMeta("right",   Meta{"Right",   "D",     true});
  g_app->RegisterActionMeta("collect", Meta{"Collect", "E",     true});
  g_app->RegisterActionMeta("build",   Meta{"Build",   "B",     true});

  g_app->Render();
  emscripten_exit_with_live_runtime();
  return 0;
}
