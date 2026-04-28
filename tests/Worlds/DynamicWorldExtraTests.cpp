/**
 * @file DynamicWorldExtraTests.cpp
 * @brief Additional Catch2 tests for DynamicWorld covering:
 *   - REMAIN_STILL action
 *   - Missing build-failure cases not already covered in DynamicWorldTests.cpp
 *   - Tick-based resource production / spawning
 *   - Townhall win condition (run_over flag)
 *   - GetTickCount tracking
 *   - Collect on a building tile
 *   - Diagonal moves at world corners
 */

#include "catch2/catch.hpp"
#include "Worlds/DynamicWorld.hpp"

namespace {

class StubAgent : public cse498::AgentBase {
public:
  size_t next_action = 0;

  StubAgent(size_t id, const std::string & name, const cse498::WorldBase & world)
    : AgentBase(id, name, world) { }

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

  void SetResource(const std::string & name, size_t value) {
    mWorldResourceCounts[ResourceIndex(name)] = value;
  }

  [[nodiscard]] size_t GetResource(const std::string & name) const {
    return mWorldResourceCounts[ResourceIndex(name)];
  }

  void SetCell(size_t x, size_t y, size_t type_id) {
    main_grid[cse498::WorldPosition(x, y)] = type_id;
  }

  [[nodiscard]] size_t GetCell(size_t x, size_t y) const {
    return main_grid[cse498::WorldPosition(x, y)];
  }

  [[nodiscard]] bool IsRunOver() const { return run_over; }

  StubAgent & SpawnStubAt(size_t x, size_t y, const std::string & name) {
    auto & agent = AddAgent<StubAgent>(name);
    agent.SetLocation(cse498::WorldPosition(x, y));
    return static_cast<StubAgent &>(agent);
  }

  void RunNTicks(size_t n) {
    for (size_t i = 0; i < n; ++i) {
      Tick();
    }
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

  world.SetCell(50, 50, world.GetLumberyardId());

  int result = world.DoAction(agent, world.COLLECT);

  REQUIRE(result == 0);
  REQUIRE(world.GetResource("wood") == 0);
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
// Build failures - additional missing resource cases
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld build lumberyard - fails when only steel is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 20);
  world.SetResource("steel", 19);

