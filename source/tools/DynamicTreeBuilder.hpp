#pragma once

#include <memory>
#include "../Agents/ClassicAgent.hpp"
#include "BehaviorTree.hpp"

namespace cse498 {

class DynamicTreeBuilder {
public:
    static std::shared_ptr<Node> Build(ClassicAgent* agent);
};

} // namespace cse498
