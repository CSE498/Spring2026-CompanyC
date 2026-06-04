/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class for all World modules.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AgentBase.hpp"
#include "ItemBase.hpp"
#include "WorldPosition.hpp"
#include "WorldGrid.hpp"
#include "../tools/ActionLog.hpp"
#include "../tools/DataLog.hpp"
#include "../tools/Timer.hpp"

namespace cse498
{

  using item_ptr_t = std::unique_ptr<ItemBase>;
  using item_set_t = std::vector<item_ptr_t>;
  using agent_ptr_t = std::unique_ptr<AgentBase>;
  using agent_set_t = std::vector<agent_ptr_t>;

  class WorldBase
  {
  protected:
    /// NOTE: derived worlds may choose to have more than one grid.
    WorldGrid main_grid; ///< Main grid for this world

    item_set_t item_set;   ///< Vector of pointers to non-agent entities (ItemBase)
    agent_set_t agent_set; ///< Vector of pointers to agent entities (AgentBase)

    std::unordered_map<std::string, size_t> world_global_counts; /// Set of global resources / counts
    std::vector<std::string> mWorldResourceNames;                ///< List of resource names for this world (e.g., "wood", "stone", etc.)
    std::vector<int> mWorldResourceCounts;                       ///< List of resource counts for this world (e.g., 10 wood, 5 stone, etc.)
    bool run_over = false; ///< Are we finished executing and now shutting down?

    /// Helper function that is run whenever a new agent is created.
    /// @note Override this function to provide agents with actions or other setup.
    virtual void ConfigAgent(AgentBase & /* agent */) {}

  private:
    /// Shared timer for this world instance (used for developer benchmarking/profiling).
    /// @note Access via GetTimer() so all modules use a consistent entry point.
    Timer mTimer;

    /// Action log to keep track of all the actions done by agents in this world
    ActionLog mActionLog;

    /// Position collected when an agent's world position changes.
    DataLog<WorldPosition> mPositionLog;

    /// Numeric analytics collected once per world tick.
    DataLog<> mAnalyticsLog;

  public:
    WorldBase() = default;
    virtual ~WorldBase() = default;

    // -- Accessors --

    /// Get the total number of NON-agent entities
    [[nodiscard]] size_t GetNumItems() const { return item_set.size(); }

    /// Get the total number of AGENT entities
    [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }

    /// Access the shared Timer
    /// Usage: world.GetTimer().Start("...") / Stop("...")
    Timer &GetTimer() { return mTimer; }

    /// Access the shared Timer for this world (const overload)
    [[nodiscard]] const Timer &GetTimer() const { return mTimer; }

    /// Return the current score for worlds that provide score-based results.
    /// Old plan: worlds override GetScore(). New plan: worlds can pass score text into EndGameScreen.
    /// Keeping this as a fallback in case we want a shared numeric score later.
    [[nodiscard]] virtual double GetScore() const { return 0.0; }

    /// Read the standard gameplay session timer.
    [[nodiscard]] double GetPlaytimeSeconds() const {
      return mTimer.Elapsed("Game::Session");
    }

    /// Access the ActionLog
    [[nodiscard]] ActionLog &GetActionLog() { return mActionLog; }

    /// Access the position log used by analytics/end-game heatmaps.
    [[nodiscard]] DataLog<WorldPosition> &GetPositionLog() { return mPositionLog; }

    /// Access the position log used by analytics/end-game heatmaps (const overload).
    [[nodiscard]] const DataLog<WorldPosition> &GetPositionLog() const { return mPositionLog; }

    /// Access the analytics log.
    [[nodiscard]] DataLog<> &GetAnalyticsLog() { return mAnalyticsLog; }
    [[nodiscard]] const DataLog<> &GetAnalyticsLog() const { return mAnalyticsLog; }

    /// Return a reference to an Item with a given ID.
    [[nodiscard]] ItemBase &GetItem(size_t id)
    {
      assert(id < item_set.size());
      return *item_set[id];
    }

    /// Return a CONST reference to an Item with a given ID.
    [[nodiscard]] const ItemBase &GetItem(size_t id) const
    {
      assert(id < item_set.size());
      return *item_set[id];
    }

    /// Return a reference to an Agent with a given ID.
    [[nodiscard]] AgentBase &GetAgent(size_t id)
    {
      assert(id < agent_set.size());
      return *agent_set[id];
    }

    /// Return a CONST reference to an Agent with a given ID.
    [[nodiscard]] const AgentBase &GetAgent(size_t id) const
    {
      assert(id < agent_set.size());
      return *agent_set[id];
    }

    [[nodiscard]] const std::unordered_map<std::string, size_t> &GetWorldGlobalCounts() const
    {
      return world_global_counts;
    }

    /// Return the current numeric analytics values for this world tick.
    /// Derived worlds can override this to add world-specific numeric metrics.
    [[nodiscard]] virtual std::unordered_map<std::string, double> GetAnalyticsSnapshot() const {
      std::unordered_map<std::string, double> snapshot;
      for (const auto& [name, count] : world_global_counts) {
        snapshot[name] = static_cast<double>(count);
      }
      return snapshot;
    }

    /// Record this tick's analytics snapshot in the shared analytics log.
    void RecordAnalyticsSnapshot()
    {
      mAnalyticsLog.AddSnapshot(GetAnalyticsSnapshot());
    }

