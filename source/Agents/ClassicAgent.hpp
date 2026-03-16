
/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief An Agent that will walk back and forth along a row or column.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "../core/AgentBase.hpp"

namespace cse498 {

  class ClassicAgent : public AgentBase {
  protected:
    bool vertical=true; ///< Is this agent moving down&up?  False = right&left.
    bool reverse=false;  ///< Is this agent on their way back? (up/left?)

    // Maybe like mode set to instead of moving direction

    // maybe link to shared resource positions by agents

    // Possibly inventory if they are going to gather resources

  public:
    ClassicAgent(size_t id, const std::string & name, const WorldBase & world)
      : AgentBase(id, name, world) { }
    ~ClassicAgent() = default;

    ClassicAgent & SetHorizontal() { vertical = false; return *this; }
    ClassicAgent & SetVertical() { vertical = true; return *this; }
    ClassicAgent & ToggleDirection() { reverse = !reverse; return *this; }

    /// @brief This agent needs a specific set of actions to function.
    /// @return Success: are required actions available?
    bool Initialize() override {
      return HasAction("up") && HasAction("down") && HasAction("left") && HasAction("right");
    }

    // NEED TO WORK ON ACTION MAP, CRITICAL CLASS IN THESE GIVEN CODE ( simply maps word actions to numbers )

    // Need to think of actions to preform based on agent
    // Maybe one agent that does everything or even divide into more sub agents

    /* DECISIONS

    maybe user can select each agent and mark their main goal

    survive
    gather
    patrol
    attack
    explore

    */

    // need to use behavior tree to select a good option, then just return that option and we are done

    /// Choose the action to take a step in the appropriate direction.
    size_t SelectAction(const WorldGrid & /* grid*/) override

    // I think there should be a call on the behavior tree here so our agents think critically,
    // or to be quick we can just let the agents preform simple options

    /*
    Im pretty sure you get fed a grid of a surrounded area, and based on this make a decision that was
    supplied to the agent at the start

    Then you return the decision but don't need to mess with any physical changes to the agent,
    just like what direction to move or maybe like harvest or attack or ...
    */
    {
      // If the last step failed, try going in the other direction.
      if (action_result == 0) ToggleDirection();

      // Take a step in the direction we are trying to go in.
      if (vertical) {
        if (reverse) return action_map["up"];
        else         return action_map["down"];
      } else {
        if (reverse) return action_map["left"];
        else         return action_map["right"];
      }
      return 0;  // Should never actually get here...
    }

  };

} // End of namespace cse498
