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
#include <functional>
#include <cassert>

/**
 * @brief A Leaf Node that executes a function and returns a Status.
 */
class ActionNode : public Node {
public:
    using ActionFunc = std::function<Status(Blackboard&)>;

    ActionNode(std::string n, ActionFunc func) : nodeName(n), action(func) {
        assert(func && "ActionNode must be initialized with a valid function");
    }

    Status tick(Blackboard& bb) override;

    /** @brief Returns the name of the action. */
    std::string getName() const override { return nodeName; }

private:
    std::string nodeName;
    ActionFunc action;
};

/**
 * @brief A Leaf Node that returns Success or Failure based on a Blackboard check.
 */
class ConditionNode : public Node {
public:
    // A function pointer/lambda that takes a Blackboard and returns a bool
    using ConditionFunc = std::function<bool(const Blackboard&)>;

    ConditionNode(std::string n, ConditionFunc func) : name(n), condition(func) {
        assert(func && "ConditionNode must be initialized with a valid function");
    }

    Status tick(Blackboard& bb) override;
    std::string getName() const override { return name; }
private:
    std::string name;
    ConditionFunc condition;
};

#endif
