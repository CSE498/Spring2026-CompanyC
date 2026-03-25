/**
 * @file AnalyticsModuleDemo.cpp
 * @brief End-to-end driver for the analytics module (Timer, ActionLog, DataLog,
 *        ReplayDriver, OutputManager).
 *
 * Demonstrates how these components compose: time a session, record agent actions,
 * aggregate numeric samples in DataLog, optionally replay through ReplayDriver,
 * and report via OutputManager plus a console session dashboard.
 */

#include "ActionLog.hpp"
#include "DataLog.hpp"
#include "OutputManager.hpp"
#include "ReplayDriver.hpp"
#include "Timer.hpp"

#include "../core/AgentBase.hpp"
#include "../core/WorldBase.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{

  using namespace cse498;

  class DemoAgent : public AgentBase
  {
  public:
    DemoAgent(size_t id, const std::string &name, const WorldBase &world)
        : AgentBase(id, name, world) {}

    size_t SelectAction(const WorldGrid &grid) override
    {
      (void)grid;
      return 0;
    }
  };

  class DemoWorld : public WorldBase
  {
  protected:
    void ConfigAgent(AgentBase &agent) override
    {
      // Register every action string used in the scripted session (name -> arbitrary id).
      agent.AddAction("click:ZoneA", 1);
      agent.AddAction("click:ZoneB", 2);
      agent.AddAction("movement:ZoneA", 3);
      agent.AddAction("movement:ZoneB", 4);
    }

  public:
    int DoAction(AgentBase &agent, size_t action_id) override
    {
      (void)agent;
      (void)action_id;
      return 1;
    }
  };

  // Non-owning shared_ptr so ActionLog::recordAction can reference world-owned agents.
  std::shared_ptr<AgentBase> ObserveAgent(AgentBase &agent)
  {
    return std::shared_ptr<AgentBase>(&agent, [](AgentBase *) {});
  }

  std::string MakeAction(const char *interaction, const char *zone)
  {
    return std::string(interaction) + ":" + zone;
  }

  // Split "click:ZoneA" into verb and zone; returns false if malformed.
  bool ParseActionLabel(const std::string &label, std::string &verb_out,
                        std::string &zone_out)
  {
    const auto pos = label.find(':');
    if (pos == std::string::npos)
      return false;
    verb_out = label.substr(0, pos);
    zone_out = label.substr(pos + 1);
    return !verb_out.empty() && !zone_out.empty();
  }

  struct FlattenedEvent
  {
    size_t agent_id{};
    ActionEntry entry{};
  };

  std::vector<FlattenedEvent> FlattenActions(const ActionLog &log)
  {
    std::vector<FlattenedEvent> out;
    for (const auto &kv : log.getActions())
    {
      for (const auto &e : kv.second)
      {
        out.push_back({kv.first, e});
      }
    }
    std::sort(out.begin(), out.end(), [](const FlattenedEvent &a, const FlattenedEvent &b)
              { return a.entry.timeOfAction < b.entry.timeOfAction; });
    return out;
  }

  void PrintSessionDashboard(const std::vector<FlattenedEvent> &timeline,
                             double session_seconds_wall,
                             double session_seconds_timer,
                             const DataLog &durations_log,
                             std::chrono::high_resolution_clock::time_point session_start_wall)
  {
    std::map<std::string, int> zone_counts;
    int clicks = 0;
    int movements = 0;

    double total_duration_us = 0.0;

    for (const auto &fe : timeline)
    {
      std::string verb;
      std::string zone;
      if (ParseActionLabel(fe.entry.actionType, verb, zone))
      {
        zone_counts[zone] += 1;
        if (verb == "click")
          ++clicks;
        else if (verb == "movement")
          ++movements;
      }
      total_duration_us += static_cast<double>(fe.entry.duration.count());
    }

    const std::size_t total_events = timeline.size();
    const double avg_ms =
        total_events > 0 ? (total_duration_us / static_cast<double>(total_events)) / 1000.0
                         : 0.0;

    std::cout << '\n';
    std::cout << "======================================\n";
    std::cout << "     SESSION SUMMARY DASHBOARD\n";
    std::cout << "======================================\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Total Duration: " << session_seconds_timer
              << " seconds (Timer \"session\")\n";
    std::cout << "Wall span (first to last event): " << session_seconds_wall << " seconds\n";
    std::cout << "Total Events: " << total_events << "\n\n";

    std::cout << "Activity by Zone:\n";
    for (const auto &z : zone_counts)
    {
      std::cout << "  - " << z.first << ": " << z.second << " events\n";
    }
    std::cout << '\n';

    std::cout << "Interaction Summary:\n";
    std::cout << "  - Clicks: " << clicks << '\n';
    std::cout << "  - Movements: " << movements << "\n\n";

    std::cout << "Additional Metrics:\n";
    std::cout << "  - Average action duration (from ActionLog): " << avg_ms << " ms\n";

    if (!durations_log.IsEmpty())
    {
      std::cout << "  - DataLog sample mean (ms between scripted steps): "
                << durations_log.Mean() << '\n';
      std::cout << "  - DataLog median: " << durations_log.Median() << '\n';
      std::cout << "  - DataLog min / max: " << durations_log.Min() << " / "
                << durations_log.Max() << '\n';
    }

    if (!timeline.empty())
    {
      const auto t_first = timeline.front().entry.timeOfAction;
      const auto t_last = timeline.back().entry.timeOfAction;
      const double first_since_start =
          std::chrono::duration<double>(t_first - session_start_wall).count();
      const double last_since_start =
          std::chrono::duration<double>(t_last - session_start_wall).count();
      std::cout << "  - First event (seconds since demo start): " << first_since_start << " s\n";
      std::cout << "  - Last event (seconds since demo start): " << last_since_start << " s\n";
    }
    else
    {
      std::cout << "  - First/Last event timestamps: (no events recorded)\n";
    }

    std::cout << "\n======================================\n";
  }

} // namespace

