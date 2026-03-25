/**
 * @file BehaviorTree.hpp
 * @author Dillan Kowalski
 * @brief Base class and interface definitions for the Behavior Tree system.
 * This file defines the Status enum and the abstract Node base class
 * that all Composite and Leaf nodes must implement.
 */

#ifndef BEHAVIOR_TREE_HPP
#define BEHAVIOR_TREE_HPP

#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <vector>

namespace cse498 {
/** @brief Return statuses for tree execution. */
enum class Status {
    Failure = 0,
    Success = 1,
    Running = -1
};

using BBValue = std::variant<int, double, std::string, bool>;
using Blackboard = std::unordered_map<std::string, BBValue>;

template<typename T>
inline void updateBB(Blackboard& bb, const std::string& key, T value){
    bb[key] = BBValue(std::in_place_type<T>, value);
}

/**
 * @brief Abstract base class for all nodes in the Behavior Tree.
 * Any custom action or condition must inherit from this and implement tick().
 */
class Node {
public:
    virtual ~Node() = default;

    /**
     * @brief Executes the node's specific logic.
     * @param bb Reference to the agent's memory
     */
    virtual Status tick(Blackboard& bb) = 0;

    /**
     * @brief Returns the identifier for the node.
     * @return A string representing the node's name.
     */
    virtual const std::string& getName() const = 0;
};

/**
 * @brief The primary manager for an agent's decision-making logic.
 * @details
 * The BehaviorTree coordinates the execution of the node hierarchy
 * and provides a Blackboard memory system. It serves as the
 * central hub for agent intelligence.
 */
class BehaviorTree {
private:
    std::shared_ptr<Node> root;
    Blackboard blackboard;  //The actual memory
public:
    void setRoot(std::shared_ptr<Node> node);

    Status update();

    void setMemory(const std::string& key, BBValue value) {
        blackboard[key] = value;
    }

    template<typename T>
    void setMemory(const std::string& key, T value){
        blackboard[key] = BBValue(std::in_place_type<T>, value);
    };

    const Blackboard& getBlackboard() const { return blackboard ;}
};
}
#endif
