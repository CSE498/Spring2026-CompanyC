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

  void SetAgentLocation(size_t x, size_t y) { SetLocation(WorldPosition(x, y)); }
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

  void SetResource(const std::string & name, size_t value) {
    mWorldResourceCounts[ResourceIndex(name)] = value;
  }
  

  // Direct grid write for test setup (bypasses random generation).
  void SetCell(size_t x, size_t y, size_t type_id) {
    main_grid[x, y] = type_id;
  }

  [[nodiscard]] size_t GetCell(size_t x, size_t y) const {
    return main_grid[x, y];
  }

  [[nodiscard]] size_t GetResource(const std::string & name) const {
    return mWorldResourceCounts[ResourceIndex(name)];
  }

  StubAgent & SpawnStubAt(size_t x, size_t y, std::string agent_name) {
    auto & agent = AddAgent<StubAgent>(agent_name);
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
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

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
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

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
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  int result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result != 0);
}

TEST_CASE("DynamicWorld move - blocked by out-of-bounds", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  // Place agent on the top edge (y=0); moving up should fail.
  auto & agent = world.SpawnStubAt(50, 0, "test_agent");

  int result = world.DoAction(agent, world.MOVE_UP);
  REQUIRE(result == 0);

  // Position must not change.
  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(50));
  REQUIRE(pos.Y() == Approx(0));

  // setting the location of the agent to be on the left most corner
  agent.SetAgentLocation(0, 50);
  // trying to move beyond the left boundary of the grid
  result = world.DoAction(agent, world.MOVE_LEFT);
  REQUIRE(result == 0);

  // position should not change
  pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(0));
  REQUIRE(pos.Y() == Approx(50));

  // changing the location of the agent to be the right most corner of the grid
  agent.SetAgentLocation(80, 80);
  // trying to move the agent down, should fail
  result = world.DoAction(agent, world.MOVE_DOWN);
  REQUIRE(result == 0);

  // position should not change
  pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(80));
  REQUIRE(pos.Y() == Approx(80));

  // setting the location of the agent to be the right most corner of the grid
  agent.SetAgentLocation(80, 80);
  // trying to move past the right boundary, should fail
  result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result == 0);

  // position should not change
  pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(80));
  REQUIRE(pos.Y() == Approx(80));
}

TEST_CASE("DynamicWorld move - can move onto resource tiles", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  // Place a tree to the right — agents must be able to stand on it to collect.
  world.SetCell(51, 50, world.GetTreeId());

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
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");
  world.SetCell(50, 50, world.GetTreeId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("wood") == 1);
  // Tile must become grass after collection.
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld collect - stone", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");
  world.SetCell(50, 50, world.GetStoneId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("stone") == 1);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld collect - wheat", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");
  world.SetCell(50, 50, world.GetWheatId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result != 0);
  REQUIRE(world.GetResource("wheat") == 1);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld collect - fails on grass", "[DynamicWorld][collect]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");
  // Ensure the tile is plain grass (no resource).
  world.SetCell(50, 50, world.GetGrassId());

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
    world.SetCell(10 + i, 10, world.GetTreeId());
    auto & agent = world.SpawnStubAt(10 + i, 10, "test_agent");
    world.DoAction(agent, world.COLLECT);
  }

  REQUIRE(world.GetResource("wood") == 3);
}
// -----------------------------------------------------------------------
// BuildX tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  // Start on a valid buildable tile
  world.SetCell(50, 50, world.GetGrassId());

  // Give the exact resources needed for this building
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);

  // Build should succeed, place the structure and deduct resources
  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.GetLumberyardId());
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("steel") == 0);
}

TEST_CASE("DynamicWorld build lumberyard - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Leave wood one short so the build fails
  world.SetResource("wood", 19);
  world.SetResource("steel", 20);

  // Build should fail and leave the tile/resources unchanged
  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wood") == 19);
  REQUIRE(world.GetResource("steel") == 20);
}

TEST_CASE("DynamicWorld build quarry - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.GetQuarryId());
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wood") == 0);
}

TEST_CASE("DynamicWorld build quarry - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetTreeId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTreeId());
  REQUIRE(world.GetResource("stone") == 20);
  REQUIRE(world.GetResource("wood") == 20);
}

TEST_CASE("DynamicWorld build spawner - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.GetSpawnerId());
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

TEST_CASE("DynamicWorld build farm - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.GetFarmId());
  REQUIRE(world.GetResource("wheat") == 0);
  REQUIRE(world.GetResource("wood") == 0);
}

TEST_CASE("DynamicWorld build townhall - succeeds", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 500);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result != 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTownhallId());
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

TEST_CASE("DynamicWorld build townhall - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 499);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wood") == 500);
  REQUIRE(world.GetResource("stone") == 500);
  REQUIRE(world.GetResource("steel") == 499);
  REQUIRE(world.GetResource("wheat") == 500);
}

