/**
 * @file Group23_demo.cpp
 * @brief Example driver for Group 23 tools: ActionLog, DataLog, Timer, ReplayDriver, and
 *        OutputManager.
 *
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 */

#include "../source/Agents/PacingAgent.hpp"
#include "../source/Worlds/MazeWorld.hpp"
#include "../source/tools/ActionLog.hpp"
#include "../source/tools/DataLog.hpp"
#include "../source/tools/OutputManager.hpp"
#include "../source/tools/ReplayDriver.hpp"

#include <cstddef>
#include <string>

using namespace cse498;

namespace {

/**
 * @brief Build a default logging context tag for this demo.
 * @return Context with tag "Group23", no specific agent, tick 0.
 */
[[nodiscard]] LogContext MakeDemoLogContext() {
  LogContext ctx;
  ctx.tag = "Group23";
  ctx.agentId = 0;
  ctx.tick = 0;
  return ctx;
}

/**
 * @brief Configure console and buffer logging with timestamps and metadata.
 * @param logger OutputManager instance to configure.
 */
void ConfigureLogger(OutputManager &logger) {
  logger.setLevel(LogLevel::Verbose);
  logger.enableTimestamps(true);
  logger.enableMetadata(true);
  logger.enableTarget(OutputTarget::Console, true);
  logger.enableTarget(OutputTarget::Buffer, true);
}

/**
 * @brief Format a timer segment duration in seconds for log messages.
 * @param timer Timer that has already stopped the named segment.
 * @param segmentName Label passed to Timer::Start / Stop.
 * @return String suitable for concatenation into log lines (Timer::Last returns 0.0 if missing).
 */
[[nodiscard]] std::string FormatTimerSeconds(const Timer &timer,
                                             const std::string &segmentName) {
  return std::to_string(timer.Last(segmentName));
}

/**
 * @brief Demonstrate ActionLog: record movement, query by agent, clear, and time the block.
 * @param world Simulation world (owns ActionLog).
 * @param timer Shared world timer.
 * @param logger Demo logger.
 * @param ctx Logging context.
 * @param pacer Agent used for scripted moves.
 */
void DemonstrateActionLog(MazeWorld &world, Timer &timer, OutputManager &logger,
                          LogContext &ctx, PacingAgent &pacer) {
  // World's action history: record, query, and clear in this demo block.
  ActionLog &actionLog = world.GetActionLog();

  timer.Start("ActionLog::DemoBlock:session");

  // Scripted move; actionEnd updates duration on the agent's last log entry (if any).
  const WorldPosition positionBeforeMove = pacer.GetLocation().AsWorldPosition();
  pacer.SetLocation(positionBeforeMove.Up());
  actionLog.actionEnd(pacer);

  const std::vector<ActionEntry> pacerActions =
      actionLog.getActionsByAgent(pacer);

  // Exercise the full log map API (used by replay and analytics).
  const auto &allActions = actionLog.getActions();
  std::size_t totalEntries = 0;
  for (const auto &pair : allActions) {
    totalEntries += pair.second.size();
  }
  logger.logVerbose("ActionLog — total recorded entries: " + std::to_string(totalEntries),
                    ctx);

  actionLog.clear();

  timer.Stop("ActionLog::DemoBlock:session");

  const std::string actionLogDuration =
      FormatTimerSeconds(timer, "ActionLog::DemoBlock:session");

  logger.logNormal("ActionLog — actions for Pacer1: " + std::to_string(pacerActions.size()),
                   ctx);
  logger.logVerbose("ActionLog cleared", ctx);
  logger.logNormal("ActionLog demo block duration: " + actionLogDuration + " seconds", ctx);
}

/**
 * @brief Demonstrate numeric and typed DataLog usage.
 * @param logger Demo logger.
 * @param ctx Logging context.
 * @param pacer Agent supplying sample positions.
 * @param baselinePosition Position before the scripted Up() move (used for relative Rights).
 */
void DemonstrateDataLog(OutputManager &logger, LogContext &ctx, PacingAgent &pacer,
                        const WorldPosition &baselinePosition) {
  // Sample distances (arbitrary doubles) to exercise mean/median/min/max in logs.
  DataLog<double> distanceLog;
  distanceLog.Add(1.5);
  distanceLog.Add(2.0);
  distanceLog.Add(4.5);

  logger.logNormal("DataLog (Distance) — count: " + std::to_string(distanceLog.Count()) +
                       ", mean: " + std::to_string(distanceLog.Mean()) +
                       ", median: " + std::to_string(distanceLog.Median()) +
                       ", min: " + std::to_string(distanceLog.Min()) +
                       ", max: " + std::to_string(distanceLog.Max()),
                   ctx);

  // Grid cells visited or referenced; used to show DataLog with a non-scalar type.
  DataLog<WorldPosition> positionLog;
  positionLog.Add(pacer.GetLocation().AsWorldPosition());
  // Deliberately use the pre-move baseline so Rights chain from the original cell.
  positionLog.Add(baselinePosition.Right());
  positionLog.Add(baselinePosition.Right().Right());

  logger.logNormal("DataLog (Position) — count: " + std::to_string(positionLog.Count()),
                   ctx);
  logger.logVerbose("Min/Max position values recorded successfully", ctx);

  distanceLog.Clear();
  positionLog.Clear();

  if (!distanceLog.IsEmpty() || !positionLog.IsEmpty()) {
    logger.logNormal("DataLog — warning: expected empty logs after Clear()", ctx);
  }
}

/**
 * @brief Record two moves, replay them stepwise, and log timing.
 * @param world Simulation world.
 * @param timer Shared world timer.
 * @param logger Demo logger.
 * @param ctx Logging context.
 * @param pacer Agent whose actions are replayed.
 * @param down Expected action ID for "down" (must match agent/world registration).
 * @param right Expected action ID for "right" (must match agent/world registration).
 */
void DemonstrateReplay(MazeWorld &world, Timer &timer, OutputManager &logger,
                       LogContext &ctx, PacingAgent &pacer,
                       std::size_t down, std::size_t right) {
  ActionLog &actionLog = world.GetActionLog();

  pacer.SetLocation(WorldPosition{3, 1});

  if (pacer.GetActionID("down") != down || pacer.GetActionID("right") != right) {
    logger.logNormal(
        "ReplayDriver demo skipped: agent missing 'down' or 'right' action registration", ctx);
    return;
  }

  actionLog.recordAction(pacer, down);
  actionLog.recordAction(pacer, right);

  ReplayDriver replayDriver(world);

  timer.Start("ReplayDriver::Replay:session");
  replayDriver.startReplay(actionLog);

  constexpr std::size_t kMaxReplaySteps = 100000;
  std::size_t steps = 0;
  while (!replayDriver.isFinished()) {
    replayDriver.update();
    ++steps;
    if (steps > kMaxReplaySteps) {
      logger.logNormal("ReplayDriver — aborted: exceeded step limit (" +
                           std::to_string(kMaxReplaySteps) + ")",
                       ctx);
      break;
    }
  }

  timer.Stop("ReplayDriver::Replay:session");
}

}  // namespace

