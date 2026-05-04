/**
 * @file DynamicTreeBuilderTests.cpp
 * @author Group 06
 * @brief Unit tests for DynamicTreeBuilder branch logic.
 *
 * Tests each branch in isolation by manually populating the blackboard and
 * ticking the root node directly — no Sense() involved. This verifies that
 * the tree structure and conditions are correct independent of ClassicAgent.
 *
 * Blackboard keys read by DynamicTreeBuilder:
 *   on_grass         (bool) - agent standing on a grass tile
 *   wood_count       (int)
 *   stone_count      (int)
 *   steel_count      (int)
 *   wheat_count      (int)
 *   has_spawner      (bool) - spawner already placed
 *   has_built_quarry (bool) - quarry already placed
 *   has_lumberyard   (bool) - lumberyard already placed
 *   has_farm         (bool) - farm already placed
 *   grid             (any)  - const WorldGrid& (SeekGrass branch only)
 *   chosen_action    (str)  - written by whichever branch fires
 */

#include "catch2/catch.hpp"
#include "tools/DynamicTreeBuilder.hpp"
#include "tools/BehaviorTree.hpp"
#include "Agents/ClassicAgent.hpp"
#include "Worlds/DynamicWorld.hpp"

namespace cse498 {

// Thin DynamicWorld subclass — exposes only what tree builder tests need.
// Named DTBTestWorld to avoid collision with TestDynamicWorld in DynamicWorldTests.cpp.
class DTBTestWorld : public DynamicWorld {
public:
    void SetCell(size_t x, size_t y, size_t type_id) { main_grid[x, y] = type_id; }
    size_t GrassId() const { return mGrassId; }
    const WorldGrid& GetGrid() const { return main_grid; }

    ClassicAgent& SpawnClassicAt(size_t x, size_t y) {
        auto& agent = AddAgent<ClassicAgent>("classic_agent");
        agent.SetLocation(WorldPosition(x, y));
        return static_cast<ClassicAgent&>(agent);
    }
};

}

using namespace cse498;

// -----------------------------------------------------------------------
// Townhall branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - townhall branch fires with all resources maxed", "[DynamicTreeBuilder][townhall]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",    true);
    updateBB(bb, "wood_count",  500);
    updateBB(bb, "stone_count", 500);
    updateBB(bb, "steel_count", 500);
    updateBB(bb, "wheat_count", 500);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_townhall");
}

TEST_CASE("DynamicTreeBuilder - townhall branch skipped when one resource is short", "[DynamicTreeBuilder][townhall]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       500);
    updateBB(bb, "stone_count",      499); // one short
    updateBB(bb, "steel_count",      500);
    updateBB(bb, "wheat_count",      500);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);
    updateBB(bb, "has_farm",         false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) != "build_townhall");
}

TEST_CASE("DynamicTreeBuilder - townhall branch skipped when not on grass", "[DynamicTreeBuilder][townhall]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",    false); // correct resources, wrong tile
    updateBB(bb, "wood_count",  500);
    updateBB(bb, "stone_count", 500);
    updateBB(bb, "steel_count", 500);
    updateBB(bb, "wheat_count", 500);
    updateBB<std::any>(bb, "grid", std::cref(world.GetGrid())); // SeekGrass needs grid

    root->tick(bb);

    bool not_townhall = !bb.count("chosen_action") ||
                        std::get<std::string>(bb.at("chosen_action")) != "build_townhall";
    REQUIRE(not_townhall);
}

// -----------------------------------------------------------------------
// Spawner branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - spawner branch fires when conditions met", "[DynamicTreeBuilder][spawner]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",    true);
    updateBB(bb, "stone_count", 30);
    updateBB(bb, "wheat_count", 30);
    updateBB(bb, "wood_count",  0);
    updateBB(bb, "steel_count", 0);
    updateBB(bb, "has_spawner", false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_spawner");
}

TEST_CASE("DynamicTreeBuilder - spawner branch skipped when already built", "[DynamicTreeBuilder][spawner]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",    true);
    updateBB(bb, "stone_count", 30);
    updateBB(bb, "wheat_count", 30);
    updateBB(bb, "wood_count",  0);
    updateBB(bb, "steel_count", 0);
    updateBB(bb, "has_spawner", true); // already built

    root->tick(bb);

    bool not_spawner = !bb.count("chosen_action") ||
                       std::get<std::string>(bb.at("chosen_action")) != "build_spawner";
    REQUIRE(not_spawner);
}

// -----------------------------------------------------------------------
// Quarry branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - quarry branch fires when conditions met", "[DynamicTreeBuilder][quarry]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "stone_count",      20); // 20 < 30 so spawner won't fire
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_quarry");
}

TEST_CASE("DynamicTreeBuilder - quarry branch skipped when already built", "[DynamicTreeBuilder][quarry]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "stone_count",      20);
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", true); // already built

    root->tick(bb);

    bool not_quarry = !bb.count("chosen_action") ||
                      std::get<std::string>(bb.at("chosen_action")) != "build_quarry";
    REQUIRE(not_quarry);
}

