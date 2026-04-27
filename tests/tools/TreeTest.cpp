/**
 * @file TreeTests.cpp
 * @author Dillan Kowalski
 * @brief Implementation of behavior tree [Composite/Leaf/Test] logic for Group
 * 06.
 */

#include "tools/CompositeNodes.hpp"
#include "tools/DecoratorNodes.hpp"
#include "tools/LeafNodes.hpp"
#include "catch2/catch.hpp"

using namespace cse498;

/**
 * @brief A simple node for testing that returns a fixed status.
 */
class StubNode : public Node {
  Status status;

public:
  StubNode(Status s) : status(s) {}
  Status tick(Blackboard & /*bb*/) override { return status; }
  const std::string &getName() const override {
    static const std::string kName = "Stub";
    return kName;
  }
};

TEST_CASE("Sequence Node Logic", "[sequence]") {
  Blackboard dummyBB;
  auto seq = std::make_shared<Sequence>("TestSequence");

  SECTION("Fails if any child fails") {
    seq->addChild(std::make_shared<StubNode>(Status::Success));
    seq->addChild(std::make_shared<StubNode>(
        Status::Failure)); // The "Failure" that stops the chain
    seq->addChild(std::make_shared<StubNode>(Status::Success));

    REQUIRE(seq->tick(dummyBB) == Status::Failure);
  }

  SECTION("Sequence returns Running and stops if a child is Running") {
    auto runningNode = std::make_shared<StubNode>(Status::Running);
    seq->addChild(runningNode);
    seq->addChild(
        std::make_shared<StubNode>(Status::Success)); // Should not be reached

    REQUIRE(seq->tick(dummyBB) == Status::Running);
  }
}

TEST_CASE("Selector Node Logic", "[composite]") {
  Blackboard dummyBB;
  auto selector = std::make_shared<Selector>("TestSelector");
  auto successNode = std::make_shared<StubNode>(Status::Success);
  auto failureNode = std::make_shared<StubNode>(Status::Failure);
  auto runningNode = std::make_shared<StubNode>(Status::Running);

  SECTION("Selector succeeds if the first child succeeds") {
    selector->addChild(successNode);
    selector->addChild(failureNode); // Shouldn't be reached
    REQUIRE(selector->tick(dummyBB) == Status::Success);
  }

  SECTION("Selector falls back and succeeds if the first child fails") {
    selector->addChild(failureNode);
    selector->addChild(successNode);
    REQUIRE(selector->tick(dummyBB) == Status::Success);
  }

  SECTION("Selector fails if all children fail") {
    selector->addChild(failureNode);
    selector->addChild(failureNode);
    REQUIRE(selector->tick(dummyBB) == Status::Failure);
  }

  SECTION("Selector returns Running and stops if a child is Running") {
    selector->addChild(runningNode);
    selector->addChild(successNode); // Should not be reached

    REQUIRE(selector->tick(dummyBB) == Status::Running);
  }
}

TEST_CASE("Action Node Basic Logic", "[leaf]") {
  Blackboard dummyBB;
  // Initial state
  updateBB(dummyBB, "gold", 10);

  // Define logic that increments gold
  auto gatherGold = [](Blackboard &bb) {
    int current = std::get<int>(bb.at("gold"));
    updateBB(bb, "gold", current + 5);
    return Status::Success;
  };

  ActionNode moveNode("GatherGold", gatherGold);

  SECTION("Action node executes logic and modifies blackboard") {
    REQUIRE(moveNode.tick(dummyBB) == Status::Success);
    REQUIRE(std::get<int>(dummyBB["gold"]) == 15);
    REQUIRE(moveNode.getName() == "GatherGold");
  }
}

TEST_CASE("Inverter Decorator Logic", "[decorator]") {
  Blackboard dummyBB;
  auto inverter = std::make_shared<Inverter>("TestInverter");

  SECTION("Inverter turns Success into Failure") {
    inverter->setChild(std::make_shared<StubNode>(Status::Success));
    REQUIRE(inverter->tick(dummyBB) == Status::Failure);
  }

  SECTION("Inverter turns Failure into Success") {
    inverter->setChild(std::make_shared<StubNode>(Status::Failure));
    REQUIRE(inverter->tick(dummyBB) == Status::Success);
  }

  SECTION("Inverter passes through Running status") {
    inverter->setChild(std::make_shared<StubNode>(Status::Running));
    REQUIRE(inverter->tick(dummyBB) == Status::Running);
  }
}

TEST_CASE("Condition Node Logic", "[leaf]") {
  Blackboard dummyBB;
  updateBB(dummyBB, "is_hungry", true);

  auto checkHunger = [](const Blackboard &bb) {
    return std::get<bool>(bb.at("is_hungry"));
  };

  ConditionNode hungerNode("HungryCheck", checkHunger);

  SECTION("Returns Success when condition is true") {
    REQUIRE(hungerNode.tick(dummyBB) == Status::Success);
  }

  SECTION("Returns Failure when condition is changed to false") {
    updateBB(dummyBB, "is_hungry", false);
    REQUIRE(hungerNode.tick(dummyBB) == Status::Failure);
  }
}

TEST_CASE("Blackboard Edge Cases - Missing Keys", "[blackboard]") {
  Blackboard emptyBB; // Totally empty

  auto riskyAction = [](Blackboard &bb) {
    // This will throw an exception if "ammo" isn't there
    try {
      std::get<int>(bb.at("ammo"));
      return Status::Success;
    } catch (...) {
      return Status::Failure; // We caught the crash!
    }
  };

  ActionNode node("RiskyNode", riskyAction);

  SECTION("Node handles missing keys gracefully") {
    // If logic catches the error, this passes.
    // If not, test suite crashes here.
    REQUIRE(node.tick(emptyBB) == Status::Failure);
  }
}

TEST_CASE("Blackboard Edge Cases - Wrong Type", "[blackboard]") {
  Blackboard confusedBB;
  updateBB(confusedBB, "health", std::string("Full Health"));

  auto readHealth = [](Blackboard &bb) {
    try {
      std::get<int>(bb.at("health"));
      return Status::Success;
    } catch (const std::bad_variant_access &) {
      return Status::Failure; // Correctly identified wrong type
    }
  };

  ActionNode node("TypeCheck", readHealth);

  SECTION("Node fails when types mismatch") {
    REQUIRE(node.tick(confusedBB) == Status::Failure);
  }
}
