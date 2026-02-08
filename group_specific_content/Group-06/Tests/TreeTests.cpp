#include <catch2/catch_test_macros.hpp>
#include "../Classes/CompositeNodes.hpp"

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
}
