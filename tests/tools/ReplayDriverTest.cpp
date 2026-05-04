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
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer.SetLocation(cse498::WorldPosition{3,1});

    const cse498::WorldPosition curr_position = pacer.GetLocation().AsWorldPosition();

    constexpr size_t down = 2;
    constexpr size_t right = 4;

    SECTION("update function replays multiple actions")
    {
        actionLog.recordAction(pacer.GetID(), down, std::chrono::microseconds(0));
        actionLog.recordAction(pacer.GetID(), right, std::chrono::microseconds(50));

        pacer.SetLocation(curr_position);
        replayDriver.startReplay(actionLog);

        REQUIRE(replayDriver.isRunning());
        REQUIRE(!replayDriver.isFinished());

        replayDriver.update();
        REQUIRE(curr_position.Down() == pacer.GetLocation().AsWorldPosition());

        replayDriver.update();
        REQUIRE(curr_position.Down().Right() == pacer.GetLocation().AsWorldPosition());

        REQUIRE(replayDriver.isFinished());
        REQUIRE(!replayDriver.isRunning());
    }
}

TEST_CASE("ReplayDriver pause/resume functionality", "[core]")
{
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer");
    pacer.SetLocation(cse498::WorldPosition{3, 1});
    const auto curr_position = pacer.GetLocation().AsWorldPosition();

    constexpr size_t down = 2;
    constexpr size_t right = 4;

    actionLog.recordAction(pacer.GetID(), down, std::chrono::microseconds(0));
    actionLog.recordAction(pacer.GetID(), right, std::chrono::microseconds(50));

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
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer1 = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer1.SetLocation(cse498::WorldPosition{3,1});

    auto& pacer2 = world.AddAgent<cse498::PacingAgent>("Pacer 2");
    pacer2.SetHorizontal();
    pacer2.SetLocation(cse498::WorldPosition{7,7});

    constexpr size_t down = 2;
    constexpr size_t left = 3;
    constexpr size_t right = 4;

    const auto p1_curr_position = pacer1.GetLocation().AsWorldPosition();
    const auto p2_curr_position = pacer2.GetLocation().AsWorldPosition();

    actionLog.recordAction(pacer1.GetID(), down, std::chrono::microseconds(0));
    actionLog.recordAction(pacer2.GetID(), left, std::chrono::microseconds(50));
    actionLog.recordAction(pacer1.GetID(), right, std::chrono::microseconds(100));
    actionLog.recordAction(pacer2.GetID(), down, std::chrono::microseconds(150));

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

TEST_CASE("ReplayDriver resetReplay resets progress but keeps event if not cleared", "[core]")
{
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    pacer.SetLocation(cse498::WorldPosition{3,1});
    const auto curr_position = pacer.GetLocation().AsWorldPosition();

    constexpr size_t right = 4;

    actionLog.recordAction(pacer.GetID(), right, std::chrono::microseconds(0));

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

TEST_CASE("ReplayDriver handles invalid actionType safely", "[core]")
{
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");
    const auto start = pacer.GetLocation().AsWorldPosition();

    constexpr size_t invalidAction = 999;

    actionLog.recordAction(pacer.GetID(), invalidAction, std::chrono::microseconds(0));

    replayDriver.startReplay(actionLog);
    replayDriver.update();

    REQUIRE(pacer.GetLocation().AsWorldPosition() == start);
}

TEST_CASE("ReplayDriver clearReplay() functionality", "[core]")
{
    cse498::ActionLog actionLog;
    cse498::MazeWorld world;
    cse498::ReplayDriver replayDriver(world);

    auto& pacer = world.AddAgent<cse498::PacingAgent>("Pacer 1");

    constexpr size_t right = 4;
    constexpr size_t down = 2;

    actionLog.recordAction(pacer.GetID(), right, std::chrono::microseconds(0));
    actionLog.recordAction(pacer.GetID(), down, std::chrono::microseconds(50));

    replayDriver.startReplay(actionLog);
    replayDriver.update();
    replayDriver.update();

    replayDriver.clearReplay();

    REQUIRE(replayDriver.getEvents().empty());
    REQUIRE_FALSE(replayDriver.isRunning());
    REQUIRE_FALSE(replayDriver.isPaused());
    REQUIRE(replayDriver.isFinished());
}