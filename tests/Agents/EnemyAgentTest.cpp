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

// Ensures enemy agents focus on the closest one
TEST_CASE("EnemyAgent targets closest matching agent when multiple exist", "[EnemyAgent]") {
    cse498::TestWorld world;

    // Far player added first — without the fix, this is the one that gets picked
    auto& far_player = world.AddAgent<cse498::WebInterface>("Player");
    far_player.SetLocation(cse498::WorldPosition{8, 5});

    // Near player added second — this is the correct target
    auto& near_player = world.AddAgent<cse498::WebInterface>("Player");
    near_player.SetLocation(cse498::WorldPosition{4, 5});

    auto& enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
    enemy.SetVisionRadius(10).SetLocation(cse498::WorldPosition{3, 5});

    size_t action = enemy.SelectAction(world.GetGrid());

    REQUIRE(action == enemy.GetActionID("attack"));
}

TEST_CASE("EnemyAgent returns no action when both patrol directions are blocked", "[EnemyAgent]") {
    cse498::TestWorld world;

    // Place player far away so patrol logic is used
    auto& player = world.AddAgent<cse498::WebInterface>("Player");
    player.SetLocation(cse498::WorldPosition{9, 9});

    auto& enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
    enemy.SetHorizontal().SetVisionRadius(1)
         .SetLocation(cse498::WorldPosition{5, 5});

    // Block all horizontal/vertical neighbors, trapping the enemy
    world.GetGrid()[cse498::WorldPosition{4, 5}] = world.wall_id;
    world.GetGrid()[cse498::WorldPosition{6, 5}] = world.wall_id;
    world.GetGrid()[cse498::WorldPosition{5, 6}] = world.wall_id;
    world.GetGrid()[cse498::WorldPosition{5, 4}] = world.wall_id;

    size_t action = enemy.SelectAction(world.GetGrid());

    REQUIRE(action == 0);
}

namespace cse498 {

class TestWorldNoAttack : public WorldBase {
public:
    TestWorldNoAttack() {
        wall_id  = main_grid.AddCellType("wall",  "Wall",  '#', false);
        floor_id = main_grid.AddCellType("floor", "Floor", '.', true);
        main_grid.Resize(10, 10, floor_id);
    }

    void ConfigAgent(AgentBase& agent) override {
        // Intentionally omit "attack"
        agent.AddAction("up",    1);
        agent.AddAction("down",  2);
        agent.AddAction("left",  3);
        agent.AddAction("right", 4);
    }

    int DoAction(AgentBase&, size_t) override { return 1; }

    size_t wall_id  = 0;
    size_t floor_id = 0;
};

} // namespace cse498

TEST_CASE("EnemyAgent falls back to patrol when adjacent but attack is not registered", "[EnemyAgent]") {
    cse498::TestWorldNoAttack world;

    auto& player = world.AddAgent<cse498::WebInterface>("Player");
    player.SetLocation(cse498::WorldPosition{4, 5});

    auto& enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");
    enemy.SetHorizontal().SetVisionRadius(5)
         .SetLocation(cse498::WorldPosition{3, 5});

    size_t action = enemy.SelectAction(world.GetGrid());

    // Attack is not registered, so the agent should fall through to patrol
    REQUIRE_FALSE(enemy.HasAction("attack"));
    REQUIRE(action == enemy.GetActionID("right"));
}

TEST_CASE("EnemyAgent clamps current HP when max is lowered below it", "[EnemyAgent]") {
    cse498::TestWorld world;
    auto& enemy = world.AddAgent<cse498::EnemyAgent>("Enemy");

    SECTION("HP clamped to new max when max is lowered") {
        enemy.SetMaxHP(100);
        enemy.SetHP(80);
        enemy.SetMaxHP(50);

        REQUIRE(enemy.GetMaxHP() == 50);
        REQUIRE(enemy.GetHP() == 50);
    }

    SECTION("HP clamped to zero when max is set to zero") {
        enemy.SetMaxHP(100);
        enemy.SetHP(50);
        enemy.SetMaxHP(0);

        REQUIRE(enemy.GetMaxHP() == 0);
        REQUIRE(enemy.GetHP() == 0);
    }
}
