#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/tools/ActionLog.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

TEST_CASE("Test adding agents into the ActionLog", "[core]") {
  // Create ActionLog and MazeWorld
  cse498::ActionLog actionLog;
  cse498::MazeWorld world;

  // Add agents to the world and store references
  auto &pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
  pacer1.SetLocation(cse498::WorldPosition{3, 1});

  auto &guard1 = world.AddAgent<cse498::PacingAgent>("Guard 1");
  guard1.SetHorizontal();
  guard1.SetLocation(cse498::WorldPosition{7, 7});

  auto &pacer2 = world.AddAgent<cse498::PacingAgent>("Pacer 2");

  size_t up = 1;
  size_t down = 2;
  size_t left = 3;
  size_t right = 4;

  SECTION("Test recording single actions") {
    // Move pacer1 up
    cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
    pacer1.SetLocation(cur_position.Up());
    actionLog.recordAction(pacer1.GetID(), up, std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::steady_clock::now().time_since_epoch()));
    std::this_thread::sleep_for(std::chrono::microseconds(10)); // 10 µs delay
    actionLog.actionEnd(pacer1.GetID(), std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Call actionEnd on an agent not in the map
    actionLog.actionEnd(pacer2.GetID(), std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Call actionEnd on an agent without any actions
    actionLog.actionEnd(guard1.GetID(), std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Move guard1 left
    cur_position = guard1.GetLocation().AsWorldPosition();
    guard1.SetLocation(cur_position.Left());
    actionLog.recordAction(guard1.GetID(), left, std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Move pacer1 up again
    cur_position = pacer1.GetLocation().AsWorldPosition();
    pacer1.SetLocation(cur_position.Up());
    actionLog.recordAction(pacer1.GetID(), up, std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Get actions for each agent
    auto pacerActions = actionLog.getActionsByAgent(pacer1.GetID());
    auto guardActions = actionLog.getActionsByAgent(guard1.GetID());

    // Verify counts
    REQUIRE(pacerActions.size() == 2);
    REQUIRE(guardActions.size() == 1);

    // Verify duration of pacer1 first action
    CHECK(pacerActions[0].duration != std::chrono::microseconds::zero());

    // Verify action types
    CHECK(pacerActions[0].actionType == up);
    CHECK(pacerActions[1].actionType == up);
    CHECK(guardActions[0].actionType == left);
    actionLog.clear();
  }

  SECTION("Test clearing the log") {
    // Add some actions
    actionLog.recordAction(pacer1.GetID(), down, std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));
    actionLog.recordAction(guard1.GetID(), right, std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));
    actionLog.recordAction(pacer1.GetID(), up, std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch()));

    // Check map size (number of agents with recorded actions)
    CHECK(actionLog.getActions().size() == 2);

    // Check individual agent actions
    CHECK(actionLog.getActionsByAgent(pacer1.GetID()).size() == 2);
    CHECK(actionLog.getActionsByAgent(guard1.GetID()).size() == 1);

    // Clear the log
    actionLog.clear();

    // Clearing an already cleared log
    actionLog.clear();

    // Verify it's empty
    CHECK(actionLog.getActions().empty());
    CHECK(actionLog.getActionsByAgent(pacer1.GetID()).empty());
    CHECK(actionLog.getActionsByAgent(guard1.GetID()).empty());
  }
}
