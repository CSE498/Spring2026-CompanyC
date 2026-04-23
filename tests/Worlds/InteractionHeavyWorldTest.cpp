/**
 * @file InteractionHeavyWorldTest.cpp
 * @author Truong Phan
 * 
 * @brief Unit tests for the InteractionHeavyWorld class.
 * 
 * @citation: These tests were written with Claude AI assistance. 
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Worlds/InteractionHeavyWorld.hpp"
#include <string>
#include <vector>

namespace cse498 {

// Known positions (x = col, y = row, 0-indexed):
//   S = (1,1)  — player start
//   H = (2,3)  — hunter spawn
//   G = (6,6)  — goblin spawn
//   P = (4,8)  — pacer spawn
//   C = (7,7)  — chest
//   X = (7,9)  — exit

class TestInteractionHeavyWorld : public InteractionHeavyWorld
{
public:
    TestInteractionHeavyWorld()
    {
        static const std::vector<std::string> kTestLayout = {
            "##########",
            "#S       #",
            "# ## ### #",
            "# H  #   #",
            "## # #   #",
            "#  # #   #",
            "# ## #G###",
            "#    # C #",
            "### P#   #",
            "#######X##"
        };

        size_t width  = kTestLayout[0].size();
        size_t height = kTestLayout.size();

        main_grid.Resize(width, height, mFloorCellID);
        LoadDungeon(kTestLayout);
    }
};

TEST_CASE("Test Default Values", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    REQUIRE(world.GetPlayerHP() == 100);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount() == 0);
    REQUIRE(world.GetStartPosition().CellX() == 1);
    REQUIRE(world.GetStartPosition().CellY() == 1);
}

TEST_CASE("Test Spawn Positions", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    auto hunters = world.GetHunterSpawnPositions();
    REQUIRE(hunters.size() == 1);
    REQUIRE(hunters[0].CellX() == 2);
    REQUIRE(hunters[0].CellY() == 3);

    auto goblins = world.GetGoblinSpawnPositions();
    REQUIRE(goblins.size() == 1);
    REQUIRE(goblins[0].CellX() == 6);
    REQUIRE(goblins[0].CellY() == 6);

    auto pacers = world.GetPacingSpawnPositions();
    REQUIRE(pacers.size() == 1);
    REQUIRE(pacers[0].CellX() == 4);
    REQUIRE(pacers[0].CellY() == 8);
}

TEST_CASE("Test NearPosition", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    WorldPosition start = world.GetStartPosition();

    REQUIRE(world.NearPosition(start, start) == true);
    REQUIRE(world.NearPosition(WorldPosition(2, 1), start) == true);
    REQUIRE(world.NearPosition(WorldPosition(7, 9), start) == false);
}

TEST_CASE("Test GetRandomPosition", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    WorldPosition start = world.GetStartPosition();

    for (int i = 0; i < 20; ++i)
    {
        WorldPosition pos = world.GetRandomPosition();
        REQUIRE_FALSE(pos.SameCell(start));
        REQUIRE(world.NearPosition(pos, start) == false);
    }
}

TEST_CASE("Test IsEnemyAt", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // No agents have been added to the world yet, so no cell should report an enemy
    REQUIRE(world.IsEnemyAt(WorldPosition(1, 1)) == false);
    REQUIRE(world.IsEnemyAt(WorldPosition(2, 3)) == false);
    REQUIRE(world.IsEnemyAt(WorldPosition(6, 6)) == false);
}

TEST_CASE("Test PlaceBoulders", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // Placing boulders only modifies tiles; stone count is unchanged until boulders are broken
    world.PlaceBoulders(0, 0);
    REQUIRE(world.GetStoneCount() == 0);

    world.PlaceBoulders(3, 3);
    REQUIRE(world.GetStoneCount() == 0);

    world.PlaceBoulders(1, 5);
    REQUIRE(world.GetStoneCount() == 0);
}

TEST_CASE("Test BreakBoulder", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // No adjacent boulder at (1,1) — stone and gold counts remain unchanged
    world.BreakBoulder(1, 1);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount() == 0);
}

TEST_CASE("Test Collect", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // No material or chest adjacent to (1,1) — nothing to collect
    world.Collect(1, 1);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount() == 0);

    // Chest is at (7,7) — collect from (7,6) which is directly above it
    size_t gold_before = world.GetGoldCount();
    world.Collect(7, 6);
    REQUIRE(world.GetGoldCount() > gold_before);
}

TEST_CASE("Test Pay", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // No goblin agent present in the world — gold count unchanged
    world.Pay(1, 1);
    REQUIRE(world.GetGoldCount() == 0);
}

TEST_CASE("Test ThrowStone", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    // Stone count is 0 — all throws are no-ops
    world.ThrowStone(1, 1, 0, -1);
    REQUIRE(world.GetStoneCount() == 0);

    world.ThrowStone(1, 1, 0, 1);
    REQUIRE(world.GetStoneCount() == 0);

    world.ThrowStone(1, 1, -1, 0);
    REQUIRE(world.GetStoneCount() == 0);

    world.ThrowStone(1, 1, 1, 0);
    REQUIRE(world.GetStoneCount() == 0);
}

TEST_CASE("Test PrintInventory", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    world.PrintInventory();
    REQUIRE(world.GetPlayerHP()   == 100);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount()  == 0);
}

TEST_CASE("Test SyncResourceVector", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    REQUIRE(world.GetPlayerHP()   == 100);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount()  == 0);

    // Collect from an empty cell — resources unchanged, sync is a no-op
    world.Collect(1, 1);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount()  == 0);

    // No stones to throw — stone count remains 0
    world.ThrowStone(1, 1, 1, 0);
    REQUIRE(world.GetStoneCount() == 0);
}

TEST_CASE("Test Const Getters", "[InteractionHeavyWorld]")
{
    const TestInteractionHeavyWorld world;

    REQUIRE(world.GetPlayerHP()                        == 100);
    REQUIRE(world.GetStoneCount()                      == 0);
    REQUIRE(world.GetGoldCount()                       == 0);
    REQUIRE(world.GetStartPosition().CellX()           == 1);
    REQUIRE(world.GetStartPosition().CellY()           == 1);
    REQUIRE(world.GetHunterSpawnPositions().size()     == 1);
    REQUIRE(world.GetGoblinSpawnPositions().size()     == 1);
    REQUIRE(world.GetPacingSpawnPositions().size()     == 1);
    REQUIRE(world.IsEnemyAt(WorldPosition(1, 1))                                      == false);
    REQUIRE(world.NearPosition(WorldPosition(1, 1), world.GetStartPosition())         == true);
    REQUIRE(world.IsReservedPosition(world.GetStartPosition())                        == true);
    REQUIRE(world.IsReservedPosition(WorldPosition(7, 9))                             == false);
}

} // namespace cse498