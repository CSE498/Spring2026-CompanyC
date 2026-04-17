#pragma once
#include "ClassicAgent.hpp"

namespace cse498{

class ClassicDynamicAgent : public ClassicAgent{
protected:
    void BuildTree() override;

public:
    ClassicDynamicAgent(size_t id, const std::string & name, const WorldBase & world)
        : ClassicAgent(id, name, world) {}
};

} // namespace cse498
