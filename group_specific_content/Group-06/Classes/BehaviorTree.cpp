/**
 * @file BehaviorTree.cpp
 * @author Dillan Kowalski
 * @brief Implementation of the core Behavior Tree base classes.
 */

#include "BehaviorTree.hpp"
#include <stdexcept>

namespace cse498{
/**
 * @brief Assigns a new root node to the tree.
 * @param node The shared pointer to the top-level node (usually a Composite).
 */
void BehaviorTree::setRoot(std::shared_ptr<Node> node) {
    root = node;
}

/**
 * @brief Executes one "pulse" of the tree logic from the root down.
 * @return The resulting Status (Success, Failure, or Running) of the root node.
 */
Status BehaviorTree::update() {
    // We pass the internal blackboard map into the root node.
    // This starts the chain reaction where every node gets access to the same memory.
    if (!root){
        throw std::runtime_error("BehaviorTree::update() called, but no root node was set");
    }
    return root->tick(blackboard);
}

/**
 * @brief Stores or updates a value in the agent's memory map.
 * @param key The unique string identifier for the data.
 * @param value The variant value to be stored.
 */
void BehaviorTree::setMemory(const std::string& key, BBValue value) {
    blackboard[key] = std::move(value);
}
}
