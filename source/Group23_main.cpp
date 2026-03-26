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

#include <tuple>

using namespace cse498;

// Timer and ActionLog classes are implemented within the WorldBase.hpp

int main()
{
  MazeWorld world;
  auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
  pacer1.SetLocation(cse498::WorldPosition{3,1});  

  // Available actions for the agents
  [[maybe_unused]] size_t up = 1;
  size_t down = 2;
  [[maybe_unused]]  size_t left = 3;
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


  // DataLog
  DataLog distanceLog;

  // log some numeric values 
  distanceLog.Add(1.5);
  distanceLog.Add(2.0);
  distanceLog.Add(4.5);

  // do stats on numeric values
  [[maybe_unused]] const auto distanceStats =
      std::tuple{distanceLog.Count(), distanceLog.Mean(), distanceLog.Median(), distanceLog.Min(), distanceLog.Max()};

  // log generic values such as WorldPosition 
  DataLog<WorldPosition> positionLog;
  positionLog.Add(pacer1.GetLocation().AsWorldPosition());
  positionLog.Add(cur_position.Right());
  positionLog.Add(cur_position.Right().Right());

  // can still some stats for ordered values
  [[maybe_unused]] const auto positionStats = std::tuple{positionLog.Count(), positionLog.Min(), positionLog.Max()};

  // clear the logs 
  distanceLog.Clear();
  positionLog.Clear();
  [[maybe_unused]] const bool logsCleared = distanceLog.IsEmpty() && positionLog.IsEmpty();

  // DataLog


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