    [[nodiscard]] const std::vector<std::string> &GetWorldResourceNames() const
    {
      return mWorldResourceNames;
    }
    [[nodiscard]] const std::vector<int> &GetWorldResourceCounts() const
    {
      return mWorldResourceCounts;
    }

    /// Added Setters so Interaction World is easier to set up
    void SetWorldResourceNames(const std::vector<std::string> &names)
    {
      mWorldResourceNames = names;
      mWorldResourceCounts.resize(names.size(), 0);
    }

    void SetWorldResourceCount(size_t idx, int value)
    {
      if (idx < mWorldResourceCounts.size())
      {
        mWorldResourceCounts[idx] = value;
      }
    }

    void AddWorldResourceCount(size_t idx, int delta)
    {
      if (idx < mWorldResourceCounts.size())
      {
        mWorldResourceCounts[idx] += delta;
      }
    }

    [[nodiscard]] virtual size_t GetWidth() const { return main_grid.GetWidth(); }
    [[nodiscard]] virtual size_t GetHeight() const { return main_grid.GetHeight(); }
    [[nodiscard]] virtual size_t GetNumCells() const { return main_grid.GetNumCells(); }

    /// Return an editable version of the current grid for this world (main_grid by default)
    [[nodiscard]] virtual WorldGrid &GetGrid() { return main_grid; }

    /// Return the current grid for this world (main_grid by default)
    [[nodiscard]] virtual const WorldGrid &GetGrid() const { return main_grid; }

    /// Determine if the run has ended.
    [[nodiscard]] virtual bool IsRunOver() const { return run_over; }

    /// Get the tick count
    [[nodiscard]] virtual size_t GetTickCount() const { return 0; };

    // -- Agent Management --

    /// @brief Build a new agent of the specified type
    /// @tparam AGENT_T The type of agent to build
    /// @param agent_name The name of this agent
    /// @return A reference to the newly created agent
    template <typename AGENT_T>
    AGENT_T &AddAgent(std::string agent_name = "None")
    {
      auto agent_ptr = std::make_unique<AGENT_T>(agent_set.size(), agent_name, *this);
      AGENT_T &agent_ref = *agent_ptr;
      ConfigAgent(*agent_ptr);
      if (agent_ptr->Initialize() == false)
      {
        std::cerr << "Failed to initialize agent '" << agent_name << "'." << std::endl;
      }
      agent_set.emplace_back(std::move(agent_ptr)); // Move unique ptr for agent into set.
      return agent_ref;
    }

    // -- Action Management --

    /// @brief Central function for an agent to take any action
    /// @param agent The specific agent taking the action
    /// @param action The id of the action to take
    /// @return The result of this action (usually 0/1 to indicate success)
    /// @note Thus function must be overridden in any derived world.
    virtual int DoAction(AgentBase &agent, size_t action_id) = 0;

    /// @brief Step through each agent giving them a chance to take an action.
    /// @note Override function to control execution order of agents.
    /// @note Override function to control which grid each agent receives.
    virtual void RunAgents() {
      for (const auto& agent_ptr : agent_set) {
        const Location before_location = agent_ptr->GetLocation();

        size_t action_id = agent_ptr->SelectAction(main_grid);
        int result = DoAction(*agent_ptr, action_id);
        if (result) {
          mActionLog.recordAction(agent_ptr->GetID(), action_id,
              std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now().time_since_epoch()));
        }
        agent_ptr->SetActionResult(result);

        const Location& after_location = agent_ptr->GetLocation();
        if (after_location.IsPosition() &&
            (!before_location.IsPosition() ||
             before_location.AsWorldPosition() != after_location.AsWorldPosition())) {
          mPositionLog.Add(agent_ptr->IsInterface() ? "player" : "nonplayer",
                           after_location.AsWorldPosition());
        }
      }
    }

    /// @brief UpdateWorld() is run after every agent has a turn.
    /// Override this function to manage background events for a world.
    /// (E.g., weather, growth, regular physics, etc.)
    virtual void UpdateWorld() {}

    /// @brief Run all agents repeatedly until an end condition is met.
    virtual void Run()
    {
      run_over = false;
      while (!run_over) {
        Tick();
      }
    }

    virtual void Tick() {
      RunAgents();
      UpdateWorld();
      RecordAnalyticsSnapshot();
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //  World API -- the member functions below are intended to be called
    //  from Agents to get more information about their senses and options.
    //
    //////////////////////////////////////////////////////////////////////////

    // Provide a vector of IDs for other agents that the input agent is aware of.
    // (If not overridden, return ALL agents.)
    virtual std::vector<size_t> GetKnownAgents([[maybe_unused]] const AgentBase &agent) const
    {
      std::vector<size_t> out_ids;
      for (const agent_ptr_t &ptr : agent_set)
        out_ids.push_back(ptr->GetID());
      return out_ids;
    }

    // Provide a vector of IDs for items that the input agent is aware of.
    // (If not overridden, return ALL items.)
    [[nodiscard]] std::vector<size_t> GetKnownItems([[maybe_unused]] const AgentBase & agent) const {
      std::vector<size_t> out_ids;
      for (const item_ptr_t &ptr : item_set)
        out_ids.push_back(ptr->GetID());
      return out_ids;
    }
  };

} // End of namespace cse498
