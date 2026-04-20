#include <catch2/catch.hpp>

#include "../../source/Agents/GoblinAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498 {

namespace {

/**
 * @brief A minimal passive agent used for testing adjacency.
 */
class DummyAgent : public AgentBase {
public:
  DummyAgent(size_t id, const std::string &name, const WorldBase &world)
      : AgentBase(id, name, world) {}

  bool Initialize() override { return true; }

  size_t SelectAction(WorldGrid & /*grid*/) override { return 0; }
};

/**
 * @brief A minimal mock world for GoblinAgent unit tests.
 */
class MockWorld : public WorldBase {
protected:
  void ConfigAgent(AgentBase & /*agent*/) override {}

public:
  MockWorld() {
    size_t floor_id = main_grid.AddCellType("floor", "Floor", '.', true);
    main_grid.Resize(10, 10, floor_id);
  }

  int DoAction(AgentBase & /*agent*/, size_t /*action_id*/) override {
    return 1;
  }
};

} // namespace

TEST_CASE("GoblinAgent initializes as blocking", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  goblin.SetLocation(WorldPosition{4, 4});

  REQUIRE(goblin.IsBlocking());
  REQUIRE_FALSE(goblin.IsPlayerAdjacent());
  REQUIRE_FALSE(goblin.CanBePaid());
}

TEST_CASE("GoblinAgent remains stationary", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  goblin.SetLocation(WorldPosition{4, 4});

  size_t action = goblin.SelectAction(world.GetGrid());

  REQUIRE(action == 0);
  REQUIRE(goblin.GetLocation().AsWorldPosition().CellX() == 4);
  REQUIRE(goblin.GetLocation().AsWorldPosition().CellY() == 4);
}

TEST_CASE("GoblinAgent senses adjacent player", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  auto &player = world.AddAgent<DummyAgent>("Player");

  goblin.SetLocation(WorldPosition{4, 4});
  player.SetLocation(WorldPosition{5, 4});

  goblin.SelectAction(world.GetGrid());

  REQUIRE(goblin.IsPlayerAdjacent());
  REQUIRE(goblin.CanBePaid());
}

TEST_CASE("GoblinAgent does not sense distant player", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  auto &player = world.AddAgent<DummyAgent>("Player");

  goblin.SetLocation(WorldPosition{1, 1});
  player.SetLocation(WorldPosition{5, 5});

  goblin.SelectAction(world.GetGrid());

  REQUIRE_FALSE(goblin.IsPlayerAdjacent());
  REQUIRE_FALSE(goblin.CanBePaid());
}

TEST_CASE("GoblinAgent can stop blocking", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  auto &player = world.AddAgent<DummyAgent>("Player");

  goblin.SetLocation(WorldPosition{4, 4});
  player.SetLocation(WorldPosition{4, 5});

  goblin.SelectAction(world.GetGrid());

  REQUIRE(goblin.IsBlocking());
  REQUIRE(goblin.IsPlayerAdjacent());
  REQUIRE(goblin.CanBePaid());

  goblin.ClearBlocking();

  REQUIRE_FALSE(goblin.IsBlocking());
  REQUIRE(goblin.IsPlayerAdjacent());
  REQUIRE_FALSE(goblin.CanBePaid());
}

TEST_CASE("GoblinAgent supports custom target name", "[GoblinAgent]") {
  MockWorld world;
  auto &goblin = world.AddAgent<GoblinAgent>("Goblin");
  auto &hero = world.AddAgent<DummyAgent>("Hero");

  goblin.SetTargetName("Hero");
  goblin.SetLocation(WorldPosition{2, 2});
  hero.SetLocation(WorldPosition{2, 3});

  goblin.SelectAction(world.GetGrid());

  REQUIRE(goblin.IsPlayerAdjacent());
  REQUIRE(goblin.CanBePaid());
}

} // namespace cse498
