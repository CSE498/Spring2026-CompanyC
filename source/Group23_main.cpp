/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief An example driver for Group 23 module
 **/

#include "Agents/PacingAgent.hpp"
#include "Worlds/MazeWorld.hpp"
#include "Interfaces/TrashInterface.hpp"
#include "tools/DataLog.hpp"
#include "tools/ReplayDriver.hpp"
#include "tools/OutputManager.hpp"

using namespace cse498;

// Timer and ActionLog classes are implemented within the WorldBase.hpp

int main()
{
  MazeWorld world;
  world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
  world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
  world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
  world.AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
  world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});

  // ActionLog

  // ActionLog

  // Datalog

  // Datalog

  // OutputManager

  // OutputManager

  // ReplayDriver

  // ReplayDriver

  // Timer

  // Timer

  world.Run();
  
}