  int result = world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wood") == 20);
  REQUIRE(world.GetResource("steel") == 19);
}

TEST_CASE("DynamicWorld build quarry - fails when only stone is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
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

TEST_CASE("DynamicWorld build quarry - fails when only wood is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 19);

  int result = world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 20);
  REQUIRE(world.GetResource("wood") == 19);
}

TEST_CASE("DynamicWorld build spawner - fails when only stone is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
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

TEST_CASE("DynamicWorld build spawner - fails when only wheat is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 29);

  int result = world.DoAction(agent, world.BUILD_SPAWNER);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("stone") == 30);
  REQUIRE(world.GetResource("wheat") == 29);
}

TEST_CASE("DynamicWorld build farm - fails when only wheat is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
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

TEST_CASE("DynamicWorld build farm - fails when only wood is insufficient", "[DynamicWorld][build]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 19);

  int result = world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(result == 0);
  REQUIRE(world.GetCell(50, 50) == world.GetGrassId());
  REQUIRE(world.GetResource("wheat") == 20);
  REQUIRE(world.GetResource("wood") == 19);
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
  cse498::WorldBase & base = world;
  REQUIRE(base.GetTickCount() == 0);
}

TEST_CASE("DynamicWorld GetTickCount - increments each Tick call", "[DynamicWorld][tick]") {
  TestDynamicWorld world;
  cse498::WorldBase & base = world;

  world.Tick();
  REQUIRE(base.GetTickCount() == 1);

  world.Tick();
  REQUIRE(base.GetTickCount() == 2);

  world.RunNTicks(8);
  REQUIRE(base.GetTickCount() == 10);
}

// -----------------------------------------------------------------------
// Tick - additional production / spawning tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld Tick - lumberyard produces wood every 20 ticks", "[DynamicWorld][tick][lumberyard]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(agent, world.BUILD_LUMBERYARD);

  REQUIRE(world.GetResource("wood") == 0);

  world.RunNTicks(19);
  REQUIRE(world.GetResource("wood") == 0);

  world.Tick();
  REQUIRE(world.GetResource("wood") == 1);

  world.RunNTicks(19);
  REQUIRE(world.GetResource("wood") == 1);

  world.Tick();
  REQUIRE(world.GetResource("wood") == 2);
}

TEST_CASE("DynamicWorld Tick - quarry produces stone and steel every 10 ticks", "[DynamicWorld][tick][quarry]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("stone", 20);
  world.SetResource("wood", 20);
  world.DoAction(agent, world.BUILD_QUARRY);

  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  world.RunNTicks(9);
  REQUIRE(world.GetResource("stone") == 0);
  REQUIRE(world.GetResource("steel") == 0);

  world.Tick();
  REQUIRE(world.GetResource("stone") == 1);
  REQUIRE(world.GetResource("steel") == 1);

  world.RunNTicks(9);
  world.Tick();
  REQUIRE(world.GetResource("stone") == 2);
  REQUIRE(world.GetResource("steel") == 2);
}

TEST_CASE("DynamicWorld Tick - farm produces wheat every 10 ticks", "[DynamicWorld][tick][farm]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());

  world.SetResource("wheat", 20);
  world.SetResource("wood", 20);
  world.DoAction(agent, world.BUILD_FARM);

  REQUIRE(world.GetResource("wheat") == 0);

  world.RunNTicks(9);
  REQUIRE(world.GetResource("wheat") == 0);

  world.Tick();
  REQUIRE(world.GetResource("wheat") == 1);

  world.RunNTicks(9);
  world.Tick();
  REQUIRE(world.GetResource("wheat") == 2);
}

TEST_CASE("DynamicWorld Tick - two lumberyards double production", "[DynamicWorld][tick][lumberyard]") {
  TestDynamicWorld world;

  auto & builder1 = world.SpawnStubAt(40, 40, "builder");
  world.SetCell(40, 40, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(builder1, world.BUILD_LUMBERYARD);

  auto & builder2 = world.SpawnStubAt(60, 60, "builder");
  world.SetCell(60, 60, world.GetGrassId());
  world.SetResource("wood", 20);
  world.SetResource("steel", 20);
  world.DoAction(builder2, world.BUILD_LUMBERYARD);

  world.RunNTicks(20);
  REQUIRE(world.GetResource("wood") == 2);
}

TEST_CASE("DynamicWorld Tick - spawner creates an agent at tick 60", "[DynamicWorld][tick][spawner]") {
  TestDynamicWorld world;
  auto & agent = world.SpawnStubAt(50, 50, "builder");
  world.SetCell(50, 50, world.GetGrassId());
  world.SetCell(51, 50, world.GetGrassId());

  world.SetResource("stone", 30);
  world.SetResource("wheat", 30);
  world.DoAction(agent, world.BUILD_SPAWNER);

  size_t agents_before = world.GetNumAgents();

  world.RunNTicks(59);
  REQUIRE(world.GetNumAgents() == agents_before);

  world.Tick();
  REQUIRE(world.GetNumAgents() == agents_before + 1);
}

// -----------------------------------------------------------------------
// Player notification tests
// -----------------------------------------------------------------------

// Agent that records the last message passed to Notify().
class PlayerAgent : public cse498::AgentBase {
public:
  std::string last_message;

  PlayerAgent(size_t id, const std::string & name, const cse498::WorldBase & world)
    : AgentBase(id, name, world) {}

  size_t SelectAction(cse498::WorldGrid &) override { return 0; }

  void Notify(const std::string & message, const std::string & /*msg_type*/ = "none") override {
    last_message = message;
  }
};

TEST_CASE("DynamicWorld Tick - Player agent receives notification", "[DynamicWorld][tick][player]") {
  TestDynamicWorld world;
  auto & player = world.AddAgent<PlayerAgent>("Player");
  player.SetLocation(cse498::WorldPosition(10, 10));

  world.Tick();

  // Notify must have been called — message should not be empty.
  REQUIRE(!player.last_message.empty());
}

TEST_CASE("DynamicWorld Tick - Player notification contains tick count", "[DynamicWorld][tick][player]") {
  TestDynamicWorld world;
  auto & player = world.AddAgent<PlayerAgent>("Player");
  player.SetLocation(cse498::WorldPosition(10, 10));

  world.Tick();

  // After one tick, mUpdateCounter == 1, so message should mention "Tick 1".
  REQUIRE(player.last_message.find("Tick 1") != std::string::npos);
}

TEST_CASE("DynamicWorld Tick - Player notification contains resource counts", "[DynamicWorld][tick][player]") {
  TestDynamicWorld world;
  auto & player = world.AddAgent<PlayerAgent>("Player");
  player.SetLocation(cse498::WorldPosition(10, 10));

  world.SetResource("wood", 42);

  world.Tick();

  REQUIRE(player.last_message.find("42") != std::string::npos);
}

TEST_CASE("DynamicWorld Tick - non-Player agent does not receive notification", "[DynamicWorld][tick][player]") {
  TestDynamicWorld world;
  // Add a regular stub agent — it should NOT get notified.
  auto & other = world.AddAgent<PlayerAgent>("other_agent");
  other.SetLocation(cse498::WorldPosition(10, 10));

  world.Tick();

  REQUIRE(other.last_message.empty());
}

TEST_CASE("DynamicWorld Tick - spawner creates a second agent at tick 120", "[DynamicWorld][tick][spawner]") {
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

// -----------------------------------------------------------------------
// Ghost agent (named "Player") movement tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicWorld - Ghost agent named 'Player' can move into non-traversable cell",
          "[DynamicWorld][movement][ghost]") {
  TestDynamicWorld world;
  auto & ghost = world.AddAgent<PlayerAgent>("Player");
  ghost.SetLocation(cse498::WorldPosition(50, 50));
  world.SetCell(50, 50, world.GetGrassId());

  // Place a non-traversable quarry cell directly to the right of the ghost.
  world.SetCell(51, 50, world.GetQuarryId());

  // Ghost should be allowed to move into the quarry cell.
  int result = world.DoAction(ghost, world.MOVE_RIGHT);
  REQUIRE(result != 0);
  REQUIRE(ghost.GetLocation().AsWorldPosition().X() == 51);
  REQUIRE(ghost.GetLocation().AsWorldPosition().Y() == 50);
}

TEST_CASE("DynamicWorld - Non-ghost agent cannot move into non-traversable cell",
          "[DynamicWorld][movement][ghost]") {
  TestDynamicWorld world;
  auto & normal = world.SpawnStubAt(50, 50, "not_player");
  world.SetCell(50, 50, world.GetGrassId());

  // Place a non-traversable quarry cell directly to the right.
  world.SetCell(51, 50, world.GetQuarryId());

  // Non-ghost agent should be blocked.
  int result = world.DoAction(normal, world.MOVE_RIGHT);
  REQUIRE(result == 0);
  REQUIRE(normal.GetLocation().AsWorldPosition().X() == 50);
  REQUIRE(normal.GetLocation().AsWorldPosition().Y() == 50);
}