/**
 * @file DynamicWorldSequenceTests.cpp
 * @brief Catch2 tests for DynamicWorld covering:
 *   - Cell type ID uniqueness
 *   - ResourceIndex mapping
 *   - Sequential move-then-collect
 *   - Two agents racing to collect the same tile
 *   - Building on top of an existing building
 *   - Unknown action ID
 *   - Exact resource deduction with surplus
 *   - Custom world size constructor
 *   - Mixed building production (lumberyard + farm simultaneously)
 */

#include "catch2/catch.hpp"
#include "Worlds/DynamicWorld.hpp"

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

  // Use the custom-size constructor for size tests.
  TestDynamicWorld(size_t w, size_t h) : cse498::DynamicWorld(w, h) {}
  TestDynamicWorld() : cse498::DynamicWorld() {}

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

  size_t GridWidth()  const { return main_grid.GetWidth(); }
  size_t GridHeight() const { return main_grid.GetHeight(); }

  StubAgent & SpawnStubAt(size_t x, size_t y, const std::string & name) {
    auto & agent = AddAgent<StubAgent>(name);
    agent.SetLocation(cse498::WorldPosition(x, y));
    return static_cast<StubAgent &>(agent);
  }

  void RunNTicks(size_t n) {
    for (size_t i = 0; i < n; ++i) UpdateWorld();
  }
};

} // anonymous namespace

// -----------------------------------------------------------------------
// Cell type ID uniqueness
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld cell IDs - all non-zero", "[DynamicWorld][celltype]") {
  TestDynamicWorld world;

  REQUIRE(world.GetGrassId()      != 0);
  REQUIRE(world.GetTreeId()       != 0);
  REQUIRE(world.GetStoneId()      != 0);
  REQUIRE(world.GetWheatId()      != 0);
  REQUIRE(world.GetLumberyardId() != 0);
  REQUIRE(world.GetQuarryId()     != 0);
  REQUIRE(world.GetFarmId()       != 0);
  REQUIRE(world.GetSpawnerId()    != 0);
  REQUIRE(world.GetTownhallId()   != 0);
}

TEST_CASE("DynamicWorld cell IDs - all unique", "[DynamicWorld][celltype]") {
  TestDynamicWorld world;

  std::vector<size_t> ids = {
    world.GetGrassId(),
    world.GetTreeId(),
    world.GetStoneId(),
    world.GetWheatId(),
    world.GetLumberyardId(),
    world.GetQuarryId(),
    world.GetFarmId(),
    world.GetSpawnerId(),
    world.GetTownhallId()
  };

  // Sort and check for adjacent duplicates.
  std::sort(ids.begin(), ids.end());
  for (size_t i = 1; i < ids.size(); ++i) {
    REQUIRE(ids[i] != ids[i - 1]);
  }
}

// -----------------------------------------------------------------------
// ResourceIndex mapping
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld ResourceIndex - correct mappings", "[DynamicWorld][resource]") {
  TestDynamicWorld world;

  // Set each resource to a unique sentinel value and verify retrieval.
  world.SetResource("wood",  10);
  world.SetResource("stone", 20);
  world.SetResource("steel", 30);
  world.SetResource("wheat", 40);

  REQUIRE(world.GetResource("wood")  == 10);
  REQUIRE(world.GetResource("stone") == 20);
  REQUIRE(world.GetResource("steel") == 30);
  REQUIRE(world.GetResource("wheat") == 40);
}

TEST_CASE("DynamicWorld ResourceIndex - resources are independent", "[DynamicWorld][resource]") {
  TestDynamicWorld world;

  world.SetResource("wood",  5);
  world.SetResource("stone", 5);
  world.SetResource("steel", 5);
  world.SetResource("wheat", 5);

  // Overwrite one; the others must not change.
  world.SetResource("stone", 99);

  REQUIRE(world.GetResource("wood")  == 5);
  REQUIRE(world.GetResource("stone") == 99);
  REQUIRE(world.GetResource("steel") == 5);
  REQUIRE(world.GetResource("wheat") == 5);
}

// -----------------------------------------------------------------------
// Sequential move then collect
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld sequence - move onto tree then collect", "[DynamicWorld][sequence]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  // Place a tree one step to the right.
  world.SetCell(51, 50, world.GetTreeId());

  // Move right onto the tree tile.
  int move_result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(move_result != 0);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(51));
  REQUIRE(pos.Y() == Approx(50));

  // Now collect from the tree tile the agent is standing on.
  int collect_result = world.DoAction(agent, world.COLLECT);
  REQUIRE(collect_result != 0);
  REQUIRE(world.GetResource("wood") == 1);
  REQUIRE(world.GetCell(51, 50) == world.GetGrassId());
}

TEST_CASE("DynamicWorld sequence - collect then move away", "[DynamicWorld][sequence]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");
  world.SetCell(50, 50, world.GetStoneId());

  // Collect the stone.
  world.DoAction(agent, world.COLLECT);
  REQUIRE(world.GetResource("stone") == 1);

  // Agent should still be able to move (tile is now grass).
  int move_result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(move_result != 0);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(51));
}

TEST_CASE("DynamicWorld sequence - multiple moves update position correctly", "[DynamicWorld][sequence]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  world.DoAction(agent, world.MOVE_RIGHT);
  world.DoAction(agent, world.MOVE_RIGHT);
  world.DoAction(agent, world.MOVE_DOWN);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(52));
  REQUIRE(pos.Y() == Approx(51));
}

