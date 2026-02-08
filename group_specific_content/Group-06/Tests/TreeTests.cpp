/**
 * @file TreeTests.cpp
 * @author Dillan Kowalski
 * @brief Implementation of behavior tree [Composite/Leaf/Test] logic for Group 06.
 */

#include <catch2/catch_test_macros.hpp>
#include "../Classes/CompositeNodes.hpp"
#include "../Classes/LeafNodes.hpp"
#include "../Classes/DecoratorNodes.hpp"

/**
 * @brief A simple node for testing that returns a fixed status.
 */
class StubNode : public Node {
    Status status;
public:
    StubNode(Status s) : status(s) {}
    Status tick() override {return status; }
    std::string getName() const override {return "Stub"; }
};

TEST_CASE("Sequence Node Logic", "[sequence]"){
    auto seq = std::make_shared<Sequence>("TestSequence");

    SECTION("Fails if any child fails"){
        seq->addChild(std::make_shared<StubNode>(Status::Success));
        seq->addChild(std::make_shared<StubNode>(Status::Failure)); // The "Failure" that stops the chain
        seq->addChild(std::make_shared<StubNode>(Status::Success));

        REQUIRE(seq->tick() == Status::Failure);
    }

    SECTION("Sequence returns Running and stops if a child is Running") {
        auto runningNode = std::make_shared<StubNode>(Status::Running);
        seq->addChild(runningNode);
        seq->addChild(std::make_shared<StubNode>(Status::Success)); // Should not be reached

        REQUIRE(seq->tick() == Status::Running);
    }
}

TEST_CASE("Selector Node Logic", "[composite]"){
    auto selector = std::make_shared<Selector>("TestSelector");
    auto successNode = std::make_shared<StubNode>(Status::Success);
    auto failureNode = std::make_shared<StubNode>(Status::Failure);
    auto runningNode = std::make_shared<StubNode>(Status::Running);

    SECTION("Selector succeeds if the first child succeeds"){
        selector->addChild(successNode);
        selector->addChild(failureNode); //Shouldn't be reached
        REQUIRE(selector->tick() == Status::Success);
    }

    SECTION("Selector falls back and succeeds if the first child fails"){
        selector->addChild(failureNode);
        selector->addChild(successNode);
        REQUIRE(selector->tick() == Status::Success);
    }

    SECTION("Selector fails if all children fail") {
        selector->addChild(failureNode);
        selector->addChild(failureNode);
        REQUIRE(selector->tick() == Status::Failure);
    }

    SECTION("Selector returns Running and stops if a child is Running") {
        selector->addChild(runningNode);
        selector->addChild(successNode); // Should not be reached

        REQUIRE(selector->tick() == Status::Running);
    }
}

TEST_CASE("Action Node Basic Logic", "[leaf]") {
    ActionNode moveNode("MoveToTarget");

    SECTION("Action node returns success and correct name") {
        REQUIRE(moveNode.tick() == Status::Success);
        REQUIRE(moveNode.getName() == "MoveToTarget");
    }
}

TEST_CASE("Inverter Decorator Logic", "[decorator]"){
    auto inverter = std::make_shared<Inverter>("TestInverter");

    SECTION("Inverter turns Success into Failure"){
        inverter->setChild(std::make_shared<StubNode>(Status::Success));
        REQUIRE(inverter->tick() == Status::Failure);
    }

    SECTION("Inverter turns Failure into Success"){
        inverter->setChild(std::make_shared<StubNode>(Status::Failure));
        REQUIRE(inverter->tick() == Status::Success);
    }

    SECTION("Inverter passes through Running status"){
        inverter->setChild(std::make_shared<StubNode>(Status::Running));
        REQUIRE(inverter->tick() == Status::Running);
    }
}
