/**
 * @file TendencyAgentTest.cpp
 *
 * @brief Unit tests for TendencyAgent decision-making and internal logic.
 *
 * Tests cover:
 * - Initialization:
 *   • Ensures required movement and collect actions exist
 *   • Verifies Initialize() succeeds or fails appropriately
 *
 * - Memory Updates (Notify):
 *   • Resource accumulation (wood, stone, wheat, steel)
 *   • Directional failure tracking (fail_up, fail_down, etc.)
 *   • last_succeeded flag updates based on action outcomes
 *
 * - Affordability (CanAfford):
 *   • Validates resource thresholds for all build goals
 *   • Includes edge cases (just below / exactly at requirement)
 *
 * - Goal Mapping Helpers:
 *   • GoalBuildAction(): maps build goals → correct action strings
 *   • GoalResourceCell(): maps collect goals → correct resource types
 *
 * - Goal Selection (ChooseGoal):
 *   • Chooses highest-scoring valid goal based on tendencies
 *   • Prefers build goals only when affordable
 *   • Falls back to best collect goal otherwise
 *   • Applies townhall override when conditions are met
 *
 * - Scoring Functions:
 *   • ScorePersistence(): rewards repeating last successful action
 *   • ScoreAvoidance(): penalizes directions with repeated failures
 *
 * Helpers:
 * - TestWorld:
 *   Minimal mock of WorldBase providing action registration and lookup
 *
 * - TestableTendencyAgent:
 *   Subclass exposing protected methods and internal state for testing
 */

#include "catch2/catch.hpp"
#include "../../source/Agents/TendencyAgent.hpp"
#include "../../source/core/WorldBase.hpp"

#include <string>
#include <unordered_map>

using cse498::TendencyAgent;
using cse498::WorldBase;
using cse498::WorldGrid;


class TestWorld : public WorldBase {
public:
    TestWorld() = default;
    ~TestWorld() override = default;

    void AddTestAction(const std::string& name, size_t id) {
        action_ids[name] = id;
    }

    bool HasAction(const std::string& name) const {
        return action_ids.find(name) != action_ids.end();
    }

    size_t GetActionID(const std::string& name) const {
        auto it = action_ids.find(name);
        REQUIRE(it != action_ids.end());
        return it->second;
    }

    int DoAction(cse498::AgentBase&, size_t) override {
        return 1;
    }

private:
    std::unordered_map<std::string, size_t> action_ids;
};


class TestableTendencyAgent : public TendencyAgent {
public:
    TestableTendencyAgent(size_t id, const std::string& name, WorldBase& world)
        : TendencyAgent(id, name, world) {}

    using TendencyAgent::CanAfford;
    using TendencyAgent::ChooseGoal;
    using TendencyAgent::GoalBuildAction;
    using TendencyAgent::GoalResourceCell;
    using TendencyAgent::ScorePersistence;
    using TendencyAgent::ScoreAvoidance;

    using TendencyAgent::Goal;
    using TendencyAgent::mem;
    using TendencyAgent::current_goal;
};

TEST_CASE("TendencyAgent Initialize succeeds when required actions exist", "[agent][tendency]")
{
    TestWorld world;
    world.AddTestAction("up", 0);
    world.AddTestAction("down", 1);
    world.AddTestAction("left", 2);
    world.AddTestAction("right", 3);
    world.AddTestAction("collect", 4);

    TestableTendencyAgent agent(0, "tester", world);

    CHECK(agent.Initialize());
}

TEST_CASE("TendencyAgent Initialize fails when collect is missing", "[agent][tendency]")
{
    TestWorld world;
    world.AddTestAction("up", 0);
    world.AddTestAction("down", 1);
    world.AddTestAction("left", 2);
    world.AddTestAction("right", 3);
    // no collect

    TestableTendencyAgent agent(0, "tester", world);

    CHECK_FALSE(agent.Initialize());
}

