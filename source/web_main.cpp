// web_main.cpp
//
// World-specific entry point for the web demo.
//
// This file is the only place that names a concrete WorldBase subclass.
// To swap in a different world, replace the Initialize<...>() call and the
// SetCellVisual / RegisterActionMeta configuration below.

#include <emscripten.h>

#include "Interfaces/WebApp.hpp"

#include "Agents/PacingAgent.hpp"
#include "Worlds/MazeWorld.hpp"
#include "Worlds/StubWorld.hpp"

using namespace cse498;

int main() {
  g_app = std::make_unique<WebApp>();

  std::string world = "maze";

  if (world == "maze") {
    using agent_t = cse498::PacingAgent;
    auto & world = g_app->Initialize<cse498::MazeWorld>();
    world.AddAgent<agent_t>("Pacer 1").SetLocation(WorldPosition{3,1});
    world.AddAgent<agent_t>("Pacer 2").SetLocation(WorldPosition{6,1});
    world.AddAgent<agent_t>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
    world.AddAgent<agent_t>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
  } else {
    g_app->Initialize<cse498::StubWorld>();
  }

  g_app->SetCellVisual("grass", "#8fd17f", ".");
  g_app->SetCellVisual("tree",  "#3f8f3f", "T");
  g_app->SetCellVisual("stone", "#9ca3af", "S");
  g_app->SetCellVisual("wheat", "#f4d35e", "W");
  g_app->SetCellVisual("wall",  "#374151", "#");
  g_app->SetCellVisual("built", "#8b5cf6", "B");

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
