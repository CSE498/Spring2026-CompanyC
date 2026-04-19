/**
 * @file StubWorldSaveLoadTest.cpp
 * @brief Tests for StubWorld save/load state via Database + WorldHelpers
 */

#include "catch2/catch.hpp"
#include "../../source/Worlds/StubWorld.hpp"
#include "../../source/core/Database.hpp"
#include "../../source/core/WorldHelpers.hpp"

using namespace cse498;

namespace {

// Minimal agent that always returns action 0 (REMAIN_STILL)
class TestStubAgent : public AgentBase {
public:
    TestStubAgent(size_t id, const std::string& name, const WorldBase& world)
        : AgentBase(id, name, world) {}
    size_t SelectAction(WorldGrid&) override { return 0; }
};

} // namespace

TEST_CASE("StubWorld - SaveState writes base and custom fields", "[stubworld-saveload]") {
    Database db;
    StubWorld world;
    world.AddAgent<TestStubAgent>("TestPlayer");
    world.SetDatabase(&db);

    world.SaveState("stub_world");

    // Base fields saved by WorldHelpers
    REQUIRE(db.Exists("world:stub_world:meta"));
    REQUIRE(db.Exists("world:stub_world:grid"));
    REQUIRE(db.Exists("world:stub_world:agent:0"));

    // Custom fields saved by StubWorld
    REQUIRE(db.Exists("world:stub_world:started"));
    REQUIRE(db.Exists("world:stub_world:resources:wood"));
    REQUIRE(db.Exists("world:stub_world:resources:stone"));
    REQUIRE(db.Exists("world:stub_world:resources:wheat"));

    // Verify default values
    auto started = db.Load<bool>("world:stub_world:started");
    REQUIRE(started.has_value());
    REQUIRE(*started == false);

    auto wood = db.Load<int>("world:stub_world:resources:wood");
    REQUIRE(wood.has_value());
    REQUIRE(*wood == 1);

    auto stone = db.Load<int>("world:stub_world:resources:stone");
    REQUIRE(stone.has_value());
    REQUIRE(*stone == 0);
}

TEST_CASE("StubWorld - LoadState restores custom fields", "[stubworld-saveload]") {
    Database db;
    StubWorld world;
    world.AddAgent<TestStubAgent>("TestPlayer");
    world.SetDatabase(&db);

    // Save initial state
    world.SaveState("stub_world");

    // Manually modify DB to simulate a different saved state
    (void)db.Store("world:stub_world:started", true);
    (void)db.Store("world:stub_world:resources:wood", 5);
    (void)db.Store("world:stub_world:resources:stone", 3);
    (void)db.Store("world:stub_world:resources:wheat", 7);

    // Load should restore those values
    world.LoadState("stub_world");

    // Save again to read back values through DB
    Database verify_db;
    world.SetDatabase(&verify_db);
    world.SaveState("stub_world");

    auto wood = verify_db.Load<int>("world:stub_world:resources:wood");
    REQUIRE(wood.has_value());
    REQUIRE(*wood == 5);

    auto stone = verify_db.Load<int>("world:stub_world:resources:stone");
    REQUIRE(stone.has_value());
    REQUIRE(*stone == 3);

    auto wheat = verify_db.Load<int>("world:stub_world:resources:wheat");
    REQUIRE(wheat.has_value());
    REQUIRE(*wheat == 7);

    auto started = verify_db.Load<bool>("world:stub_world:started");
    REQUIRE(started.has_value());
    REQUIRE(*started == true);
}

TEST_CASE("StubWorld - SaveState/LoadState round-trip preserves agent position", "[stubworld-saveload]") {
    Database db;
    StubWorld world;
    auto& agent = world.AddAgent<TestStubAgent>("TestPlayer");
    agent.SetLocation(WorldPosition{3.0, 4.0});
    world.SetDatabase(&db);

    world.SaveState("stub_world");

    // Move agent to a different position
    agent.SetLocation(WorldPosition{1.0, 1.0});

    // Load should restore the saved position
    world.LoadState("stub_world");

    auto loc = agent.GetLocation().AsWorldPosition();
    REQUIRE(loc.X() == Approx(3.0));
    REQUIRE(loc.Y() == Approx(4.0));
}
