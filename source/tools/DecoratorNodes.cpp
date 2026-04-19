 #include "DecoratorNodes.hpp"
 #include <cassert>
namespace cse498{
/**
 * @file DecoratorNodes.cpp
 * @author Dillan Kowalski
 * @brief Implementation of Decorator node logic.
 */
 Status Inverter::tick(Blackboard& bb){
    assert(child != nullptr);

    Status s = child->tick(bb);

    if (s == Status::Success) return Status::Failure;
    if (s == Status::Failure) return Status::Success;

    return s; // If Running, stay Running
}
}
