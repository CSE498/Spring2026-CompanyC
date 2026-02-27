---
layout: default
---

# Full Spec

## Company C - Group 23

**Team Members**:
1. Collin Massmann
2. Lauren Phillips
3. Muhammad Chohan
4. Meghan Carter
5. Ismail Abdi

## ActionLog (Collin):

ActionLog will track all the actions/moves made by all the agents within the current world using an unordered map with a smart pointer as the keys with a vector of a struct called ActionEntry, which holds the time of the action and the action type, as the data. Additionally, all the actions/moves will have a time stamp and will track timings in between moves allowing replay, analysis, and debugging of agent behaviors. 

Implemented Functions:

recordAction(std::shared_ptr<AgentBase> agent, const std::string& action) Records the action for the given agent

std::unordered_map<std::shared_ptr<AgentBase>, std::vector<ActionEntry>>& getActions() Returns the unordered map holding all the pointers and actions

std::vector<ActionEntry> getActionsByAgent(std::shared_ptr<AgentBase> agent) Returns the actions for the given agent

clear() clears the unordered map/action log

## Timer (Lauren):
Timer is our performance utility for timing named sections of the simulation (ex: World:: Update, agent decision making, pathfinding, rendering, logging, etc.) It uses steady clock and stores a TimerEntry per name, tracking whether the timer is currently running plus summary stats across completed runs: count,last,total,min,max, and avg. Misuse (start while running, stop while not running, unknown name, etc.) is treated as a programmer error via assert to fail fast during development.

Implemented Fns:
Start(const std::string& name) Begins timing a named section
Stop(const std::string& name) Stops timing and updates stats
Reset(const std::string& name) Clears one timer’s stored stats
ResetAll() Clears all timers
HasData(const std::string& name) const True if at least one measurement exists
Last/Min/Max/Average/Count(const std::string& name) const Query recorded stats

Planned Next Steps: Integrating timings into a post session user dashboard (ex: time spent in combat vs gathering vs exploring, user stats/rankings across plays, etc.)

## ReplayDriver (Meghan):
ReplayDriver is our playback utility that reconstructs agent behavior from an ActionLog by re-sending recorded actions back into the World. It converts each logged action into a ReplayEvent (agent_id, actionType, timestamp), sorts events chronologically, and then replays them one at a time through an update() loop. This supports debugging and visualizing how multiple agents acted over time.

ReplayDriver is designed to work with WorldBase + AgentBase without depending on a specific world type. Actions are stored in the log as strings (ex: "down", "right"), and ReplayDriver resolves them at playback time using the agent’s action registry (string → action_id) before calling WorldBase::DoAction.

Implemented Functions:
startReplay(const ActionLog& log) Loads events from the ActionLog, sorts events by timestamp and begins playback
sendAction(const ReplayEvent& event) Finds the correct agent and executes action
update() Advances replay by exactly one event per call 
pauseReplay() / resumeReplay() Temporarily halts playback without clearing loaded events.
resetReplay() Resets replay progress and stops playback
clearReplay() Clears all loaded events and returns the replay system to its initial state.
Status helpers:
isRunning(), isPaused(), isFinished()