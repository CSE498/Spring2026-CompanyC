/**
 * @file ActionLog.hpp
 * @author Group 23
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
    std::chrono::steady_clock::time_point timeOfAction;
    size_t actionType;
    std::chrono::microseconds duration;
  };

  class ActionLog {
  private:
    // Unordered map holding all the actions done by all the agents using their ID's as a key
    std::unordered_map<size_t, std::vector<ActionEntry>> agentActions;
    
  public:
    /// Constructor
    ActionLog()  {std::chrono::steady_clock::now();}
    
    /**
     * @brief Records the given action for the given agent
     * 
     * @param agent the agent the action belongs to
     * @param action the action the agent is performing
     */
    void recordAction(const AgentBase& agent, const size_t& action) {
      size_t id = agent.GetID();

      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
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
    std::vector<ActionEntry> getActionsByAgent(const AgentBase& agent) const {
      size_t id = agent.GetID();
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
    void actionEnd(const AgentBase& agent){
      size_t id = agent.GetID();
      auto it = agentActions.find(id);

      if (it != agentActions.end() && !it->second.empty()) {
          auto& last = it->second.back();
          last.duration = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now() - last.timeOfAction);
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