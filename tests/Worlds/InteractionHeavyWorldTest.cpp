/**
 * @file InteractionHeavyWorldTest.cpp
 * @author Truong Phan
 *
 * @brief Unit tests for the InteractionHeavyWorld class.
 *
 * @citation: These tests were written with Claude AI assistance.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Agents/HunterAgent.hpp"
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
        using InteractionHeavyWorld::BreakBoulder;
        using InteractionHeavyWorld::Collect;
        using InteractionHeavyWorld::Pay;
        using InteractionHeavyWorld::PlaceBoulders;
        using InteractionHeavyWorld::ThrowStone;

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

        REQUIRE(world.GetHunterSpawnPositions().size() == 1);
        REQUIRE(world.GetGoblinSpawnPositions().size() == 1);
        REQUIRE(world.GetPacingSpawnPositions().size() == 1);
    }

    TEST_CASE("Test NearReferencePosition", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        WorldPosition start = world.GetStartPosition();

        REQUIRE(world.NearReferencePosition(start, start) == true);
        REQUIRE(world.NearReferencePosition(WorldPosition(2, 1), start) == true);
        REQUIRE(world.NearReferencePosition(WorldPosition(7, 9), start) == false);
    }

    TEST_CASE("Test GetRandomValidFloorPosition", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        WorldPosition start = world.GetStartPosition();

        for (int i = 0; i < 20; ++i)
        {
            auto pos = world.GetRandomValidFloorPosition();
            REQUIRE(pos.has_value());
            REQUIRE_FALSE(pos->SameCell(start));
        }
    }

    TEST_CASE("Test IsEnemyAt", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        REQUIRE(world.IsEnemyAt(WorldPosition(1, 1)) == false);
        REQUIRE(world.IsEnemyAt(WorldPosition(2, 3)) == false);
        REQUIRE(world.IsEnemyAt(WorldPosition(6, 6)) == false);
    }

    TEST_CASE("Test PlaceBoulders (deterministic behavior only)", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before = world.GetStoneCount();

        world.PlaceBoulders(0, 0);

        REQUIRE(world.GetStoneCount() == before);
    }

    TEST_CASE("Test BreakBoulder (no guaranteed adjacent boulder)", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before_stone = world.GetStoneCount();
        size_t before_gold  = world.GetGoldCount();

        world.BreakBoulder(1, 1);

        REQUIRE(world.GetStoneCount() == before_stone);
        REQUIRE(world.GetGoldCount() == before_gold);
    }

    TEST_CASE("Test Collect from empty cell", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before_stone = world.GetStoneCount();
        size_t before_gold  = world.GetGoldCount();

        world.Collect(1, 1);

        REQUIRE(world.GetStoneCount() == before_stone);
        REQUIRE(world.GetGoldCount() == before_gold);
    }

    TEST_CASE("Test Collect from chest location (valid gain case)", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before_gold = world.GetGoldCount();

        world.Collect(7, 6); // Cell with chest at (7,7) is adjacent to (7,6)

        REQUIRE(world.GetGoldCount() == before_gold + 8);
    }

    TEST_CASE("Test Pay with no goblin nearby", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before = world.GetGoldCount();

        world.Pay(1, 1);

        REQUIRE(world.GetGoldCount() == before);
    }

    TEST_CASE("Test ThrowStone with zero inventory", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t before = world.GetStoneCount();

        world.ThrowStone(1, 1, 0, -1);
        world.ThrowStone(1, 1, 0, 1);
        world.ThrowStone(1, 1, -1, 0);
        world.ThrowStone(1, 1, 1, 0);

        REQUIRE(world.GetStoneCount() == before);
    }

    TEST_CASE("Test PrintInventory does not modify state", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        world.PrintInventory();

        REQUIRE(world.GetPlayerHP() == 100);
        REQUIRE(world.GetStoneCount() == 0);
        REQUIRE(world.GetGoldCount() == 0);
    }

    TEST_CASE("Test SyncResourceVector stability", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t stone_before = world.GetStoneCount();
        size_t gold_before  = world.GetGoldCount();

        world.Collect(1, 1);
        world.ThrowStone(1, 1, 1, 0);

        REQUIRE(world.GetStoneCount() == stone_before);
        REQUIRE(world.GetGoldCount() == gold_before);
    }

    TEST_CASE("Test Const Getters", "[InteractionHeavyWorld]")
    {
        const TestInteractionHeavyWorld world;

        REQUIRE(world.GetPlayerHP() == 100);
        REQUIRE(world.GetStoneCount() == 0);
        REQUIRE(world.GetGoldCount() == 0);

        REQUIRE(world.GetStartPosition().CellX() == 1);
        REQUIRE(world.GetStartPosition().CellY() == 1);

        REQUIRE(world.GetHunterSpawnPositions().size() == 1);
        REQUIRE(world.GetGoblinSpawnPositions().size() == 1);
        REQUIRE(world.GetPacingSpawnPositions().size() == 1);

        REQUIRE(world.IsEnemyAt(WorldPosition(1, 1)) == false);
        REQUIRE(world.IsReservedPosition(world.GetStartPosition()) == true);
    }

    TEST_CASE("Test Break + Collect consistency (no assumptions about map state)", "[InteractionHeavyWorld]")
    {
        TestInteractionHeavyWorld world;

        size_t stone_before = world.GetStoneCount();
        size_t gold_before  = world.GetGoldCount();

        world.BreakBoulder(1, 1);
        world.Collect(1, 1);

        REQUIRE(world.GetStoneCount() >= stone_before);
        REQUIRE(world.GetGoldCount() >= gold_before);
    }
} // namespace cse498