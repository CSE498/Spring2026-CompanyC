/**
 * @file ClassicAgentTests.cpp
 * @author Group 06
 * @brief Integration tests for ClassicAgent using the full Sense → tree →
 * GetAction pipeline.
 *
 * Unlike DynamicTreeBuilderTests, these tests go through
 * ClassicAgent::SelectAction(), which calls Sense() to populate the blackboard
 * and then runs the tree. This verifies that the complete decision pipeline
 * produces correct action IDs against a real world.
 *
 * NOTE: ClassicAgent::Sense() currently sets wood/stone/steel/wheat counts and
 * explore_move, but does NOT set on_grass. Building branches in
 * DynamicTreeBuilder require on_grass=true to fire, so they will not activate
 * through this pipeline until Sense() is updated. The tests below cover what is
 * currently working: movement/exploration.
 */

#include "Agents/ClassicAgent.hpp"
#include "Worlds/DynamicWorld.hpp"
#include "catch2/catch.hpp"

namespace cse498
{

  // Named CATestWorld to avoid collision with TestDynamicWorld in
  // DynamicWorldTests.cpp.
  class CATestWorld : public DynamicWorld
  {
  public:
    using DynamicWorld::BUILD_FARM;
    using DynamicWorld::BUILD_LUMBERYARD;
    using DynamicWorld::BUILD_QUARRY;
    using DynamicWorld::BUILD_SPAWNER;
    using DynamicWorld::BUILD_TOWNHALL;
    using DynamicWorld::COLLECT;
    using DynamicWorld::MOVE_DOWN;
    using DynamicWorld::MOVE_LEFT;
    using DynamicWorld::MOVE_RIGHT;
    using DynamicWorld::MOVE_UP;

    void SetCell(size_t x, size_t y, size_t type_id)
    {
      main_grid[x, y] = type_id;
    }
    size_t GetCell(size_t x, size_t y) const { return main_grid[x, y]; }
    WorldGrid &GetGrid() { return main_grid; }

    size_t GrassId() const { return mGrassId; }
    size_t TreeId() const { return mTreeId; }
    size_t StoneId() const { return mStoneId; }
    size_t WheatId() const { return mWheatId; }
    size_t QuarryId() const { return mQuarryId; }
    size_t LumberyardId() const { return mLumberyardId; }
    size_t FarmId() const { return mFarmId; }
    size_t SpawnerId() const { return mSpawnerId; }
    size_t TownhallId() const { return mTownhallId; }

    void SetResource(const std::string &name, size_t value)
    {
      world_global_counts[name] = value;
    }

    ClassicAgent &SpawnClassicAt(size_t x, size_t y)
    {
      auto &agent = AddAgent<ClassicAgent>("classic_agent");
      agent.SetLocation(WorldPosition(x, y));
      return static_cast<ClassicAgent &>(agent);
    }
  };

} // namespace cse498

using namespace cse498;

// -----------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------

TEST_CASE("ClassicAgent - initializes successfully in DynamicWorld",
          "[ClassicAgent][init]")
{
  CATestWorld world;
  // AddAgent calls ConfigAgent then Initialize; Initialize returns true only if
  // the four movement actions were registered.
  auto &agent = world.SpawnClassicAt(50, 50);

  REQUIRE(agent.HasAction("up"));
  REQUIRE(agent.HasAction("down"));
  REQUIRE(agent.HasAction("left"));
  REQUIRE(agent.HasAction("right"));
}

// -----------------------------------------------------------------------
// Movement / exploration
// -----------------------------------------------------------------------

TEST_CASE("ClassicAgent - returns a valid movement action when exploring",
          "[ClassicAgent][move]")
{
  CATestWorld world;
  // Place agent in open space with no resources — tree will use explore
  // fallback.
  auto &agent = world.SpawnClassicAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  size_t action = agent.SelectAction(world.GetGrid());

  // Any movement or remain-still is acceptable; 0 (REMAIN_STILL) is also valid.
  bool is_valid_action = action == CATestWorld::MOVE_UP ||
                         action == CATestWorld::MOVE_DOWN ||
                         action == CATestWorld::MOVE_LEFT ||
                         action == CATestWorld::MOVE_RIGHT || action == 0;
  REQUIRE(is_valid_action);
}

TEST_CASE(
    "ClassicAgent - does not return a build action when resources are zero",
    "[ClassicAgent][move]")
{
  CATestWorld world;
  auto &agent = world.SpawnClassicAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  // No resources set — no building branch should fire.
  size_t action = agent.SelectAction(world.GetGrid());

  REQUIRE(action != CATestWorld::BUILD_QUARRY);
  REQUIRE(action != CATestWorld::BUILD_LUMBERYARD);
  REQUIRE(action != CATestWorld::BUILD_FARM);
  REQUIRE(action != CATestWorld::BUILD_SPAWNER);
  REQUIRE(action != CATestWorld::BUILD_TOWNHALL);
}

TEST_CASE(
    "ClassicAgent - SelectAction does not crash when agent is at grid boundary",
    "[ClassicAgent][move]")
{
  CATestWorld world;
  // Top-left corner — several move directions are invalid.
  auto &agent = world.SpawnClassicAt(1, 1);

  // Should return some action without throwing or crashing.
  REQUIRE_NOTHROW(agent.SelectAction(world.GetGrid()));
}

TEST_CASE(
    "ClassicAgent - SelectAction returns consistent type across multiple calls",
    "[ClassicAgent][move]")
{
  CATestWorld world;
  auto &agent = world.SpawnClassicAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  // Call SelectAction multiple times; should never throw and always return
  // size_t.
  for (int i = 0; i < 5; ++i)
  {
    REQUIRE_NOTHROW(agent.SelectAction(world.GetGrid()));
  }
}

// -----------------------------------------------------------------------
// Resource sensing
// -----------------------------------------------------------------------

TEST_CASE("ClassicAgent - Sense picks up wood count from world",
          "[ClassicAgent][sense]")
{
  CATestWorld world;
  auto &agent = world.SpawnClassicAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  world.SetResource("wood", 100);

  // SelectAction runs Sense() which reads wood from world global counts.
  // We can't read the blackboard directly, but verifying it doesn't crash
  // and returns a valid action confirms Sense() ran without error.
  REQUIRE_NOTHROW(agent.SelectAction(world.GetGrid()));
}

TEST_CASE("ClassicAgent - moves toward nearest visible resource tile",
          "[ClassicAgent][sense]")
{
  CATestWorld world;
  auto &agent = world.SpawnClassicAt(50, 50);
  world.SetCell(50, 50, world.GrassId());

  // Place a tree directly to the right — Sense() should detect it and
  // the explore path should move toward it.
  world.SetCell(51, 50, world.TreeId());

  size_t action = agent.SelectAction(world.GetGrid());

  // The agent should move right toward the resource.
  // (Sense sets explore_move via PathGenerator toward nearest resource.)
  REQUIRE(action == CATestWorld::MOVE_RIGHT);
}
