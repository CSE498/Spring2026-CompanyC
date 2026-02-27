/**
 * @file ActionLog.hpp
 * @author Collin Massmann
 *
 * @brief Tracks all the actions dones by agents within a world
 */
#pragma once

#include "../core/AgentBase.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498
{  

  // Data structure holding all the information about the action
  struct ActionEntry {
  std::chrono::high_resolution_clock::time_point timeOfAction;
  std::string actionType;
  std::chrono::microseconds duration;
  };

  class ActionLog {
  private:
    // Unordered map holding all the actions done by all the agents using their ID's as a key
    std::unordered_map<size_t, std::vector<ActionEntry>> agentActions;
    
    // Clock used for timing actions
    std::chrono::high_resolution_clock::time_point simStartTime;
    
  public:
    /// Constructor
    ActionLog() : simStartTime(std::chrono::high_resolution_clock::now()) {}
    
    /**
     * @brief Records the given action for the given agent
     * 
     * @param agent the agent the action belongs to
     * @param action the action the agent is performing
     */
    void recordAction(std::shared_ptr<AgentBase> agent, const std::string& action) {
      if (!agent) return;
      
      size_t id = agent->GetID();

      auto now = std::chrono::high_resolution_clock::now();
      ActionEntry entry{now, action, std::chrono::microseconds::zero()};
      agentActions[id].push_back(entry);
    }

    /**
     * @brief Returns the action log
     * 
     * @return The unordered map holding all the actions
     */
    const std::unordered_map<size_t, std::vector<ActionEntry>>& getActions() const {
      return agentActions;
    }
    
    /**
     * @brief Returns all the actions done by one agent
     * 
     * @param agent the agent whose actions is requested
     * @return The vector of all the agents actions
     */
    std::vector<ActionEntry> getActionsByAgent(std::shared_ptr<AgentBase> agent) const {
      if (!agent) return {};

      size_t id = agent->GetID();
      auto it = agentActions.find(id);
      
      if (it != agentActions.end()) {
        return it->second;
      }
      return {};
    }

    /**
     * @brief Ends an agents action if it took time to complete
     * 
     * @param agent the agent whos action is ending
     */
    void actionEnd(std::shared_ptr<AgentBase> agent){
      if (!agent) return;

      size_t id = agent->GetID();
      auto it = agentActions.find(id);

      if (it != agentActions.end() && !it->second.empty()) {
          auto& last = it->second.back();
          last.duration = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now() - last.timeOfAction);
      }
    }
    
    /**
     * @brief Clears the action log
     */
    void clear() {
      agentActions.clear();
    }
    
  };
}