TEST_CASE("TendencyAgent Notify updates resource memory", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    CHECK(agent.mem.wood == 0);
    CHECK(agent.mem.stone == 0);
    CHECK(agent.mem.wheat == 0);
    CHECK(agent.mem.steel == 0);

    agent.Notify("wood", "resource");
    agent.Notify("stone", "resource");
    agent.Notify("wheat", "resource");
    agent.Notify("steel", "resource");
    agent.Notify("wood", "resource");

    CHECK(agent.mem.wood == 2);
    CHECK(agent.mem.stone == 1);
    CHECK(agent.mem.wheat == 1);
    CHECK(agent.mem.steel == 1);
}

TEST_CASE("TendencyAgent Notify updates failure memory and last_succeeded", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    CHECK(agent.mem.last_succeeded);

    agent.Notify("up", "action_failed");
    agent.Notify("left", "action_failed");
    agent.Notify("up", "action_failed");

    CHECK_FALSE(agent.mem.last_succeeded);
    CHECK(agent.mem.fail_up == 2);
    CHECK(agent.mem.fail_left == 1);
    CHECK(agent.mem.fail_down == 0);
    CHECK(agent.mem.fail_right == 0);
}

TEST_CASE("TendencyAgent CanAfford checks lumberyard cost correctly", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    CHECK_FALSE(agent.CanAfford(TestableTendencyAgent::Goal::BuildLumberyard));

    agent.mem.wood = 19;
    agent.mem.steel = 20;
    CHECK_FALSE(agent.CanAfford(TestableTendencyAgent::Goal::BuildLumberyard));

    agent.mem.wood = 20;
    agent.mem.steel = 19;
    CHECK_FALSE(agent.CanAfford(TestableTendencyAgent::Goal::BuildLumberyard));

    agent.mem.wood = 20;
    agent.mem.steel = 20;
    CHECK(agent.CanAfford(TestableTendencyAgent::Goal::BuildLumberyard));
}

TEST_CASE("TendencyAgent CanAfford checks townhall cost correctly", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.mem.wood = 500;
    agent.mem.stone = 500;
    agent.mem.steel = 500;
    agent.mem.wheat = 499;
    CHECK_FALSE(agent.CanAfford(TestableTendencyAgent::Goal::BuildTownhall));

    agent.mem.wheat = 500;
    CHECK(agent.CanAfford(TestableTendencyAgent::Goal::BuildTownhall));
}

TEST_CASE("TendencyAgent GoalBuildAction maps build goals correctly", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.current_goal = TestableTendencyAgent::Goal::BuildLumberyard;
    CHECK(agent.GoalBuildAction() == "build_lumberyard");

    agent.current_goal = TestableTendencyAgent::Goal::BuildQuarry;
    CHECK(agent.GoalBuildAction() == "build_quarry");

    agent.current_goal = TestableTendencyAgent::Goal::BuildFarm;
    CHECK(agent.GoalBuildAction() == "build_farm");

    agent.current_goal = TestableTendencyAgent::Goal::BuildSpawner;
    CHECK(agent.GoalBuildAction() == "build_spawner");

    agent.current_goal = TestableTendencyAgent::Goal::BuildTownhall;
    CHECK(agent.GoalBuildAction() == "build_townhall");

    agent.current_goal = TestableTendencyAgent::Goal::CollectWood;
    CHECK(agent.GoalBuildAction() == "");
}

TEST_CASE("TendencyAgent GoalResourceCell maps collect goals correctly", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.current_goal = TestableTendencyAgent::Goal::CollectWood;
    CHECK(agent.GoalResourceCell() == "tree");

    agent.current_goal = TestableTendencyAgent::Goal::CollectStone;
    CHECK(agent.GoalResourceCell() == "stone");

    agent.current_goal = TestableTendencyAgent::Goal::CollectWheat;
    CHECK(agent.GoalResourceCell() == "wheat");

    agent.current_goal = TestableTendencyAgent::Goal::BuildFarm;
    CHECK(agent.GoalResourceCell() == "");
}

