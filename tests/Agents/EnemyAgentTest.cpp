#include "catch2/catch.hpp"

#include "../../source/Agents/EnemyAgent.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/tools/WebInterface.hpp"

namespace cse498 {

class TestWorld : public WorldBase {
public:
  TestWorld() {
    wall_id = main_grid.AddCellType("wall", "Wall", '#', false);
    floor_id = main_grid.AddCellType("floor", "Floor", '.', true);

    main_grid.Resize(10, 10, floor_id);
  }

  void ConfigAgent(AgentBase &agent) override {
    agent.AddAction("up", 1);
    agent.AddAction("down", 2);
    agent.AddAction("left", 3);
    agent.AddAction("right", 4);
    agent.AddAction("attack", 5);
  }

  int DoAction(AgentBase &, size_t) override { return 1; }

  size_t wall_id = 0;
  size_t floor_id = 0;
};

} // namespace cse498

TEST_CASE("EnemyAgent patrols when player is outside vision", "[EnemyAgent]") {
  cse498::TestWorld world;

  auto &player = world.AddAgent<cse498::WebInterface>("Player");
  player.SetLocation(cse498::WorldPosition{1, 1});

  auto &enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
  enemy.SetHorizontal().SetVisionRadius(2).SetLocation(
      cse498::WorldPosition{7, 7});

  size_t action = enemy.SelectAction(world.GetGrid());

  REQUIRE((action == enemy.GetActionID("left") ||
           action == enemy.GetActionID("right")));
}

TEST_CASE("EnemyAgent chases when player is inside vision", "[EnemyAgent]") {
  cse498::TestWorld world;

  auto &player = world.AddAgent<cse498::WebInterface>("Player");
  player.SetLocation(cse498::WorldPosition{4, 3});

  auto &enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
  enemy.SetVisionRadius(5).SetLocation(cse498::WorldPosition{2, 3});

  size_t action = enemy.SelectAction(world.GetGrid());

  REQUIRE(action == enemy.GetActionID("right"));
}

TEST_CASE("EnemyAgent attacks when adjacent", "[EnemyAgent]") {
  cse498::TestWorld world;

  auto &player = world.AddAgent<cse498::WebInterface>("Player");
  player.SetLocation(cse498::WorldPosition{3, 3});

  auto &enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
  enemy.SetVisionRadius(5).SetLocation(cse498::WorldPosition{2, 3});

  size_t action = enemy.SelectAction(world.GetGrid());

  REQUIRE(action == enemy.GetActionID("attack"));
}

TEST_CASE("EnemyAgent reverses patrol when blocked", "[EnemyAgent]") {
  cse498::TestWorld world;

  auto &player = world.AddAgent<cse498::WebInterface>("Player");
  player.SetLocation(cse498::WorldPosition{9, 9});

  auto &enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
  enemy.SetHorizontal().SetVisionRadius(1).SetLocation(
      cse498::WorldPosition{9, 5});

  size_t action = enemy.SelectAction(world.GetGrid());

  REQUIRE(action == enemy.GetActionID("left"));
}

TEST_CASE("EnemyAgent chases left when player is to the left", "[EnemyAgent]") {
  cse498::TestWorld world;

  auto &player = world.AddAgent<cse498::WebInterface>("Player");
  player.SetLocation(cse498::WorldPosition{2, 3});

  auto &enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
  enemy.SetVisionRadius(5).SetLocation(cse498::WorldPosition{4, 3});

  size_t action = enemy.SelectAction(world.GetGrid());

  REQUIRE(action == enemy.GetActionID("left"));
}
