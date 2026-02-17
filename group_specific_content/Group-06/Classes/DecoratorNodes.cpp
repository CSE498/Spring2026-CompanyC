/**
 * @file DecoratorNodes.cpp
 * @author Dillan Kowalski
 * @brief Implementation of Decorator node logic.
 */

 #include "DecoratorNodes.hpp"

 Status Inverter::tick(Blackboard& bb){
    if (!child) return Status::Failure;

    Status s = child->tick(bb);

    if (s == Status::Success) return Status::Failure;
    if (s == Status::Failure) return Status::Success;

    return s; // If Running, stay Running
}
