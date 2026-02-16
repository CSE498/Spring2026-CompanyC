/**
 * @file CompositeNodes.hpp
 * @author Dillan Kowalski
 * @brief Logic for Sequence and Selector behavior tree nodes.
 */

#ifndef COMPOSITE_NODES_HPP
#define COMPOSITE_NODES_HPP

#include "BehaviorTree.hpp"
#include <vector>
#include <string>
#include <memory>

// Sequence: Runs children in order until one FAILS or returns RUNNING
class Sequence : public Node{
private:
    std::vector<std::shared_ptr<Node>> children;
    std::string name;

public:
    /**
     * @brief Constructor that creates a sequence with a specific name.
     * @param n The string name for this node.
     */
    Sequence(std::string n) : name(n) {}

    /**
     * @brief Appends a child to the sequence list.
     * @param child A shared pointer to the child node to be added.
     */
    void addChild(std::shared_ptr<Node> child) {children.push_back(child); }

    Status tick(Blackboard& bb) override;

    /**
     * @brief Returns the name of this sequence.
     * @return The string name used for identification.
     */
    std::string getName() const override { return name; }
};

/**
 * @brief A Selector node (Logical OR).
 * @details Ticks children in order until one succeeds or returns 'Running'.
 * If a child returns Failure, the Selector moves to the next child.
 * It only returns Failure if ALL children fail.
 */
class Selector : public Node {
private:
    std::vector<std::shared_ptr<Node>> children;
    std::string name;

public:
    /** @brief Constructs a selector with a specific name.
     ** @param n The node name.
     */
    Selector(std::string n) : name(n) {}

    /** @brief Adds a child to the selector's fallback list.
     ** @param child The node to add.
     */
    void addChild(std::shared_ptr<Node> child) {children.push_back(child); }

    Status tick(Blackboard& bb) override;

    /** @brief Returns the node name.
     ** @return The string name.
     */
    std::string getName() const override { return name; }
};

#endif
