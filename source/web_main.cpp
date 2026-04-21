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

#include "Agents/ClassicAgent.hpp"
#include "Agents/PacingAgent.hpp"
#include "Agents/SmartAgent.hpp"
#include "Agents/TendencyAgent.hpp"
#include "Agents/HunterAgent.hpp"
#include "Agents/GoblinAgent.hpp"

#include "Worlds/DynamicWorld.hpp"
#include "Worlds/InteractionHeavyWorld.hpp"
#include "Worlds/MazeWorld.hpp"
#include "Worlds/SokobanWorld.hpp"
#include "Worlds/StubWorld.hpp"

using namespace cse498;

// Minimal agent that always returns a fixed action ID for controlled testing.
class StubAgent : public cse498::AgentBase
{
public:
  StubAgent(size_t id, const std::string &name, const cse498::WorldBase &world)
      : AgentBase(id, name, world) {}

  size_t SelectAction(cse498::WorldGrid &) override
  {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> choice(0, this->action_map.size() - 1);

    return choice(gen);
  }
};

int main()
{
  g_app = std::make_unique<WebApp>();

  std::string run_mode = GetUrlParam("world");

  if (run_mode == "classic_agent")
  {
    using world_t = cse498::MazeWorld;
    auto &world = g_app->Initialize<world_t>();
    world.AddAgent<ClassicAgent>("Classic 1").SetLocation(WorldPosition{3, 1});
  }
  else if (run_mode == "smart_agent")
  {
    using world_t = cse498::MazeWorld;
    auto &world = g_app->Initialize<world_t>();
    world.AddAgent<SmartAgent>("SmartAgent").SetLocation(WorldPosition{3, 1});
  }
  else if (run_mode == "tendency_agent")
  {
    using world_t = cse498::MazeWorld; // chnage back to maze
    auto &world = g_app->Initialize<world_t>();
    world.AddAgent<TendencyAgent>("Tendency").SetLocation(WorldPosition{3, 1});
  }
  else if (run_mode == "hunter_agent")
  {
    using world_t = cse498::InteractionHeavyWorld;
    auto &world = g_app->Initialize<world_t>();
    world.AddAgent<HunterAgent>("Hunter").SetLocation(WorldPosition{3, 1});
  }
  else if (run_mode == "dynamic")
  {
    constexpr int basicAgentCount = 15;

    using world_t = cse498::DynamicWorld;
    auto &world = g_app->Initialize<world_t>();
    world.AddAgent<StubAgent>("Leader").SetLocation(Location{{0, 0}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> x_pos(0, world.GetWidth() - 1);
    std::uniform_int_distribution<int> y_pos(0, world.GetHeight() - 1);

    for (int i = 0; i < basicAgentCount; i++)
    {
      std::string name = "Basic Agent " + std::to_string(i + 1);
      world.AddAgent<StubAgent>(name).SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
    }
  }
  else if (run_mode == "interaction")
  {
    using world_t = cse498::InteractionHeavyWorld;
    auto &world = g_app->Initialize<world_t>();

    // Hunters are spawned from the 'H' markers loaded from the interaction map.
    const auto hunter_spawns = world.GetHunterSpawnPositions();
    for (size_t i = 0; i < hunter_spawns.size(); ++i)
    {
        world.AddAgent<cse498::HunterAgent>("Hunter " + std::to_string(i + 1))
            .SetLocation(hunter_spawns[i]);
    }

    // Goblins are spawned from the 'G' markers loaded from the interaction map.
    const auto goblin_spawns = world.GetGoblinSpawnPositions();
    for (size_t i = 0; i < goblin_spawns.size(); ++i)
    {
      world.AddAgent<cse498::GoblinAgent>("Goblin " + std::to_string(i + 1))
          .SetLocation(goblin_spawns[i]);
    }

    const auto pacer_spawns = world.GetPacerSpawnPositions();
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> coin(0, 1);
    for (size_t i = 0; i < pacer_spawns.size(); ++i)
    {
        auto &agent = world.AddAgent<cse498::PacingAgent>("Pacer " + std::to_string(i + 1));
        agent.SetLocation(pacer_spawns[i]);
        if (coin(gen) == 0)
            agent.SetHorizontal();
        else
            agent.SetVertical();
    }
  }
  else if (run_mode == "maze")
  {
    using agent_t = cse498::PacingAgent;
    auto &world = g_app->Initialize<cse498::MazeWorld>();
    world.AddAgent<agent_t>("Pacer 1").SetLocation(WorldPosition{3, 1});
    world.AddAgent<agent_t>("Pacer 2").SetLocation(WorldPosition{6, 1});
    world.AddAgent<agent_t>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7, 7});
    world.AddAgent<agent_t>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8, 8});
  }
  else if (run_mode == "sokoban")
  {
    g_app->Initialize<cse498::SokobanWorld>();
  } else { // run_mode == "stub"
    auto& stub = g_app->Initialize<cse498::StubWorld>();
    stub.SetDatabase(&g_app->GetDatabase());
    g_app->SetSaveCallback([&stub]() { stub.SaveState("stub_world"); });
    g_app->SetLoadCallback([&stub]() { stub.LoadState("stub_world"); });
  }

  g_app->SetCellVisual("grass", "#8fd17f", ".");
  g_app->SetCellVisual("wall", "#0c1523", "#");
  g_app->SetCellVisual("button", "#629cfa", "o");
  g_app->SetCellVisual("built", "#8b5cf6", "B");
  g_app->SetCellVisual("diamond_ore", "#eae2fb", "D");
  g_app->SetCellVisual("exit", "#a12989", "E");
  g_app->SetCellVisual("gold_ore", "#e1e827", "G");
  g_app->SetCellVisual("iron_ore", "#525252", "I");
  g_app->SetCellVisual("boulder", "#6e4f08", "O");
  g_app->SetCellVisual("stone", "#9ca3af", "S");
  g_app->SetCellVisual("tree", "#3f8f3f", "T");
  g_app->SetCellVisual("wheat", "#f4d35e", "W");
  g_app->SetCellVisual("pressed", "#0f30ee", "X");

  using Meta = cse498::WebInterface::ActionMeta;
  g_app->RegisterActionMeta("start", Meta{"Start", "Enter", false});
  g_app->RegisterActionMeta("reset", Meta{"Reset", "R", false});
  g_app->RegisterActionMeta("save", Meta{"Save", "", false});
  g_app->RegisterActionMeta("load", Meta{"Load", "", false});
  g_app->RegisterActionMeta("up", Meta{"Up", "W", true});
  g_app->RegisterActionMeta("down", Meta{"Down", "S", true});
  g_app->RegisterActionMeta("left", Meta{"Left", "A", true});
  g_app->RegisterActionMeta("right", Meta{"Right", "D", true});
  g_app->RegisterActionMeta("collect", Meta{"Collect", "E", true});
  g_app->RegisterActionMeta("pay", Meta{"Pay", "O", true});
  g_app->RegisterActionMeta("break_boulder", Meta{"Break", "B", true});
  g_app->RegisterActionMeta("throw_up", Meta{"Throw Up", "I", true});
  g_app->RegisterActionMeta("throw_down", Meta{"Throw Down", "K", true});
  g_app->RegisterActionMeta("throw_left", Meta{"Throw Left", "J", true});
  g_app->RegisterActionMeta("throw_right", Meta{"Throw Right", "L", true});
  g_app->RegisterActionMeta("print_inventory", Meta{"Inventory", "P", true});

  // Connect to SaveServer for save/load persistence.
  // Default: ws://localhost:8080, override with ?server=ws://host:port
  std::string server_url = GetUrlParam("server", "ws://localhost:8080");
  g_app->ConnectToServer(server_url);

  g_app->Render();
  emscripten_exit_with_live_runtime();
  return 0;
}
