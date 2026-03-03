#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/ActionLog.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/tools/ReplayDriver.hpp"
#include "../../source/core/AgentBase.hpp"

#include <chrono>
#include <memory>
#include <thread>

TEST_CASE("ReplayDriver replays actions for single agent correctly", "[core]")
{
    // Verifies that ReplayDriver executes actions correctly for one agent
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    // Add agents to the world and store references
    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer.SetLocation(cse498::WorldPosition{3,1});
  
    auto pacerPtr = std::shared_ptr<cse498::AgentBase>(&pacer, [](cse498::AgentBase*){});
    const cse498::WorldPosition curr_position = pacer.GetLocation().AsWorldPosition();

    SECTION("SendAction applies one action using agent_id")
    {
        cse498::ReplayEvent event;
        event.agent_id = pacer.GetID();
        event.actionType = "down";
        event.time = std::chrono::high_resolution_clock::now();

        replayDriver.sendAction(event); //Send action to agent

        REQUIRE(pacer.GetActionResult() == 1);  // Check if doAction is successful
        REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Down()); // Check if agent is in expected position after action
    }

    SECTION("update function replays multiple actions ") 
    {
        actionLog.recordAction(pacerPtr, "down");
        actionLog.recordAction(pacerPtr, "right");

        pacer.SetLocation(curr_position);
        replayDriver.startReplay(actionLog);

        REQUIRE(replayDriver.isRunning());
        REQUIRE(!replayDriver.isFinished());

        // down
        replayDriver.update(); 
        REQUIRE(curr_position.Down() == pacer.GetLocation().AsWorldPosition());

        // right
        replayDriver.update(); 
        REQUIRE(curr_position.Down().Right() == pacer.GetLocation().AsWorldPosition());


        REQUIRE(replayDriver.isFinished());
        REQUIRE(!replayDriver.isRunning());
    }
}

TEST_CASE("ReplayDriver pause/resume functionality", "[core]")
{   
    // Verifies replay pauses correctly and resumes execution properly
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer");
    pacer.SetLocation(cse498::WorldPosition{3, 1});

     auto pacerPtr = std::shared_ptr<cse498::AgentBase>(&pacer, [](cse498::AgentBase*){});
    const auto curr_position = pacer.GetLocation().AsWorldPosition();

    actionLog.recordAction(pacerPtr, "down");
    actionLog.recordAction(pacerPtr, "right");

    pacer.SetLocation(curr_position);
    replayDriver.startReplay(actionLog);

    SECTION("update does nothing during paused and resume continues")
    {
        replayDriver.update();
        REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Down());

        replayDriver.pauseReplay();
        REQUIRE(replayDriver.isPaused());

        replayDriver.update();
        REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Down());

        replayDriver.resumeReplay();
        REQUIRE(!replayDriver.isPaused());
        
        replayDriver.update();
        REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Down().Right());

        REQUIRE(replayDriver.isFinished());
    }
}

TEST_CASE("ReplayDriver replays action in chronological order with multiple agents", "[core]")
{
    // Ensures ReplayDriver executes events in correct chronological order
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

  
  // Add agents to the world and store references
    auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer1.SetLocation(cse498::WorldPosition{3,1});
  
    auto& pacer2 = world.AddAgent<cse498::PacingAgent>("Pacer 2");
    pacer2.SetHorizontal();
    pacer2.SetLocation(cse498::WorldPosition{7,7});

    auto pacer1Ptr = std::shared_ptr<cse498::AgentBase>(&pacer1, [](cse498::AgentBase*){});
    auto pacer2Ptr = std::shared_ptr<cse498::AgentBase>(&pacer2, [](cse498::AgentBase*){});

    const auto p1_curr_position = pacer1.GetLocation().AsWorldPosition();
    const auto p2_curr_position = pacer2.GetLocation().AsWorldPosition();

    actionLog.recordAction(pacer1Ptr, "down");
    std::this_thread::sleep_for(std::chrono::microseconds(50));

    actionLog.recordAction(pacer2Ptr, "left");
    std::this_thread::sleep_for(std::chrono::microseconds(50));

    actionLog.recordAction(pacer1Ptr, "right");
    std::this_thread::sleep_for(std::chrono::microseconds(50));

    actionLog.recordAction(pacer2Ptr, "down");

    pacer1.SetLocation(p1_curr_position);
    pacer2.SetLocation(p2_curr_position);

    replayDriver.startReplay(actionLog);

    replayDriver.update();
    REQUIRE(pacer1.GetLocation().AsWorldPosition() == p1_curr_position.Down());
    REQUIRE(pacer2.GetLocation().AsWorldPosition() == p2_curr_position);

    replayDriver.update();
    REQUIRE(pacer1.GetLocation().AsWorldPosition() == p1_curr_position.Down());
    REQUIRE(pacer2.GetLocation().AsWorldPosition() == p2_curr_position.Left());


    replayDriver.update();
    REQUIRE(pacer1.GetLocation().AsWorldPosition() == p1_curr_position.Down().Right());
    REQUIRE(pacer2.GetLocation().AsWorldPosition() == p2_curr_position.Left());

    replayDriver.update();
    REQUIRE(pacer1.GetLocation().AsWorldPosition() == p1_curr_position.Down().Right());
    REQUIRE(pacer2.GetLocation().AsWorldPosition() == p2_curr_position.Left().Down());

    REQUIRE(replayDriver.isFinished());
    REQUIRE(!replayDriver.isRunning());
}

TEST_CASE("Empty log handling", "[core]")
{
    // Verifies ReplayDriver handles empty logs safely without crashing
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    SECTION("startReplay on empty log")
    {
        replayDriver.startReplay(actionLog);
        REQUIRE(!replayDriver.isRunning());
        REQUIRE(replayDriver.isFinished());
    }

    SECTION("update before startReplay")
    {
        REQUIRE(!replayDriver.isRunning());
        replayDriver.update();
        REQUIRE(!replayDriver.isRunning());
    }
}

TEST_CASE("ReplayDriver resetReplay resets progress but keps event if not cleared", "[core]")
{
    // Ensures resetReplay correctly resets replay state and allows replay again
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer.SetLocation(cse498::WorldPosition{3,1});
    auto pacerPtr = std::shared_ptr<cse498::AgentBase>(&pacer, [](cse498::AgentBase*){});
    const auto curr_position = pacer.GetLocation().AsWorldPosition();

    actionLog.recordAction(pacerPtr, "right");

    pacer.SetLocation(curr_position);
    replayDriver.startReplay(actionLog);
    
    replayDriver.update();
    REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Right());
    REQUIRE(replayDriver.isFinished());

    replayDriver.resetReplay();
    REQUIRE(!replayDriver.isRunning());
    REQUIRE(!replayDriver.isFinished());

    pacer.SetLocation(curr_position);
    replayDriver.startReplay(actionLog);
    replayDriver.update();
    REQUIRE(pacer.GetLocation().AsWorldPosition() == curr_position.Right());
}