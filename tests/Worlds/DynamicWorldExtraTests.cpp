/**
 * @file DynamicWorldExtraTests.cpp
 * @brief Additional Catch2 tests for DynamicWorld covering:
 *   - REMAIN_STILL action
 *   - Missing build-failure cases (insufficient resources, non-grass tiles)
 *   - UpdateWorld resource production (lumberyard, quarry, farm)
 *   - UpdateWorld spawner agent creation
 *   - Townhall win condition (run_over flag)
 *   - GetTickCount tracking
 *   - Collect on a building tile
 *   - Diagonal moves at world corners
 */

#include "catch2/catch.hpp"
#include "Worlds/DynamicWorld.hpp"

// Use an anonymous namespace so these helpers don't collide with the
// identically-named helpers defined in DynamicWorldTest.cpp.
namespace {

class StubAgent : public cse498::AgentBase {
public:
  size_t next_action = 0;

  StubAgent(size_t id, const std::string & name, const cse498::WorldBase & world)
    : AgentBase(id, name, world) {}

  size_t SelectAction(cse498::WorldGrid &) override { return next_action; }
};

class TestDynamicWorld : public cse498::DynamicWorld {
public:
  // Expose protected action + DoAction for white-box testing.
  using cse498::DynamicWorld::DoAction;
  using cse498::DynamicWorld::REMAIN_STILL;
  using cse498::DynamicWorld::MOVE_UP;
  using cse498::DynamicWorld::MOVE_DOWN;
  using cse498::DynamicWorld::MOVE_LEFT;
  using cse498::DynamicWorld::MOVE_RIGHT;
  using cse498::DynamicWorld::MOVE_UP_LEFT;
  using cse498::DynamicWorld::MOVE_UP_RIGHT;
  using cse498::DynamicWorld::MOVE_DOWN_LEFT;
  using cse498::DynamicWorld::MOVE_DOWN_RIGHT;
  using cse498::DynamicWorld::COLLECT;
  using cse498::DynamicWorld::BUILD_LUMBERYARD;
  using cse498::DynamicWorld::BUILD_QUARRY;
  using cse498::DynamicWorld::BUILD_SPAWNER;
  using cse498::DynamicWorld::BUILD_FARM;
  using cse498::DynamicWorld::BUILD_TOWNHALL;
  using cse498::DynamicWorld::UpdateWorld;

  void SetResource(const std::string & name, size_t value) {
    mWorldResourceCounts[ResourceIndex(name)] = value;
  }

  size_t GetResource(const std::string & name) const {
    return mWorldResourceCounts[ResourceIndex(name)];
  }

  void SetCell(size_t x, size_t y, size_t type_id) {
    main_grid[x, y] = type_id;
  }

  size_t GetCell(size_t x, size_t y) const {
    return main_grid[x, y];
  }

  bool IsRunOver() const { return run_over; }

  StubAgent & SpawnStubAt(size_t x, size_t y, const std::string & name) {
    auto & agent = AddAgent<StubAgent>(name);
    agent.SetLocation(cse498::WorldPosition(x, y));
    return static_cast<StubAgent &>(agent);
  }

  // Call UpdateWorld N times with all stub agents staying still.
  void RunNTicks(size_t n) {
    for (size_t i = 0; i < n; ++i) UpdateWorld();
  }
};

} // anonymous namespace

// -----------------------------------------------------------------------
// REMAIN_STILL tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld REMAIN_STILL - agent does not move", "[DynamicWorld][remain_still]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  world.DoAction(agent, world.REMAIN_STILL);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(50));
  REQUIRE(pos.Y() == Approx(50));
}

TEST_CASE("DynamicWorld REMAIN_STILL - returns success", "[DynamicWorld][remain_still]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  int result = world.DoAction(agent, world.REMAIN_STILL);

  REQUIRE(result != 0);
}

// -----------------------------------------------------------------------
// Collect on building tile
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld collect - fails on building tile", "[DynamicWorld][collect]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  // Place a lumberyard under the agent (non-collectible structure).
  // Note: building tiles are non-traversable, so we use SetCell directly.
  world.SetCell(50, 50, world.GetLumberyardId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result == 0);
  REQUIRE(world.GetResource("wood")  == 0);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("wheat") == 0);
}

