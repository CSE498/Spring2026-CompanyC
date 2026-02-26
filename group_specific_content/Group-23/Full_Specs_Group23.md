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