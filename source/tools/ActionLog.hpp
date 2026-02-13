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
  std::chrono::microseconds duration;
};

namespace cse498
{  
  class ActionLog {
  private:
    // Using shared_ptr to Agent as key with custom hash/equality
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
      entry.duration = std::chrono::microseconds::zero();
      
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
    
    // No need for manual removal - when shared_ptr count goes to 0, entry is automatically removed
    // (though it won't be removed immediately - only when accessed/erased or map is cleared)
    
    void clear() {
      agentActions.clear();
    }
    
  };
}