/**
 * @file ActionLog.hpp
 * @author Collin Massmann
 * 
 * @brief Tracks all the actions/moves made by all the agents made within the current world
 */

#pragma once

// include agent class
#include <string>
// include timer class

struct ActionEntry {
  // time of action
  // enum type of action
  // may include length of action 
};

class ActionLog {
private:
  /// Internal data structures
  std::unordered_map<AgentID, std::vector<ActionRecord>> agentActions;

  // timer class usage

public:
  // Constructor/Destructor
  ActionLog();
  ~ActionLog();

  // will be another record action will be added giving the ability to add a timestamp of sorts
  /**
   * @brief Records an action done by an agent and adds it to agentActions
   * 
   * @param agentID The ID of the agent
   * @param action The action done by the agent // may switch to enum if possible
   */
  void recordAction(AgentID agentId, const std::string& action);
    
  // Query methods
  /**
   * @brief Gets the Action log
   * 
   * @return agentActions The unordered map containing all the agents and their actions
   */
  std::unordered_map<AgentID, std::vector<ActionRecord>> getActions() const {return agentActions};
  /**
   * @brief Gets the action log for one agent
   * 
   * @param agentID The ID of the agent of which the action log is requested
   * @return std::vector<ActionRecord> The vector holding all the agents actions
   */
  std::vector<ActionRecord> getActionsByAgent(AgentID agentId) const;

  // File operations if needed
  // bool saveToFile(const std::string& filename) const;
  // bool loadFromFile(const std::string& filename);

  // Utility
  // clears the ActionLog or clears agentActions not decided yet
  void clear();
};

