/**
 * @file ActionMapTests.cpp
 * @brief Unit tests for the ActionMap class.
 */

#include "catch2/catch.hpp"

#include "tools/ActionMap.hpp"

TEST_CASE("ActionMap - Basic Functionality", "[ActionMap]") {
  cse498::ActionMap am;
  bool wasCalled = false;

  // Define a simple function that flips the flag
  auto simpleAction = [&]() { wasCalled = true; };

  REQUIRE(am.AddFunction<>("TestFunc", std::function<void()>(simpleAction)) ==
          true);

  SECTION("Trigger executes the stored function") {
    auto err = am.Trigger("TestFunc");
    REQUIRE_FALSE(err.has_value());
    REQUIRE(wasCalled == true);
  }

  SECTION("Triggering a non-existent function does nothing (safe)") {
    // Reset flag
    wasCalled = false;
    auto err = am.Trigger("GhostFunc"); // Should just print error and continue
    REQUIRE(err.has_value());
    REQUIRE(wasCalled == false);
  }
}

TEST_CASE("ActionMap - Multiple Registrations", "[ActionMap]") {
  cse498::ActionMap am;
  bool saveCalled = false;
  bool loadCalled = false;

  am.AddFunction<>("Save", std::function<void()>([&]() { saveCalled = true; }));
  am.AddFunction<>("Load", std::function<void()>([&]() { loadCalled = true; }));

  SECTION("Triggering one does not affect the other") {
    auto err = am.Trigger("Save");
    REQUIRE_FALSE(err.has_value());
    REQUIRE(saveCalled == true);
    REQUIRE(loadCalled == false);
  }
}

TEST_CASE("ActionMap - Duplicate Registration Fails", "[ActionMap]") {
  cse498::ActionMap am;
  int value = 0;

  // First version sets value to 1
  REQUIRE(am.AddFunction<>(
              "Update", std::function<void()>([&]() { value = 1; })) == true);

  // Second version sets value to 2
  REQUIRE(am.AddFunction<>(
              "Update", std::function<void()>([&]() { value = 2; })) == false);

  SECTION("The map does not overwrite existing logic") {
    auto err = am.Trigger("Update");
    REQUIRE_FALSE(err.has_value());
    REQUIRE(value == 1);
  }
}

TEST_CASE("ActionMap - ReplaceFunction Overwrites", "[ActionMap]") {
  cse498::ActionMap am;
  int value = 0;

  am.AddFunction<>("Update", std::function<void()>([&]() { value = 1; }));

  am.ReplaceFunction<>("Update", std::function<void()>([&]() { value = 2; }));

  SECTION("ReplaceFunction updates the stored function") {
    auto err = am.Trigger("Update");
    REQUIRE_FALSE(err.has_value());
    REQUIRE(value == 2);
  }
}

TEST_CASE("ActionMap - RemoveFunction", "[ActionMap]") {
  cse498::ActionMap am;

  am.AddFunction<>("DeleteMe", std::function<void()>([]() {}));

  REQUIRE(am.HasFunction("DeleteMe") == true);

  REQUIRE(am.RemoveFunction("DeleteMe") == true);
  REQUIRE(am.HasFunction("DeleteMe") == false);

  REQUIRE(am.RemoveFunction("DeleteMe") == false);
}

TEST_CASE("ActionMap - Clear and Count", "[ActionMap]") {
  cse498::ActionMap am;

  am.AddFunction<>("A", std::function<void()>([]() {}));
  am.AddFunction<>("B", std::function<void()>([]() {}));

  REQUIRE(am.Count() == 2);

  am.Clear();

  REQUIRE(am.Count() == 0);
}

TEST_CASE("ActionMap - GetFunctionNames Sorted", "[ActionMap]") {
  cse498::ActionMap am;

  am.AddFunction<>("Zeta", std::function<void()>([]() {}));
  am.AddFunction<>("Alpha", std::function<void()>([]() {}));
  am.AddFunction<>("Beta", std::function<void()>([]() {}));

  auto names = am.GetFunctionNames();

  REQUIRE(names.size() == 3);
  REQUIRE(names[0] == "Alpha");
  REQUIRE(names[1] == "Beta");
  REQUIRE(names[2] == "Zeta");
}

TEST_CASE("ActionMap - Functions With Arguments", "[ActionMap]") {
  cse498::ActionMap am;
  int result = 0;

  am.AddFunction<int>("Add",
                      std::function<void(int)>([&](int x) { result += x; }));

  SECTION("Trigger with correct argument type") {
    auto err = am.Trigger("Add", 5);
    REQUIRE_FALSE(err.has_value());
    REQUIRE(result == 5);
  }

  SECTION("Trigger with wrong signature fails") {
    auto err = am.Trigger("Add");
    REQUIRE(err.has_value());
  }
}