// -----------------------------------------------------------------------
// Diagonal move boundary tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld move - diagonal blocked at top-left corner", "[DynamicWorld][move]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(0, 0, "test_agent");

  SECTION("up-left at (0,0) is blocked") {
    int result = world.DoAction(agent, world.MOVE_UP_LEFT);
    REQUIRE(result == 0);
    auto pos = agent.GetLocation().AsWorldPosition();
    REQUIRE(pos.X() == Approx(0));
    REQUIRE(pos.Y() == Approx(0));
  }

  SECTION("up at (0,0) is blocked") {
    int result = world.DoAction(agent, world.MOVE_UP);
    REQUIRE(result == 0);
  }

  SECTION("left at (0,0) is blocked") {
    int result = world.DoAction(agent, world.MOVE_LEFT);
    REQUIRE(result == 0);
  }
}

// -----------------------------------------------------------------------
// Build failures - missing resource cases
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - fails when only steel is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 20);
  world.SetResource("steel", 19);  // one short

  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wood")  == 20);
  REQUIRE(world.GetResource("steel") == 19);
}

TEST_CASE("DynamicWorld build lumberyard - fails on non-grass tile", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetTreeId());

  world.SetResource("wood", 20);
  world.SetResource("steel", 20);

  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTreeId());
  REQUIRE(world.GetResource("wood")  == 20);
  REQUIRE(world.GetResource("steel") == 20);
}

TEST_CASE("DynamicWorld build quarry - fails when only stone is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 19);  // one short
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 19);
  REQUIRE(world.GetResource("wood")  == 20);
}

TEST_CASE("DynamicWorld build quarry - fails when only wood is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 19);  // one short

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld build spawner - fails when only stone is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 29);  // one short
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 29);
  REQUIRE(world.GetResource("wheat") == 30);
}

TEST_CASE("DynamicWorld build spawner - fails when only wheat is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 29);  // one short

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld build spawner - fails on non-grass tile", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetStoneId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetStoneId());
}

TEST_CASE("DynamicWorld build farm - fails when only wheat is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 19);  // one short
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wheat") == 19);
  REQUIRE(world.GetResource("wood")  == 20);
}

TEST_CASE("DynamicWorld build farm - fails when only wood is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 19);  // one short

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld build farm - fails on non-grass tile", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetWheatId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetWheatId());
}

TEST_CASE("DynamicWorld build townhall - fails on non-grass tile", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetTreeId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 500);
  world.SetResource("wheat", 500);

  int result = world.DoAction(agent, world.BUILD_TOWNHALL);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetTreeId());
  // Resources must remain untouched on failure.
  REQUIRE(world.GetResource("wood")  == 500);
  REQUIRE(world.GetResource("stone") == 500);
  REQUIRE(world.GetResource("steel") == 500);
  REQUIRE(world.GetResource("wheat") == 500);
}

// -----------------------------------------------------------------------
// Townhall win condition
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build townhall - sets run_over flag", "[DynamicWorld][build][win]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 500);
  world.SetResource("stone", 500);
  world.SetResource("steel", 500);
  world.SetResource("wheat", 500);

  REQUIRE(!world.IsRunOver());
  world.DoAction(agent, world.BUILD_TOWNHALL);
  REQUIRE(world.IsRunOver());
}

// -----------------------------------------------------------------------
// GetTickCount tracking
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld GetTickCount - starts at zero", "[DynamicWorld][tick]") {
  TestDynamicWorld world;
  REQUIRE(world.GetTickCount() == 0);
}

TEST_CASE("DynamicWorld GetTickCount - increments each UpdateWorld call", "[DynamicWorld][tick]") {
  TestDynamicWorld world;

  world.UpdateWorld();
  REQUIRE(world.GetTickCount() == 1);

  world.UpdateWorld();
  REQUIRE(world.GetTickCount() == 2);

  world.RunNTicks(8);
  REQUIRE(world.GetTickCount() == 10);
}

