#include "LeafNodes.hpp"

Status ActionNode::tick() {
    std::cout << "  Agent is: " << action << std::endl;
    return Status::Success;
}
