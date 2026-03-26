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
  auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
  pacer1.SetLocation(cse498::WorldPosition{3,1});  

  // Available actions for the agents
  size_t up = 1;
  size_t down = 2;
  size_t left = 3;
  size_t right = 4;

  // ActionLog
  // ActionLog is already created within the world through the WorldBase.hpp file 
  // Have and agents action recorded within the log
  cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
  pacer1.SetLocation(cur_position.Up());
  world.GetActionLog().actionEnd(pacer1);

  // Obtain the actions of one agent
  auto pacerActions = world.GetActionLog().getActionsByAgent(pacer1);

  // Obtain the actions of all the agents
  world.GetActionLog().getActions();

  // Clear the ActionLog
  world.GetActionLog().clear();

  // ActionLog


  // Datalog

  // Datalog


  // OutputManager

  // OutputManager


  // ReplayDriver
  cse498::ReplayDriver replayDriver(world);

    // Reset agent to starting position
  pacer1.SetLocation(cse498::WorldPosition{3,1});

  // Record some actions
  world.GetActionLog().recordAction(pacer1, down);
  world.GetActionLog().recordAction(pacer1, right);

  // Start replay
  replayDriver.startReplay(world.GetActionLog());

  // Update step by step
  while (!replayDriver.isFinished()) {
      replayDriver.update();
  }

  // ReplayDriver


  // Timer

  // Timer
  
}