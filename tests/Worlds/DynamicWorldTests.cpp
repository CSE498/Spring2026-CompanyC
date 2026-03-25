#include "catch2/catch.hpp"

#include "Worlds/DynamicWorld.hpp"

namespace cse498 {

// Minimal agent that always returns a fixed action ID for controlled testing.
class StubAgent : public AgentBase {
public:
  size_t next_action = 0;

  StubAgent(size_t id, const std::string & name, const WorldBase & world)
    : AgentBase(id, name, world) { }

  size_t SelectAction(const WorldGrid &) override { return next_action; }
};

// Expose protected DoAction and action enum + grid for white-box testing.
class TestDynamicWorld : public DynamicWorld {
public:
  using DynamicWorld::DoAction;
  using DynamicWorld::MOVE_UP;
  using DynamicWorld::MOVE_DOWN;
  using DynamicWorld::MOVE_LEFT;
  using DynamicWorld::MOVE_RIGHT;
  using DynamicWorld::MOVE_UP_LEFT;
  using DynamicWorld::MOVE_UP_RIGHT;
  using DynamicWorld::MOVE_DOWN_LEFT;
  using DynamicWorld::MOVE_DOWN_RIGHT;
  using DynamicWorld::COLLECT;

  // Direct grid write for test setup (bypasses random generation).
  void SetCell(size_t x, size_t y, size_t type_id) {
    mMainGrid(x, y) = type_id;
  }

  size_t GetCell(size_t x, size_t y) const {
    return mMainGrid(x, y);
  }

  size_t GrassId()  const { return mGrassId;  }
  size_t TreeId()   const { return mTreeId;   }
  size_t StoneId()  const { return mStoneId;  }
  size_t WheatId()  const { return mWheatId;  }
  size_t QuarryId() const { return mQuarryId; }

  size_t GetResource(const std::string & name) const {
    auto it = mWorldGlobalCounts.find(name);
    return (it == mWorldGlobalCounts.end()) ? 0 : it->second;
  }

  StubAgent & SpawnStubAt(size_t x, size_t y) {
    auto & agent = AddAgent<StubAgent>("test_agent");
    agent.SetLocation(WorldPosition(x, y));
    return static_cast<StubAgent &>(agent);
  }
};

} // namespace cse498

// -----------------------------------------------------------------------
// Move tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld move - cardinal directions", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  // Place agent in the centre of the 100x100 grid so all moves are in-bounds.
  auto & agent = world.SpawnStubAt(50, 50);

  SECTION("move up decrements y by 1") {
    world.DoAction(agent, world.MOVE_UP);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(50));
    REQUIRE(pos.Y() == Approx(49));
  }

  SECTION("move down increments y by 1") {
    world.DoAction(agent, world.MOVE_DOWN);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(50));
    REQUIRE(pos.Y() == Approx(51));
  }

  SECTION("move left decrements x by 1") {
    world.DoAction(agent, world.MOVE_LEFT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(49));
    REQUIRE(pos.Y() == Approx(50));
  }

  SECTION("move right increments x by 1") {
    world.DoAction(agent, world.MOVE_RIGHT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(51));
    REQUIRE(pos.Y() == Approx(50));
  }
}

TEST_CASE("DynamicWorld move - diagonal directions", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);

  SECTION("move up-left") {
    world.DoAction(agent, world.MOVE_UP_LEFT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(49));
    REQUIRE(pos.Y() == Approx(49));
  }

  SECTION("move up-right") {
    world.DoAction(agent, world.MOVE_UP_RIGHT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(51));
    REQUIRE(pos.Y() == Approx(49));
  }

  SECTION("move down-left") {
    world.DoAction(agent, world.MOVE_DOWN_LEFT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(49));
    REQUIRE(pos.Y() == Approx(51));
  }

  SECTION("move down-right") {
    world.DoAction(agent, world.MOVE_DOWN_RIGHT);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(51));
    REQUIRE(pos.Y() == Approx(51));
  }
}

TEST_CASE("DynamicWorld move - move succeeds and returns true", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);

  int result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result != 0);
}

TEST_CASE("DynamicWorld move - blocked by out-of-bounds", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  // Place agent on the top edge (y=0); moving up should fail.
  auto & agent = world.SpawnStubAt(50, 0);

  int result = world.DoAction(agent, world.MOVE_UP);
  REQUIRE(result == 0);

  // Position must not change.
  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(50));
  REQUIRE(pos.Y() == Approx(0));
}

TEST_CASE("DynamicWorld move - blocked by building tile", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);

  // Place a quarry directly to the right of the agent.
  world.SetCell(51, 50, world.QuarryId());

  int result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result == 0);

  // Agent should not have moved.
  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(50));
  REQUIRE(pos.Y() == Approx(50));
}

TEST_CASE("DynamicWorld move - can move onto resource tiles", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);

  // Place a tree to the right — agents must be able to stand on it to collect.
  world.SetCell(51, 50, world.TreeId());

  int result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result != 0);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(51));
  REQUIRE(pos.Y() == Approx(50));
}

// -----------------------------------------------------------------------
// Collect tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld collect - wood", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.TreeId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("wood") == 1);
  // Tile must become grass after collection.
  REQUIRE(world.GetCell(50, 50) == world.GrassId());
}

TEST_CASE("DynamicWorld collect - stone", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.StoneId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("stone") == 1);
  REQUIRE(world.GetCell(50, 50) == world.GrassId());
}

TEST_CASE("DynamicWorld collect - wheat", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.WheatId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("wheat") == 1);
  REQUIRE(world.GetCell(50, 50) == world.GrassId());
}

TEST_CASE("DynamicWorld collect - fails on grass", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  // Ensure the tile is plain grass (no resource).
  world.SetCell(50, 50, world.GrassId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result == 0);
  REQUIRE(world.GetResource("wood")  == 0);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

TEST_CASE("DynamicWorld collect - multiple collections accumulate", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;

  // Collect wood from three separate tiles using three agents.
  for (size_t i = 0; i < 3; ++i) {
    world.SetCell(10 + i, 10, world.TreeId());
    auto & agent = world.SpawnStubAt(10 + i, 10);
    world.DoAction(agent, world.COLLECT);
  }

  REQUIRE(world.GetResource("wood") == 3);
}