// -----------------------------------------------------------------------
// Lumberyard branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - lumberyard branch fires when conditions met", "[DynamicTreeBuilder][lumberyard]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "steel_count",      20);
    updateBB(bb, "stone_count",      0); // stone=0 blocks spawner and quarry
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_lumberyard");
}

TEST_CASE("DynamicTreeBuilder - lumberyard branch skipped when already built", "[DynamicTreeBuilder][lumberyard]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "steel_count",      20);
    updateBB(bb, "stone_count",      0);
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   true); // already built

    root->tick(bb);

    bool not_lumberyard = !bb.count("chosen_action") ||
                          std::get<std::string>(bb.at("chosen_action")) != "build_lumberyard";
    REQUIRE(not_lumberyard);
}

// -----------------------------------------------------------------------
// Farm branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - farm branch fires when conditions met", "[DynamicTreeBuilder][farm]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "wheat_count",      20); // 20 < 30 so spawner won't fire
    updateBB(bb, "stone_count",      0);  // stone=0 blocks spawner and quarry
    updateBB(bb, "steel_count",      0);  // steel=0 blocks lumberyard
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);
    updateBB(bb, "has_farm",         false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_farm");
}

TEST_CASE("DynamicTreeBuilder - farm branch skipped when already built", "[DynamicTreeBuilder][farm]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "wheat_count",      20);
    updateBB(bb, "stone_count",      0);
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);
    updateBB(bb, "has_farm",         true); // already built

    root->tick(bb);

    bool not_farm = !bb.count("chosen_action") ||
                    std::get<std::string>(bb.at("chosen_action")) != "build_farm";
    REQUIRE(not_farm);
}

// -----------------------------------------------------------------------
// Priority tests
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - townhall takes priority over spawner", "[DynamicTreeBuilder][priority]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",    true);
    updateBB(bb, "wood_count",  500);
    updateBB(bb, "stone_count", 500); // also satisfies spawner (>= 30)
    updateBB(bb, "steel_count", 500);
    updateBB(bb, "wheat_count", 500); // also satisfies spawner (>= 30)
    updateBB(bb, "has_spawner", false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_townhall");
}

TEST_CASE("DynamicTreeBuilder - spawner takes priority over quarry", "[DynamicTreeBuilder][priority]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "stone_count",      30); // satisfies both spawner (>=30) and quarry (>=20)
    updateBB(bb, "wheat_count",      30); // satisfies spawner
    updateBB(bb, "wood_count",       20); // satisfies quarry
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_spawner");
}

TEST_CASE("DynamicTreeBuilder - quarry takes priority over lumberyard", "[DynamicTreeBuilder][priority]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true);
    updateBB(bb, "wood_count",       20); // satisfies both quarry and lumberyard
    updateBB(bb, "stone_count",      20); // satisfies quarry; too low for spawner
    updateBB(bb, "steel_count",      20); // satisfies lumberyard
    updateBB(bb, "wheat_count",      0);  // blocks spawner and farm
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);

    root->tick(bb);

    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_quarry");
}

// -----------------------------------------------------------------------
// SeekGrass branch
// -----------------------------------------------------------------------

TEST_CASE("DynamicTreeBuilder - seek grass fires and outputs a movement action", "[DynamicTreeBuilder][seekgrass]") {
    DTBTestWorld world;
    // Ensure a reachable grass tile exists near the agent.
    world.SetCell(42, 40, world.GrassId());
    auto& agent = world.SpawnClassicAt(40, 40);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         false); // not on grass -> SeekGrass activates
    updateBB(bb, "wood_count",       20);    // enough for quarry desire
    updateBB(bb, "stone_count",      20);
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);
    updateBB(bb, "has_farm",         false);
    updateBB<std::any>(bb, "grid",   std::cref(world.GetGrid()));

    root->tick(bb);

    REQUIRE(bb.count("chosen_action") > 0);
    std::string action = std::get<std::string>(bb.at("chosen_action"));
    bool is_move = action == "move_up"   || action == "move_down" ||
                   action == "move_left" || action == "move_right" ||
                   action == "remain_still";
    REQUIRE(is_move);
}

TEST_CASE("DynamicTreeBuilder - seek grass does not fire when already on grass", "[DynamicTreeBuilder][seekgrass]") {
    DTBTestWorld world;
    auto& agent = world.SpawnClassicAt(50, 50);
    auto root = DynamicTreeBuilder::Build(&agent);

    Blackboard bb;
    updateBB(bb, "on_grass",         true); // on grass -> SeekGrass condition is false
    updateBB(bb, "wood_count",       20);
    updateBB(bb, "stone_count",      20);
    updateBB(bb, "steel_count",      0);
    updateBB(bb, "wheat_count",      0);
    updateBB(bb, "has_spawner",      false);
    updateBB(bb, "has_built_quarry", false);
    updateBB(bb, "has_lumberyard",   false);
    updateBB(bb, "has_farm",         false);

    root->tick(bb);

    // Quarry branch fires instead — seek grass must not have overridden it.
    REQUIRE(std::get<std::string>(bb.at("chosen_action")) == "build_quarry");
}
