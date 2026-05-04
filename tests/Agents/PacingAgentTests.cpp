#include <catch2/catch.hpp>

#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498 {

class MockWorld : public WorldBase {
public:
    MockWorld() {
        size_t floor_id = main_grid.AddCellType("floor", "Floor", '.', true);
        main_grid.Resize(10, 10, floor_id);
    }

    void ConfigAgent(AgentBase &agent) override {
        agent.AddAction("up",    1);
        agent.AddAction("down",  2);
        agent.AddAction("left",  3);
        agent.AddAction("right", 4);
    }

    int DoAction(AgentBase &, size_t) override { return 1; }
};



TEST_CASE("PacingAgent constructs in vertical mode and moves down first", "[PacingAgent]") {
    MockWorld world;
    auto &agent = world.AddAgent<PacingAgent>("Pacer");
    agent.SetLocation(WorldPosition{5, 5});

    SECTION("starts in vertical mode") {
        // First action should be down, not left/right
        size_t action = agent.SelectAction(world.GetGrid());

        REQUIRE(action == agent.GetActionID("down"));
        REQUIRE(action != agent.GetActionID("left"));
        REQUIRE(action != agent.GetActionID("right"));
    }

    SECTION("switches to horizontal when configured") {
        agent.SetHorizontal();

        size_t action = agent.SelectAction(world.GetGrid());

        REQUIRE(action == agent.GetActionID("right"));
        REQUIRE(action != agent.GetActionID("down"));
        REQUIRE(action != agent.GetActionID("up"));
    }

    SECTION("reverse flag changes initial direction") {
        agent.ToggleDirection();

        size_t action = agent.SelectAction(world.GetGrid());

        REQUIRE(action == agent.GetActionID("up"));
    }
} 

} // namespace cse498