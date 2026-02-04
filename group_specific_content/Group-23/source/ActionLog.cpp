/**
 * @file ActionLog.cpp
 * @author Collin Massmann
 * 
 * @brief Tracks all the actions/moves made by all the agents made within the current world
 */

#include "ActionLog.hpp"
// include agent class
#include <string>
// include timer class

void ActionLog::recordAction(AgentID agentId, const std::string& action){
  // creates a struct ActionEntry from the ActionLog.hpp file

  // adds the ActionEntry to the agentActions map
}

std::vector<ActionRecord> ActionLog::getActionsByAgent(AgentID agentId) const{
    // uses agentID to search in the map

    // returns the vector of the agents actions
    return 0;
}