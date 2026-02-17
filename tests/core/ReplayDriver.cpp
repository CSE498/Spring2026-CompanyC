#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/ActionLog.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/tools/ReplayDriver.hpp"
#include "../../source/core/AgentBase.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <thread>

TEST_CASE("Test replay driver functionality", "[core]")
{
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    // Add agents to the world and store references
    auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer1.SetLocation(cse498::WorldPosition{3,1});
  
    auto& pacer2 = world.AddAgent<cse498::PacingAgent>("Pacer 2");
    pacer2.SetHorizontal();
    pacer2.SetLocation(cse498::WorldPosition{7,7});

    SECTION("Check if sending action manually to the agent works correctly")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});    // Create shared pointer for agent to record action in log and create replay event
        cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition(); // Get current position of agent
        actionLog.recordAction(pacer1Ptr, "down"); // Record action in log for replay

        // Create replay event for move down action
        ReplayEvent event;
        event.agent = pacer1Ptr;
        event.actionType = "down";

        replayDriver.sendAction(event); //Send action to agent
        REQUIRE(pacer1Ptr->GetActionResult() == 1);  // Check if doAction is successful
        REQUIRE(pacer1Ptr->GetLocation().AsWorldPosition() == cur_position.Down()); // Check if agent is in expected position after action
        replayDriver.clearReplay(); // Clear events for next test
    }

    SECTION("Check if update sends actions correctly to the agent") 
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});

        // Record action down in log for replay
        cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(cur_position.Down());
        actionLog.recordAction(pacer1Ptr, "down");

        // Record action right in log for replay
        cse498::WorldPosition new_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(new_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        // Event for move down action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "down";

        // Event for move right action
        ReplayEvent event2;
        event2.agent = pacer1Ptr;
        event2.actionType = "right";

        event1.agent->SetLocation(cur_position); // Reset position to test replay

        replayDriver.startReplay(actionLog); // Start replay with the recorded actions

        replayDriver.update(); // Update to send recorded actions to the agent

        REQUIRE(pacer1Ptr->GetActionResult() == 1); // Check if event 2 action was successful
        REQUIRE(cur_position.Down().Right() == pacer1Ptr->GetLocation().AsWorldPosition()); // Check if agent is in expected position after actions
        replayDriver.clearReplay(); // Clear events for next test
    }
    SECTION("Check if update sends actions to different agents correctly")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});
        auto pacer2Ptr = std::shared_ptr<cse498::AgentBase>(&pacer2, [](cse498::AgentBase*){}); // Add second agent to test replay with multiple agents

        cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(cur_position.Down());
        actionLog.recordAction(pacer1Ptr, "down");

        cse498::WorldPosition cur_position2 = pacer2.GetLocation().AsWorldPosition();
        pacer2.SetLocation(cur_position2.Left());
        actionLog.recordAction(pacer2Ptr, "left");

        cse498::WorldPosition new_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(new_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        cse498::WorldPosition new_position2 = pacer2.GetLocation().AsWorldPosition();
        pacer2.SetLocation(new_position2.Down());
        actionLog.recordAction(pacer2Ptr, "down");

        // Event for move down action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "down";

        // Event for move left action
        ReplayEvent event2;
        event2.agent = pacer2Ptr;
        event2.actionType = "left";

        // Event for move right action
        ReplayEvent event3;
        event3.agent = pacer1Ptr;
        event3.actionType = "right";

        // Event for move down action
        ReplayEvent event4;
        event4.agent = pacer2Ptr;
        event4.actionType = "down";

        // Reset positions to test replay
        event1.agent->SetLocation(cur_position);
        event2.agent->SetLocation(cur_position2);

        replayDriver.startReplay(actionLog); // Start replay with the recorded actions

        replayDriver.update(); // Update to send actions to agents

        REQUIRE(pacer1Ptr->GetActionResult() == 1); // Check if pacer1 move down action was successful
        REQUIRE(pacer2Ptr->GetActionResult() == 1); // Check if pac
        REQUIRE(cur_position.Down().Right() == pacer1Ptr->GetLocation().AsWorldPosition()); // Check if pacer1 is in expected position after actions
        REQUIRE(cur_position2.Left().Down() == pacer2Ptr->GetLocation().AsWorldPosition()); // Check if pacer2 is in expected position after actions
        replayDriver.clearReplay(); // Clear events for next test
    }
    SECTION("Check if pause and resume replay work correctly")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});

        cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(cur_position.Down());
        actionLog.recordAction(pacer1Ptr, "down");

        cse498::WorldPosition new_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(new_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        // Event for move up action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "down";

        // Event for move right action
        ReplayEvent event2;
        event2.agent = pacer1Ptr;
        event2.actionType = "right";

        event1.agent->SetLocation(cur_position); // Reset position to test replay

        replayDriver.startReplay(actionLog); // Start replay with the recorded actions

        REQUIRE(replayDriver.isRunning() == true); // Check if replay is running
        REQUIRE(replayDriver.isPaused() == false); // Check if replay is not paused

        replayDriver.pauseReplay(); // Pause replay

        REQUIRE(replayDriver.isPaused() == true); // Check if replay is paused

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        replayDriver.resumeReplay(); // Resume replay

        replayDriver.update(); // Update to send actions to agent

        REQUIRE(pacer1Ptr->GetActionResult() == 1); // Check if event 2 action was successful
        REQUIRE(cur_position.Down().Right() == pacer1Ptr->GetLocation().AsWorldPosition()); // Check if agent is in expected position after actions
        replayDriver.clearReplay(); // Clear events for next test
    }
    SECTION("Check if replay finishes correctly")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});

        cse498::WorldPosition new_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(new_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        // Event for move up action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "right";

        replayDriver.startReplay(actionLog); // Start replay with the recorded action

        replayDriver.update(); // Update to send action to agent

        REQUIRE(replayDriver.isFinished() == true); // Check if replay is finished after all actions are sent
    }
    SECTION("Check if starting replay with empty log does nothing")
    {
        replayDriver.startReplay(actionLog); // Start replay with empty log

        REQUIRE(replayDriver.isRunning() == false); // Check is replay is not running
        REQUIRE(replayDriver.isFinished() == true); // Check is replay is finished
    }
    SECTION("Check if starting replay with no actions does nothing")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});

        cse498::WorldPosition new_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(new_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        // Event for move up action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "right";

        actionLog.clear(); // Clear actions from log to test starting replay with no actions
        REQUIRE(actionLog.getActions().empty()); // Check that log is empty

        replayDriver.startReplay(actionLog); // Start replay with log that has no actions

        replayDriver.update(); // Update to attempt to send actions to agent

        REQUIRE(replayDriver.isRunning() == false); // Check is replay is not running since there are no actions to send
        REQUIRE(replayDriver.isFinished() == true); // Check is replay is finished since there are no actions to send
    }
    SECTION("Check if resetting replay works correctly")
    {
        auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});

        cse498::WorldPosition cur_position = pacer1.GetLocation().AsWorldPosition();
        pacer1.SetLocation(cur_position.Right());
        actionLog.recordAction(pacer1Ptr, "right");

        // Event for move up action
        ReplayEvent event1;
        event1.agent = pacer1Ptr;
        event1.actionType = "right";

        replayDriver.startReplay(actionLog); // Start replay with the recorded action

        replayDriver.update(); // Update to send action to agent

        replayDriver.resetReplay(); // Reset replay to test if replay can be started again after resetting


        REQUIRE(replayDriver.isRunning() == false); // Check if replay is not running after resetting
        REQUIRE(replayDriver.isFinished() == false); //check if replay is not finished after resetting
 
        pacer1.SetLocation(cur_position); // Reset position to test replay again

        replayDriver.startReplay(actionLog); // Start replay again with the same recorded action after resetting

        replayDriver.update(); // Update to send action to agent

        REQUIRE(cur_position.Right() == pacer1Ptr->GetLocation().AsWorldPosition()); // Check if agent is in expected position after action is sent again after resetting

        replayDriver.clearReplay(); // Clear events for next test
    }
}
