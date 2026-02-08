/**
 * @file BehaviorTree.hpp
 * @author Dillan Kowalski
 * @brief Base class and interface definitions for the Behavior Tree system.
 * * This file defines the Status enum and the abstract Node base class
 * that all Composite and Leaf nodes must implement.
 */

#ifndef BEHAVIOR_TREE_HPP
#define BEHAVIOR_TREE_HPP

#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <vector>

/** @brief Return statuses for tree execution. */
enum class Status {
    Failure = 0,
    Success = 1,
    Running = -1
};

/**
 * @brief Abstract base class for all nodes in the Behavior Tree.
 * Any custom action or condition must inherit from this and implement tick().
 */
class Node {
public:
    virtual ~Node() = default;

    /**
     * @brief Executes the node's specific logic.
     * @return The resulting Status of the execution.
     */
    virtual Status tick() = 0;

    /**
     * @brief Returns the identifier for the node.
     * @return A string representing the node's name.
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief The primary manager for an agent's decision-making logic.
 *
 * @details
 * The BehaviorTree coordinates the execution of the node hierarchy
 * and provides a Blackboard memory system. It serves as the
 * central hub for agent intelligence.
 */
class BehaviorTree {
private:
    std::shared_ptr<Node> root;

    /** @brief Variant-based memory map for agent state data. */
    using BBValue = std::variant<int, double, std::string, bool>;
    std::unordered_map<std::string, BBValue> blackboard;
public:
    void setRoot(std::shared_ptr<Node> node);

    Status update();

    void setMemory(const std::string& key, BBValue value);
};

#endif