// -----------------------------------------------------------------------
// UpdateWorld - building resource production
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld UpdateWorld - lumberyard produces wood every 20 ticks", "[DynamicWorld][update][lumberyard]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(agent, world.BUILD_LUMBERYARD);

  // Resources spent on construction — now 0.
  REQUIRE(world.GetResource("wood") == 0);

  // Tick 1–19: no production yet.
  world.RunNTicks(19);
  REQUIRE(world.GetResource("wood") == 0);

  // Tick 20: first production.
  world.UpdateWorld();
  REQUIRE(world.GetResource("wood") == 1);

  // Tick 21–39: no additional production.
  world.RunNTicks(19);
  REQUIRE(world.GetResource("wood") == 1);

  // Tick 40: second production.
  world.UpdateWorld();
  REQUIRE(world.GetResource("wood") == 2);
}

TEST_CASE("DynamicWorld UpdateWorld - quarry produces stone and steel every 10 ticks", "[DynamicWorld][update][quarry]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);
  world.DoAction(agent, world.BUILD_QUARRY);

  // Resources spent — now 0.
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  // Tick 1–9: no production.
  world.RunNTicks(9);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  // Tick 10: first production of both stone and steel.
  world.UpdateWorld();
  REQUIRE(world.GetResource("stone") == 1);
  REQUIRE(world.GetResource("steel") == 1);

  // Tick 20: second production.
  world.RunNTicks(9);
  world.UpdateWorld();
  REQUIRE(world.GetResource("stone") == 2);
  REQUIRE(world.GetResource("steel") == 2);
}

TEST_CASE("DynamicWorld UpdateWorld - farm produces wheat every 10 ticks", "[DynamicWorld][update][farm]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);
  world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(world.GetResource("wheat") == 0);

  // Tick 1–9: no production.
  world.RunNTicks(9);
  REQUIRE(world.GetResource("wheat") == 0);

  // Tick 10: first production.
  world.UpdateWorld();
  REQUIRE(world.GetResource("wheat") == 1);

  // Tick 20: second production.
  world.RunNTicks(9);
  world.UpdateWorld();
  REQUIRE(world.GetResource("wheat") == 2);
}

TEST_CASE("DynamicWorld UpdateWorld - two lumberyards double production rate", "[DynamicWorld][update][lumberyard]") {
  TestDynamicWorld world;

  // Build first lumberyard at (40, 40).
  auto & builder1 = world.SpawnStubAt(40, 40, "builder");
  world.SetCell(40, 40, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(builder1, world.BUILD_LUMBERYARD);

  // Build second lumberyard at (60, 60).
  auto & builder2 = world.SpawnStubAt(60, 60, "builder");
  world.SetCell(60, 60, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(builder2, world.BUILD_LUMBERYARD);

  // After 20 ticks both should have fired once.
  world.RunNTicks(20);
  REQUIRE(world.GetResource("wood") == 2);
}

// -----------------------------------------------------------------------
// UpdateWorld - spawner creates new agents
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld UpdateWorld - spawner creates an agent at tick 60", "[DynamicWorld][update][spawner]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Clear a grass cell next to the spawner so the agent has somewhere to appear.
  world.SetCell(51, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);
  world.DoAction(agent, world.BUILD_SPAWNER);

  size_t agents_before = world.GetNumAgents();

  // Tick 1–59: no spawn yet.
  world.RunNTicks(59);
  REQUIRE(world.GetNumAgents() == agents_before);

  // Tick 60: spawner fires.
  world.UpdateWorld();
  REQUIRE(world.GetNumAgents() == agents_before + 1);
}

TEST_CASE("DynamicWorld UpdateWorld - spawner creates a second agent at tick 120", "[DynamicWorld][update][spawner]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());
  world.SetCell(51, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);
  world.DoAction(agent, world.BUILD_SPAWNER);

  size_t agents_before = world.GetNumAgents();

  world.RunNTicks(60);
  REQUIRE(world.GetNumAgents() == agents_before + 1);

  world.RunNTicks(60);
  REQUIRE(world.GetNumAgents() == agents_before + 2);
}
