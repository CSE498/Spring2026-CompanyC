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
