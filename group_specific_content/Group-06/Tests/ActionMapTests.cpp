/**
 * @file ActionMapTests.cpp
 * @brief Unit tests for the ActionMap class.
 */

#include <catch2/catch_test_macros.hpp>
#include "ActionMap.h"

TEST_CASE("ActionMap - Basic Functionality", "[ActionMap]") {
    ActionMap am;
    bool wasCalled = false;

    // Define a simple function that flips the flag
    auto simpleAction = [&]() { wasCalled = true; };

    am.AddFunction("TestFunc", simpleAction);

    SECTION("Trigger executes the stored function") {
        am.Trigger("TestFunc");
        REQUIRE(wasCalled == true);
    }

    SECTION("Triggering a non-existent function does nothing (safe)") {
        // Reset flag
        wasCalled = false;
        am.Trigger("GhostFunc"); // Should just print error and continue
        REQUIRE(wasCalled == false);
    }
}

TEST_CASE("ActionMap - Multiple Registrations", "[ActionMap]") {
    ActionMap am;
    bool saveCalled = false;
    bool loadCalled = false;

    am.AddFunction("Save", [&]() { saveCalled = true; });
    am.AddFunction("Load", [&]() { loadCalled = true; });

    SECTION("Triggering one does not affect the other") {
        am.Trigger("Save");
        REQUIRE(saveCalled == true);
        REQUIRE(loadCalled == false);
    }
}

TEST_CASE("ActionMap - Overwriting Logic", "[ActionMap]") {
    ActionMap am;
    int value = 0;

    // First version sets value to 1
    am.AddFunction("Update", [&]() { value = 1; });

<<<<<<< HEAD
    // Second version sets value to 2 (Same Key!)
=======
    // Second version sets value to 2
>>>>>>> group-06-dev
    am.AddFunction("Update", [&]() { value = 2; });

    SECTION("The map uses the most recent logic") {
        am.Trigger("Update");
        REQUIRE(value == 2);
    }
}
