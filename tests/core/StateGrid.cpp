#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/StateGrid.hpp"
#include "../../source/tools/StateGridPosition.h"

TEST_CASE("StateGrid basic functionality", "[StateGrid]")
{
    cse498::StateGrid sg1(20, 20);
    cse498::StateGrid sg2;

    SECTION("Default constructor creates empty grid")
    {
        CHECK(sg2.GetHeight() == 0);
        CHECK(sg2.GetWidth() == 0);
    }

    SECTION("Parameterized constructor sets correct dimensions")
    {
        CHECK(sg1.GetHeight() == 20);
        CHECK(sg1.GetWidth() == 20);
    }

    SECTION("InBounds works correctly")
    {
        CHECK(sg1.InBounds(StateGridPosition(0,0)) == true);
        CHECK(sg1.InBounds(StateGridPosition(19,19)) == true);

        CHECK(sg1.InBounds(StateGridPosition(20,0)) == false);
        CHECK(sg1.InBounds(StateGridPosition(0,20)) == false);
        CHECK(sg1.InBounds(StateGridPosition(100,100)) == false);
    }

    SECTION("SetState and GetState store and retrieve correctly")
    {
        StateGridPosition pos(9,9);

        cse498::State state;
        state.stateID = 1;
        state.isAccessible = true;

        sg1.SetState(pos, state);

        cse498::State retrieved = sg1.GetState(pos);

        CHECK(retrieved.stateID == 1);
        CHECK(retrieved.isAccessible == true);
    }

    SECTION("Multiple states do not interfere with each other")
    {
        StateGridPosition pos1(2,3);
        StateGridPosition pos2(5,6);

        cse498::State state1;
        state1.stateID = 42;
        state1.isAccessible = true;

        cse498::State state2;
        state2.stateID = 99;
        state2.isAccessible = false;

        sg1.SetState(pos1, state1);
        sg1.SetState(pos2, state2);

        auto r1 = sg1.GetState(pos1);
        auto r2 = sg1.GetState(pos2);

        CHECK(r1.stateID == 42);
        CHECK(r1.isAccessible == true);

        CHECK(r2.stateID == 99);
        CHECK(r2.isAccessible == false);
    }

    SECTION("Edge positions work correctly")
    {
        StateGridPosition topLeft(0,0);
        StateGridPosition bottomRight(19,19);

        cse498::State state;
        state.stateID = 7;
        state.isAccessible = true;

        sg1.SetState(topLeft, state);
        sg1.SetState(bottomRight, state);

        CHECK(sg1.GetState(topLeft).stateID == 7);
        CHECK(sg1.GetState(bottomRight).stateID == 7);
    }

    SECTION("Default state values are correct")
    {
        StateGridPosition pos(5,5);

        auto state = sg1.GetState(pos);

        CHECK(state.stateID == 0);
        CHECK(state.isAccessible == false);
    }
}