/**
 * @brief Entry point: runs scripted demos for Group 23 logging and replay utilities.
 * @return 0 on normal completion.
 */
int main() {
  MazeWorld world;
  Timer &timer = world.GetTimer();
  PacingAgent &pacer = world.AddAgent<PacingAgent>("Pacer 1");
  pacer.SetLocation(WorldPosition{3, 1});

  // Action IDs aligned with MazeWorld::ActionType (up/down/left/right); used for replay demo.
  [[maybe_unused]] size_t up = 1;
  size_t down = 2;
  [[maybe_unused]] size_t left = 3;
  size_t right = 4;

  OutputManager logger;
  ConfigureLogger(logger);

  LogContext ctx = MakeDemoLogContext();

  logger.logNormal("Simulation started", ctx);
  logger.logVerbose("Pacer1 initial position set to (3,1)", ctx);

  // Snapshot before ActionLog demo's Up() so position DataLog can anchor Rights to this cell.
  const WorldPosition baselineBeforeActionDemo = pacer.GetLocation().AsWorldPosition();

  DemonstrateActionLog(world, timer, logger, ctx, pacer);

  logger.logNormal("Timer — ActionLog block: " +
                       FormatTimerSeconds(timer, "ActionLog::DemoBlock:session") + " seconds",
                   ctx);

  DemonstrateDataLog(logger, ctx, pacer, baselineBeforeActionDemo);

  DemonstrateReplay(world, timer, logger, ctx, pacer, down, right);

  logger.logNormal("Timer — Replay block: " +
                       FormatTimerSeconds(timer, "ReplayDriver::Replay:session") + " seconds",
                   ctx);

  logger.logNormal("Simulation finished.", ctx);

  return 0;
}
