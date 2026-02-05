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
     * @brief Creates a sequence with a specific name.
     * @param n The string name for this node.
     */
    Sequence(std::string n) : name(n) {}

    /**
     * @brief Appends a child to the sequence list.
     * @param child A shared pointer to the child node to be added.
     */
    void addChild(std::shared_ptr<Node> child) {children.push_back(child); }

    Status tick() override;

    /**
     * @brief Returns the name of this sequence.
     * @return The string name used for identification.
     */
    std::string getName() const override { return name; }
};

#endif
