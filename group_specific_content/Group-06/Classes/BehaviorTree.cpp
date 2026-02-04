#include "BehaviorTree.hpp"

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
    return root ? root->tick() : Status::Failure;
}

/**
 * @brief Stores or updates a value in the agent's memory map.
 * * @param key The unique string identifier for the data.
 * @param value The variant value to be stored.
 */
void BehaviorTree::setMemory(const std::string& key, BBValue value) {
    blackboard[key] = value;
}
