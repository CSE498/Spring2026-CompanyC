/**
 * @file DecoratorNodes.hpp
 * @author Dillan Kowalski
 * @brief Base class for Decorator nodes.
 */

#ifndef DECORATOR_NODES_HPP
#define DECORATOR_NODES_HPP

#include "BehaviorTree.hpp"
#include <stdexcept>

namespace cse498{
/**
 * @brief Abstract base for nodes that modify the result of a single child.
 */
class DecoratorNode : public Node {
protected:
    std::shared_ptr<Node> child;
    std::string name;
public:
    DecoratorNode(std::string n) : name(n){}
    void setChild(std::shared_ptr<Node> c) {
        if (!c) {
            throw std::invalid_argument("Cannot set a null child for DecoratorNode '" + name + "'");
        }
        child = c;
    }
    const std::string& getName() const override { return name; }
};

/**
 * @brief Flips Success to Failure and vice-versa.
 */
class Inverter : public DecoratorNode {
public:
    using DecoratorNode::DecoratorNode;  // Use parent constructor
    Status tick(Blackboard& bb) override;
};
}
#endif
