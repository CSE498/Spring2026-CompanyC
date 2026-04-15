#include "catch2/catch.hpp"

#include "Worlds/DynamicWorld.hpp"

namespace cse498 {

// Minimal agent that always returns a fixed action ID for controlled testing.
class StubAgent : public AgentBase {
public:
  size_t next_action = 0;

  StubAgent(size_t id, const std::string & name, const WorldBase & world)
    : AgentBase(id, name, world) { }

  size_t SelectAction(WorldGrid &) override { return next_action; }
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
  using DynamicWorld::BUILD_LUMBERYARD;
  using DynamicWorld::BUILD_QUARRY;
  using DynamicWorld::BUILD_SPAWNER;
  using DynamicWorld::BUILD_FARM;
  using DynamicWorld::BUILD_TOWNHALL;

  // Expose building ids so tests can verify the correct structure was placed
  size_t LumberyardId() const { return GetLumberyardId(); }
  size_t FarmId()       const { return GetFarmId(); }
  size_t SpawnerId()    const { return GetSpawnerId(); }
  size_t TownhallId()   const { return GetTownhallId(); }
  size_t GrassId()      const { return GetGrassId(); }
  size_t TreeId()       const { return GetTreeId(); }
  size_t StoneId()      const { return GetStoneId(); }
  size_t WheatId()      const { return GetWheatId(); }
  size_t QuarryId()     const { return GetQuarryId(); }

  void SetResource(const std::string & name, size_t value) {
    mWorldResourceCounts[ResourceIndex(name)] = value;
  }
  

  // Direct grid write for test setup (bypasses random generation).
  void SetCell(size_t x, size_t y, size_t type_id) {
    main_grid[x, y] = type_id;
  }

  size_t GetCell(size_t x, size_t y) const {
    return main_grid[x, y];
  }

  size_t GetResource(const std::string & name) const {
    return mWorldResourceCounts[ResourceIndex(name)];
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
// -----------------------------------------------------------------------
// BuildX tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);

  // Start on a valid buildable tile
  world.SetCell(50, 50, world.GrassId());

  // Give the exact resources needed for this building
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);

  // Build should succeed, place the structure and deduct resources
  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.LumberyardId());
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("steel") == 0);
}

TEST_CASE("DynamicWorld build lumberyard - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  // Leave wood one short so the build fails
  world.SetResource("wood", 19);
  world.SetResource("steel", 20);

  // Build should fail and leave the tile/resources unchanged
  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GrassId());
  REQUIRE(world.GetResource("wood") == 19);
  REQUIRE(world.GetResource("steel") == 20);
}

TEST_CASE("DynamicWorld build quarry - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.QuarryId());
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wood") == 0);
}

TEST_CASE("DynamicWorld build quarry - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.TreeId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.TreeId());
  REQUIRE(world.GetResource("stone") == 20);
  REQUIRE(world.GetResource("wood") == 20);
}

TEST_CASE("DynamicWorld build spawner - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.SpawnerId());
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

TEST_CASE("DynamicWorld build farm - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.FarmId());
  REQUIRE(world.GetResource("wheat") == 0);
  REQUIRE(world.GetResource("wood") == 0);
}

TEST_CASE("DynamicWorld build townhall - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 500);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.TownhallId());
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

TEST_CASE("DynamicWorld build townhall - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 499);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GrassId());
  REQUIRE(world.GetResource("wood") == 500);
  REQUIRE(world.GetResource("stone") == 500);
  REQUIRE(world.GetResource("steel") == 499);
  REQUIRE(world.GetResource("wheat") == 500);
}
