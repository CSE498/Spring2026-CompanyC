/**
 * @file LeafNodes.cpp
 * @author Dillan Kowalski
 * @brief Implementation of Action node logic for Group 06.
 */

#include "LeafNodes.hpp"

Status ActionNode::tick(Blackboard& bb) {
    if(action){
        return action(bb);
    }
    return Status::Failure;
}

Status ConditionNode::tick(Blackboard& bb){
    if(condition && condition(bb)){  //Checks if function exists and if the function returns true
        return Status::Success;
    }
    return Status::Failure;
}