// -----------------------------------------------------------------------
// Two agents racing to collect the same tile
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld collect - second agent on same tile gets nothing", "[DynamicWorld][collect]") {
  TestDynamicWorld world;

  world.SetCell(50, 50, world.GetTreeId());

  auto & agent1 = world.SpawnStubAt(50, 50, "agent1");
  auto & agent2 = world.SpawnStubAt(50, 50, "agent2");

  // First agent collects — succeeds and converts tile to grass.
  int result1 = world.DoAction(agent1, world.COLLECT);
  REQUIRE(result1 != 0);
  REQUIRE(world.GetResource("wood") == 1);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());

  // Second agent tries to collect the same (now-grass) tile — fails.
  int result2 = world.DoAction(agent2, world.COLLECT);
  REQUIRE(result2 == 0);
  REQUIRE(world.GetResource("wood") == 1);  // no extra wood
}

// -----------------------------------------------------------------------
// Building on top of an existing building
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build - cannot build on existing building", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");

  // First build a lumberyard.
  world.SetCell(50, 50, world.GetGrassId());
  world.SetResource("wood", 40);
  world.SetResource("steel", 40);
  world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(world.GetCell(50, 50) == world.GetLumberyardId());

  // Now try to build a quarry on the same tile.
  world.SetResource("stone", 20);
  world.SetResource("wood", 20);
  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  // Tile must still be the lumberyard.
  REQUIRE(world.GetCell(50, 50) == world.GetLumberyardId());
  // Resources must be untouched.
  REQUIRE(world.GetResource("stone") == 20);
  REQUIRE(world.GetResource("wood")  == 20);
}

// -----------------------------------------------------------------------
// Unknown action ID
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld DoAction - unknown action ID returns 0", "[DynamicWorld][action]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "test_agent");

  // Use an action ID well beyond the defined range.
  int result = world.DoAction(agent, 999);
  REQUIRE(result == 0);

  // Position must not change.
  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(50));
  REQUIRE(pos.Y() == Approx(50));
}

// -----------------------------------------------------------------------
// Exact resource deduction with surplus
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - deducts exact cost with surplus resources", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Give far more than required (cost: 20 wood, 20 steel).
  world.SetResource("wood",  100);
  world.SetResource("steel", 100);

  world.DoAction(agent, world.BUILD_LUMBERYARD);

  // Exactly 20 of each should be deducted, leaving 80.
  REQUIRE(world.GetResource("wood")  == 80);
  REQUIRE(world.GetResource("steel") == 80);
}

TEST_CASE("DynamicWorld build quarry - deducts exact cost with surplus resources", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Cost: 20 stone, 20 wood.
  world.SetResource("stone", 100);
  world.SetResource("wood",  100);

  world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(world.GetResource("stone") == 80);
  REQUIRE(world.GetResource("wood")  == 80);
}

TEST_CASE("DynamicWorld build spawner - deducts exact cost with surplus resources", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Cost: 30 stone, 30 wheat.
  world.SetResource("stone", 100);
  world.SetResource("wheat", 100);

  world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(world.GetResource("stone") == 70);
  REQUIRE(world.GetResource("wheat") == 70);
}

TEST_CASE("DynamicWorld build farm - deducts exact cost with surplus resources", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  // Cost: 20 wheat, 20 wood.
  world.SetResource("wheat", 100);
  world.SetResource("wood",  100);

  world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(world.GetResource("wheat") == 80);
  REQUIRE(world.GetResource("wood")  == 80);
}

// -----------------------------------------------------------------------
// Custom world size
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld - custom size constructor sets correct dimensions", "[DynamicWorld][init]") {
  TestDynamicWorld world(30, 20);

  REQUIRE(world.GridWidth()  == 30);
  REQUIRE(world.GridHeight() == 20);
}

TEST_CASE("DynamicWorld - custom size world rejects out-of-bounds move", "[DynamicWorld][init][move]") {
  TestDynamicWorld world(10, 10);
  // Place agent on the right edge.
  auto & agent = world.SpawnStubAt(9, 5, "test_agent");

  int result = world.DoAction(agent, world.MOVE_RIGHT);
  REQUIRE(result == 0);

  auto pos = agent.GetLocation().AsWorldPosition();
  REQUIRE(pos.X() == Approx(9));
}

// -----------------------------------------------------------------------
// Mixed building production
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld UpdateWorld - lumberyard and farm produce independently", "[DynamicWorld][update]") {
  TestDynamicWorld world;

  // Build a lumberyard at (40, 40).
  auto & builder1 = world.SpawnStubAt(40, 40, "builder");
  world.SetCell(40, 40, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(builder1, world.BUILD_LUMBERYARD);

  // Build a farm at (60, 60).
  auto & builder2 = world.SpawnStubAt(60, 60, "builder");
  world.SetCell(60, 60, world.GetGrassId());
  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);
  world.DoAction(builder2, world.BUILD_FARM);

  // After 20 ticks: lumberyard fires (every 20), farm fires twice (every 10).
  world.RunNTicks(20);

  REQUIRE(world.GetResource("wood")  == 1);  // lumberyard tick 20
  REQUIRE(world.GetResource("wheat") == 2);  // farm ticks 10 and 20
}