TEST_CASE("TendencyAgent ChooseGoal prefers strongest collect goal when nothing buildable", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.SetCollect(2.0)
         .SetWoodAffinity(3.0)
         .SetStoneAffinity(1.0)
         .SetWheatAffinity(0.5);

    agent.SetBuild(1.0)
         .SetLumberyardDesire(100.0)
         .SetQuarryDesire(100.0)
         .SetFarmDesire(100.0)
         .SetSpawnerDesire(100.0);

    // No resources, so no build goals are affordable.
    agent.ChooseGoal();

    CHECK(agent.current_goal == TestableTendencyAgent::Goal::CollectWood);
}

TEST_CASE("TendencyAgent ChooseGoal prefers affordable build goal over weaker collect goal", "[agent][tendency]")
{
    TestWorld world;
    world.AddTestAction("build_lumberyard", 10);

    TestableTendencyAgent agent(0, "tester", world);

    agent.SetBuild(3.0)
         .SetLumberyardDesire(2.0);

    agent.SetCollect(1.0)
         .SetWoodAffinity(1.0)
         .SetStoneAffinity(1.0)
         .SetWheatAffinity(1.0);

    // Make lumberyard affordable.
    agent.mem.wood = 20;
    agent.mem.steel = 20;

    agent.ChooseGoal();

    CHECK(agent.current_goal == TestableTendencyAgent::Goal::BuildLumberyard);
}

TEST_CASE("TendencyAgent ChooseGoal ignores unaffordable build goals", "[agent][tendency]")
{
    TestWorld world;
    world.AddTestAction("build_quarry", 11);

    TestableTendencyAgent agent(0, "tester", world);

    agent.SetBuild(100.0)
         .SetQuarryDesire(100.0);

    agent.SetCollect(1.0)
         .SetStoneAffinity(2.0)
         .SetWoodAffinity(0.5)
         .SetWheatAffinity(0.5);

    // Not affordable: quarry needs stone >= 20 and wood >= 20.
    agent.mem.stone = 19;
    agent.mem.wood = 20;

    agent.ChooseGoal();

    CHECK(agent.current_goal == TestableTendencyAgent::Goal::CollectStone);
}

TEST_CASE("TendencyAgent ChooseGoal uses townhall hard override when affordable", "[agent][tendency]")
{
    TestWorld world;
    world.AddTestAction("build_townhall", 20);
    world.AddTestAction("build_lumberyard", 10);

    TestableTendencyAgent agent(0, "tester", world);

    // Even if some other goal would otherwise score highly,
    // townhall should override when affordable.
    agent.SetBuild(0.1)
         .SetTownhallDesire(0.1)
         .SetCollect(100.0)
         .SetWoodAffinity(100.0);

    agent.mem.wood = 500;
    agent.mem.stone = 500;
    agent.mem.steel = 500;
    agent.mem.wheat = 500;

    agent.ChooseGoal();

    CHECK(agent.current_goal == TestableTendencyAgent::Goal::BuildTownhall);
}

TEST_CASE("TendencyAgent ScorePersistence rewards repeating successful last action", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.mem.last_action = "right";
    agent.mem.last_succeeded = true;

    CHECK(agent.ScorePersistence("right") == Approx(2.0));
    CHECK(agent.ScorePersistence("left") == Approx(0.0));

    agent.mem.last_succeeded = false;
    CHECK(agent.ScorePersistence("right") == Approx(0.0));
}

TEST_CASE("TendencyAgent ScoreAvoidance penalizes failed directions", "[agent][tendency]")
{
    TestWorld world;
    TestableTendencyAgent agent(0, "tester", world);

    agent.mem.fail_up = 2;
    agent.mem.fail_left = 1;
    agent.mem.fail_down = 0;
    agent.mem.fail_right = 3;

    CHECK(agent.ScoreAvoidance("up") == Approx(-1.5));
    CHECK(agent.ScoreAvoidance("left") == Approx(-0.75));
    CHECK(agent.ScoreAvoidance("down") == Approx(0.0));
    CHECK(agent.ScoreAvoidance("right") == Approx(-2.25));
}
