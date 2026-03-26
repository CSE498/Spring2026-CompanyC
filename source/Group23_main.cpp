/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief An example driver for Group 23 module
 **/

#include <iostream>
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

  //get the shared Timer owned by WorldBase
  auto & timer = world.GetTimer();

  auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
  pacer1.SetLocation(cse498::WorldPosition{3,1});  

  // Available actions for the agents
  [[maybe_unused]] size_t up = 1;
  size_t down = 2;
  [[maybe_unused]]  size_t left = 3;
  size_t right = 4;

  // OutputManager
  OutputManager logger;
  logger.setLevel(LogLevel::Verbose);
  logger.enableTimestamps(true);
  logger.enableMetadata(true);
  logger.enableTarget(OutputTarget::Console, true);
  logger.enableTarget(OutputTarget::Buffer, true);

  LogContext ctx;
  ctx.tag = "Group23";
  ctx.agentId = 0;
  ctx.tick = 0;

  logger.logNormal("Simulation started", ctx);
  logger.logVerbose("Pacer1 initial position set to (3,1)", ctx);
  // OutputManager

  // ActionLog
  // ActionLog is already created within the world through the WorldBase.hpp file 

  //time the ActionLog demo block
  timer.Start("ActionLog::DemoBlock:session");

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

  //stop timing the ActionLog demo block
  timer.Stop("ActionLog::DemoBlock:session");

  logger.logNormal("ActionLog — actions for Pacer1: "
    + std::to_string(pacerActions.size()), ctx);
  logger.logVerbose("ActionLog cleared", ctx);
  logger.logNormal("ActionLog demo block duration: "
      + std::to_string(timer.Last("ActionLog::DemoBlock:session"))
      + " seconds", ctx);
    

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

  logger.logNormal("DataLog (Distance) — count: "  + std::to_string(distanceLog.Count())
    + ", mean: "   + std::to_string(distanceLog.Mean())
    + ", median: " + std::to_string(distanceLog.Median())
    + ", min: "    + std::to_string(distanceLog.Min())
    + ", max: "    + std::to_string(distanceLog.Max()), ctx);


  // log generic values such as WorldPosition 
  DataLog<WorldPosition> positionLog;
  positionLog.Add(pacer1.GetLocation().AsWorldPosition());
  positionLog.Add(cur_position.Right());
  positionLog.Add(cur_position.Right().Right());

  // can still some stats for ordered values
  [[maybe_unused]] const auto positionStats = std::tuple{positionLog.Count(), positionLog.Min(), positionLog.Max()};

  logger.logNormal("DataLog (Position) — count: "
    + std::to_string(positionLog.Count()), ctx);
  logger.logVerbose("Min/Max position values recorded successfully", ctx);

  // clear the logs 
  distanceLog.Clear();
  positionLog.Clear();
  [[maybe_unused]] const bool logsCleared = distanceLog.IsEmpty() && positionLog.IsEmpty();

  // DataLog





  // ReplayDriver
  cse498::ReplayDriver replayDriver(world);

    // Reset agent to starting position
  pacer1.SetLocation(cse498::WorldPosition{3,1});

  // Record some actions
  world.GetActionLog().recordAction(pacer1, down);
  world.GetActionLog().recordAction(pacer1, right);

  //time the replay block
  timer.Start("ReplayDriver::Replay:session");

  // Start replay
  replayDriver.startReplay(world.GetActionLog());

  // Update step by step
  while (!replayDriver.isFinished()) {
      replayDriver.update();
  }

  //stop timing the replay block
  timer.Stop("ReplayDriver::Replay:session");

  // ReplayDriver



  // -----------------------------------
  // Timer summary
  // -----------------------------------
  logger.logNormal("Timer — ActionLog block: "
    + std::to_string(timer.Last("ActionLog::DemoBlock:session"))
    + " seconds", ctx);
  logger.logNormal("Timer — Replay block: "
    + std::to_string(timer.Last("ReplayDriver::Replay:session"))
    + " seconds", ctx);


  logger.logNormal("Simulation finished.", ctx);

  return 0;



  
}