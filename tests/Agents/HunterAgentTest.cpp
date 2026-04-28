#include "catch2/catch.hpp"
#include "../../source/Agents/HunterAgent.hpp"
#include "../../source/core/WorldBase.hpp"

class HunterTestWorld : public cse498::WorldBase {
public:
    int DoAction(cse498::AgentBase&, size_t) override { return 1; }
};

class TestableHunterAgent : public cse498::HunterAgent {
public:
    TestableHunterAgent(size_t id, const std::string& name, cse498::WorldBase& world)
        : HunterAgent(id, name, world) {}

    State GetState() const { return mState; }
    int GetTargetX() const { return mTargetX; }
    int GetTargetY() const { return mTargetY; }
    int GetChaseMemory() const { return mChaseMemory; }
};

TEST_CASE("HunterAgent target_position parses only valid coordinates", "[HunterAgent]")
{
    HunterTestWorld world;
    TestableHunterAgent hunter(0, "hunter", world);

    hunter.Notify("3,4", "target_position");
    REQUIRE(hunter.GetState() == cse498::HunterAgent::State::Chase);
    REQUIRE(hunter.GetTargetX() == 3);
    REQUIRE(hunter.GetTargetY() == 4);
    REQUIRE(hunter.GetChaseMemory() == hunter.GetChaseMemoryTicks());

    hunter.Notify("bad,7", "target_position");
    REQUIRE(hunter.GetTargetX() == 3);
    REQUIRE(hunter.GetTargetY() == 4);

    hunter.Notify("8,nope", "target_position");
    REQUIRE(hunter.GetTargetX() == 3);
    REQUIRE(hunter.GetTargetY() == 4);

    hunter.Notify("8,9extra", "target_position");
    REQUIRE(hunter.GetTargetX() == 3);
    REQUIRE(hunter.GetTargetY() == 4);
}

TEST_CASE("HunterAgent tuning values stay behind setter invariants", "[HunterAgent]")
{
    HunterTestWorld world;
    TestableHunterAgent hunter(0, "hunter", world);

    hunter.SetDetectRadius(15);
    REQUIRE(hunter.GetDetectRadius() == 15);
    REQUIRE(hunter.GetChaseRadius() >= hunter.GetDetectRadius());

    hunter.SetChaseRadius(4);
    REQUIRE(hunter.GetChaseRadius() == hunter.GetDetectRadius());

    hunter.SetDetectRadius(-3);
    REQUIRE(hunter.GetDetectRadius() == 0);

    hunter.SetChaseMemory(-5);
    REQUIRE(hunter.GetChaseMemoryTicks() == 0);
}
