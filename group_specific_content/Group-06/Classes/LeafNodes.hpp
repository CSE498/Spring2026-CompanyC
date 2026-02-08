/**
 * @file LeafNodes.hpp
 * @author Dillan Kowalski
 * @brief Definitions for Action and Condition nodes (Leaf nodes).
 * * Contains nodes that perform specific agent actions or check
 * world states, serving as the terminal nodes of the tree.
 */

#ifndef LEAF_NODES_HPP
#define LEAF_NODES_HPP

#include "BehaviorTree.hpp"
#include <string>
#include <iostream>

/**
 * @brief A basic action node that prints a message to the console.
 */
class ActionNode : public Node {
    std::string action;
public:
    ActionNode(std::string a) : action(a){}

    Status tick() override;

    /** @brief Returns the name of the action. */
    std::string getName() const override { return action; }
};

#endif
