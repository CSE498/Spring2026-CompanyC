/**
 * @file CompositeNodes.cpp
 * @author Dillan Kowalski
 * @brief Implementation of Sequence and Selector tick logic.
 */

#include "CompositeNodes.hpp"

/**
 * @brief Executes children sequentially.
 * * @return Status::Success if all children succeed, otherwise returns the status
 * of the first child that fails or is running.
 * * @details
 * Will stop immediately
 * if any child returns Failure or Running, ensuring the agent doesn't
 * perform subsequent tasks until the current one succeeds.
 */
Status Sequence::tick(Blackboard& bb) {
    for (auto& child : children) {
        Status s = child->tick(bb);  // forwards the memory to the next level down
        if (s != Status::Success) return s;
    }
    return Status::Success;
}

Status Selector::tick(Blackboard& bb){
    for(auto& child : children){
       Status s = child->tick(bb);

       // If the child succeeds or is running,
       // the Selector is satisfied and stops looking.
       if (s == Status::Success || s == Status::Running){
           return s;
       }
    }
    // Every child failed
    return Status::Failure;
}