// -----------------------------------------------------------------------
// Move tests - blocked by all building tiles
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld move - blocked by every building tile", "[DynamicWorld][move]") {
  cse498::TestDynamicWorld world;

  struct BuildingCase {
    const char * name;
    size_t id;
  };

  const std::vector<BuildingCase> building_cases = {
    {"quarry",      world.GetQuarryId()},
    {"lumberyard",  world.GetLumberyardId()},
    {"farm",        world.GetFarmId()},
    {"spawner",     world.GetSpawnerId()},
    {"townhall",    world.GetTownhallId()}
  };

  for (const auto & building : building_cases) {
    SECTION(building.name) {
      auto & agent = world.SpawnStubAt(50, 50, std::string("agent_") + building.name);
      world.SetCell(51, 50, building.id);

      int result = world.DoAction(agent, world.MOVE_RIGHT);
      REQUIRE(result == 0);

      auto pos = agent.GetLocation().AsWorldPosition();
      REQUIRE(pos.X() == Approx(50));
      REQUIRE(pos.Y() == Approx(50));
    }
  }
}

// -----------------------------------------------------------------------
// Build failure tests - all buildings
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetTreeId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);

  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTreeId());
  REQUIRE(world.GetResource("wood") == 20);
  REQUIRE(world.GetResource("steel") == 20);
}

TEST_CASE("DynamicWorld build quarry - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("stone", 19);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 19);
  REQUIRE(world.GetResource("wood") == 20);
}

TEST_CASE("DynamicWorld build spawner - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("stone", 29);
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 29);
  REQUIRE(world.GetResource("wheat") == 30);
}

TEST_CASE("DynamicWorld build spawner - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetStoneId());
  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetStoneId());
  REQUIRE(world.GetResource("stone") == 30);
  REQUIRE(world.GetResource("wheat") == 30);
}

TEST_CASE("DynamicWorld build farm - fails without enough resources", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("wheat", 19);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wheat") == 19);
  REQUIRE(world.GetResource("wood") == 20);
}

TEST_CASE("DynamicWorld build farm - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetWheatId());
  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetWheatId());
  REQUIRE(world.GetResource("wheat") == 20);
  REQUIRE(world.GetResource("wood") == 20);
}

TEST_CASE("DynamicWorld build townhall - fails on non-grass tile", "[DynamicWorld][build]") {
  cse498::TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetTreeId());
  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 500);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTreeId());
  REQUIRE(world.GetResource("wood") == 500);
  REQUIRE(world.GetResource("stone") == 500);
  REQUIRE(world.GetResource("steel") == 500);
  REQUIRE(world.GetResource("wheat") == 500);
}

// -----------------------------------------------------------------------
// Tick / production tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld tick - lumberyard produces wood over time", "[DynamicWorld][tick][production]") {
  cse498::TestDynamicWorld world;
  auto & builder = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);

  REQUIRE(world.DoAction(builder, world.BUILD_LUMBERYARD) != 0);
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  for (size_t i = 0; i < 20; ++i) {
    world.Tick();
  }

  REQUIRE(world.GetResource("wood") == 1);

  for (size_t i = 0; i < 20; ++i) {
    world.Tick();
  }

  REQUIRE(world.GetResource("wood") == 2);
}

TEST_CASE("DynamicWorld tick - farm produces wheat over time", "[DynamicWorld][tick][production]") {
  cse498::TestDynamicWorld world;
  auto & builder = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);

  REQUIRE(world.DoAction(builder, world.BUILD_FARM) != 0);
  REQUIRE(world.GetResource("wheat") == 0);
  REQUIRE(world.GetResource("wood") == 0);

  for (size_t i = 0; i < 10; ++i) {
    world.Tick();
  }

  REQUIRE(world.GetResource("wheat") == 1);

  for (size_t i = 0; i < 10; ++i) {
    world.Tick();
  }

  REQUIRE(world.GetResource("wheat") == 2);
}

TEST_CASE("DynamicWorld tick - quarry produces stone and steel over time", "[DynamicWorld][tick][production]") {
  cse498::TestDynamicWorld world;
  auto & builder = world.SpawnStubAt(50, 50, "builder");

  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("stone", 20);
  world.SetResource("wood", 20);

  REQUIRE(world.DoAction(builder, world.BUILD_QUARRY) != 0);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wood") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  for (size_t i = 0; i < 10; ++i) {
    world.Tick();
  }
  REQUIRE(world.GetResource("stone") == 1);
  REQUIRE(world.GetResource("steel") == 1);

  for (size_t i = 0; i < 10; ++i) {
    world.Tick();
  }
  REQUIRE(world.GetResource("stone") == 2);
  REQUIRE(world.GetResource("steel") == 2);

  for (size_t i = 0; i < 20; ++i) {
    world.Tick();
  }
  REQUIRE(world.GetResource("stone") == 4);
  REQUIRE(world.GetResource("steel") == 4);
}