int main()
{
  using namespace cse498;

  const auto wall_session_start = std::chrono::high_resolution_clock::now();

  OutputManager logger;
  logger.setLevel(LogLevel::Verbose);
  logger.enableTimestamps(true);
  logger.enableMetadata(true);
  logger.enableTarget(OutputTarget::Console, true);
  logger.enableTarget(OutputTarget::Buffer, true);

  LogContext ctx;
  ctx.tag = "AnalyticsModule";

  logger.logNormal("=== Analytics module demo: starting session ===", ctx);

  // 1) Timer: track total runtime of the scripted session.
  Timer clock;
  clock.Start("session");

  // 2) World + agents: ReplayDriver replays against real AgentBase / WorldBase hooks.
  DemoWorld world;
  DemoAgent &user1 = world.AddAgent<DemoAgent>("Explorer");
  DemoAgent &user2 = world.AddAgent<DemoAgent>("Scout");

  ActionLog action_log;
  DataLog data_log;

  // 3) Scripted mock session: record actions, optional sleeps + actionEnd for durations.
  struct Step
  {
    AgentBase *agent;
    std::string action;
    int pause_ms;   // simulated work before closing the action
    bool close_end; // call actionEnd after pause
  };

  const std::vector<Step> script = {
      {&user1, MakeAction("click", "ZoneA"), 3, true},
      {&user1, MakeAction("movement", "ZoneB"), 5, true},
      {&user2, MakeAction("click", "ZoneB"), 2, true},
      {&user1, MakeAction("movement", "ZoneA"), 4, true},
      {&user2, MakeAction("click", "ZoneA"), 2, false},
      {&user2, MakeAction("movement", "ZoneB"), 6, true},
      {&user1, MakeAction("click", "ZoneB"), 3, true},
  };

  for (const Step &step : script)
  {
    auto who = ObserveAgent(*step.agent);

    action_log.recordAction(who, step.action);

    std::ostringstream ev;
    ev << "Recorded action '" << step.action << "' for agent " << step.agent->GetID();
    logger.logVerbose(ev.str(), ctx);

    if (step.pause_ms > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(step.pause_ms));
    }
    if (step.close_end)
    {
      action_log.actionEnd(who);
    }

    // DataLog: per-step simulated delay (ms) as numeric telemetry for aggregates.
    data_log.Add(static_cast<double>(step.pause_ms));
  }

  clock.Stop("session");

  const auto wall_session_end = std::chrono::high_resolution_clock::now();
  const double wall_span =
      std::chrono::duration<double>(wall_session_end - wall_session_start).count();

  // 4) ReplayDriver: load ActionLog, step replay, log progress via OutputManager.
  ctx.tag = "ReplayDriver";
  ReplayDriver replayer(world);
  replayer.startReplay(action_log);

  logger.logNormal("Starting replay of recorded actions (stepped).", ctx);

  std::size_t replay_steps = 0;
  while (!replayer.isFinished())
  {
    replayer.update();
    ++replay_steps;
    std::ostringstream step_msg;
    step_msg << "Replay step " << replay_steps << " dispatched.";
    logger.logDebug(step_msg.str(), ctx);
  }

  logger.logNormal("Replay finished.", ctx);

  // 5) Session dashboard (console): aggregates from ActionLog + Timer + DataLog.
  const auto timeline = FlattenActions(action_log);
  PrintSessionDashboard(timeline, wall_span, clock.Last("session"), data_log,
                        wall_session_start);

  logger.logNormal(
      "Buffered log lines captured during run: " +
          std::to_string(logger.getBufferedLogs().size()),
      LogContext{"AnalyticsModule", -1, -1});

  return 0;
}
