/**
 * @file LeafNodes.cpp
 * @author Dillan Kowalski
 * @brief Implementation of Action node logic for Group 06.
 */

#include "LeafNodes.hpp"

Status ActionNode::tick() {
    std::cout << "  Agent is: " << action << std::endl;
    return Status::Success;
}
