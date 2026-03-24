#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/ActionLog.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <thread>

TEST_CASE("Test adding agents into the ActionLog", "[core]")
{
  // Create ActionLog and MazeWorld
  cse498::ActionLog actionLog;
  cse498::MazeWorld world;
  
  // Add agents to the world and store references
  auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
  pacer1.SetLocation(cse498::WorldPosition{3,1});
  
  auto& guard1 = world.AddAgent<cse498::PacingAgent>("Guard 1");
  guard1.SetHorizontal();
  guard1.SetLocation(cse498::WorldPosition{7,7});

  auto& pacer2 = world.AddAgent<cse498::PacingAgent>("Pacer 2");
  pacer1.SetLocation(cse498::WorldPosition{4,1});

  SECTION("Test recording single actions")
  {
    auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});
    auto guard1Ptr = std::shared_ptr<cse498::AgentBase>(&guard1, [](cse498::AgentBase*){});
    auto pacer2Ptr = std::shared_ptr<cse498::AgentBase>(&pacer2, [](cse498::AgentBase*){});
    
    // Move pacer1 up
    cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
    pacer1.SetLocation(cur_position.Up());
    actionLog.recordAction(pacer1Ptr, "MOVE_UP");
    std::this_thread::sleep_for(std::chrono::microseconds(10)); // 10 µs delay
    actionLog.actionEnd(pacer1Ptr);

    // Call actionEnd on an agent not in the map
    actionLog.actionEnd(pacer2Ptr);
    
    // Move guard1 left
    cur_position = guard1.GetLocation().AsWorldPosition();
    guard1.SetLocation(cur_position.Left());
    actionLog.recordAction(guard1Ptr, "MOVE_LEFT");
    
    // Move pacer1 up again
    cur_position = pacer1.GetLocation().AsWorldPosition();
    pacer1.SetLocation(cur_position.Up());
    actionLog.recordAction(pacer1Ptr, "MOVE_UP");

    // Move a nonexistant agent
    actionLog.recordAction(nullptr, "MOVE_LEFT");
    
    // Get actions for each agent
    auto pacerActions = actionLog.getActionsByAgent(pacer1Ptr);
    auto guardActions = actionLog.getActionsByAgent(guard1Ptr);
    
    // Verify counts
    REQUIRE(pacerActions.size() == 2);
    REQUIRE(guardActions.size() == 1);

    // Verify duration of pacer1 first action
    CHECK(pacerActions[0].duration != std::chrono::microseconds::zero());
    
    // Verify action types
    CHECK(pacerActions[0].actionType == "MOVE_UP");
    CHECK(pacerActions[1].actionType == "MOVE_UP");
    CHECK(guardActions[0].actionType == "MOVE_LEFT");
  }
  
  SECTION("Test clearing the log")
  {
    // Create non-owning shared_ptrs
    auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});
    auto guard1Ptr = std::shared_ptr<cse498::AgentBase>(&guard1, [](cse498::AgentBase*){});
    
    // Add some actions
    actionLog.recordAction(pacer1Ptr, "ACTION_1");
    actionLog.recordAction(guard1Ptr, "ACTION_2");
    actionLog.recordAction(pacer1Ptr, "ACTION_3");
    
    // Check map size (number of agents with recorded actions)
    CHECK(actionLog.getActions().size() == 2);
    
    // Check individual agent actions
    CHECK(actionLog.getActionsByAgent(pacer1Ptr).size() == 2);
    CHECK(actionLog.getActionsByAgent(guard1Ptr).size() == 1);

    // Check agent actions on a nullptr
    CHECK(actionLog.getActionsByAgent(nullptr).size() == 0);
    
    // Clear the log
    actionLog.clear();

    // Clearing an already cleared log
    actionLog.clear();
    
    // Verify it's empty
    CHECK(actionLog.getActions().empty());
    CHECK(actionLog.getActionsByAgent(pacer1Ptr).empty());
    CHECK(actionLog.getActionsByAgent(guard1Ptr).empty());
  }

}