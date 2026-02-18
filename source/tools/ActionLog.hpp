#pragma once

#include "../core/AgentBase.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct ActionEntry {
  std::chrono::high_resolution_clock::time_point timeOfAction;
  std::string actionType;
};

namespace cse498
{  
  class ActionLog {
  private:
    // Using shared_ptr to Agent as key
    std::unordered_map<std::shared_ptr<AgentBase>, std::vector<ActionEntry>> agentActions;
    
    std::chrono::high_resolution_clock::time_point simulationStartTime;
    
  public:
    ActionLog() : simulationStartTime(std::chrono::high_resolution_clock::now()) {}
    
    void recordAction(std::shared_ptr<AgentBase> agent, const std::string& action) {
      if (!agent) return;
      
      auto now = std::chrono::high_resolution_clock::now();
      
      ActionEntry entry;
      entry.timeOfAction = now;
      entry.actionType = action;
            
      agentActions[agent].push_back(entry);
    }

    const std::unordered_map<std::shared_ptr<AgentBase>, std::vector<ActionEntry>>& getActions() const {
      return agentActions;
    }
    
    std::vector<ActionEntry> getActionsByAgent(std::shared_ptr<AgentBase> agent) const {
      auto it = agentActions.find(agent);
      if (it != agentActions.end()) {
        return it->second;
      }
      return {};
    }
    
    void clear() {
      agentActions.clear();
    }
    
  };
}