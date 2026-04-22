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

        main_grid.Resize(width, height, mFloorID);
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

    auto pacers = world.GetPacerSpawnPositions();
    REQUIRE(pacers.size() == 1);
    REQUIRE(pacers[0].CellX() == 4);
    REQUIRE(pacers[0].CellY() == 8);
}

TEST_CASE("Test NearStartingPosition", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    REQUIRE(world.NearStartingPosition(world.GetStartPosition()) == true);
    REQUIRE(world.NearStartingPosition(WorldPosition(2, 1)) == true);
    REQUIRE(world.NearStartingPosition(WorldPosition(7, 9)) == false);
}

TEST_CASE("Test GetRandomPosition", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    WorldPosition start = world.GetStartPosition();

    for (int i = 0; i < 20; ++i)
    {
        WorldPosition pos = world.GetRandomPosition();
        REQUIRE_FALSE(pos.SameCell(start));
        REQUIRE(world.NearStartingPosition(pos) == false);
    }
}

TEST_CASE("Test IsEnemyAt", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

    REQUIRE(world.IsEnemyAt(WorldPosition(1, 1)) == false);
    REQUIRE(world.IsEnemyAt(WorldPosition(2, 3)) == false);
    REQUIRE(world.IsEnemyAt(WorldPosition(6, 6)) == false);
}

TEST_CASE("Test PlaceBoulders", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

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

    world.BreakBoulder(1, 1);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount() == 0);
}

TEST_CASE("Test Collect", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

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

    world.Pay(1, 1);
    REQUIRE(world.GetGoldCount() == 0);
}

TEST_CASE("Test ThrowStone", "[InteractionHeavyWorld]")
{
    TestInteractionHeavyWorld world;

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

    world.Collect(1, 1);
    REQUIRE(world.GetStoneCount() == 0);
    REQUIRE(world.GetGoldCount()  == 0);

    world.ThrowStone(1, 1, 1, 0);
    REQUIRE(world.GetStoneCount() == 0);
}

TEST_CASE("Test Const Getters", "[InteractionHeavyWorld]")
{
    const TestInteractionHeavyWorld world;

    REQUIRE(world.GetPlayerHP()                     == 100);
    REQUIRE(world.GetStoneCount()                   == 0);
    REQUIRE(world.GetGoldCount()                    == 0);
    REQUIRE(world.GetStartPosition().CellX()        == 1);
    REQUIRE(world.GetStartPosition().CellY()        == 1);
    REQUIRE(world.GetHunterSpawnPositions().size()  == 1);
    REQUIRE(world.GetGoblinSpawnPositions().size()  == 1);
    REQUIRE(world.GetPacerSpawnPositions().size()   == 1);
    REQUIRE(world.IsEnemyAt(WorldPosition(1, 1))            == false);
    REQUIRE(world.NearStartingPosition(WorldPosition(1, 1)) == true);
}

} // namespace cse498