#pragma once

#include <cassert>
#include <string>

#include "../core/AgentBase.hpp"
#include "../tools/BehaviorTree.hpp"

namespace cse498 {

class ClassicAgent : public AgentBase {
protected:
    BehaviorTree tree;

    virtual void BuildTree();
    virtual void Sense(WorldGrid & grid);
    virtual size_t GetAction() const;

public:
    ClassicAgent(size_t id, const std::string & name, const WorldBase & world)
        : AgentBase(id, name, world) { }

    virtual ~ClassicAgent() = default;

    bool Initialize() override {
        BuildTree();

        return HasAction("up")
            && HasAction("down")
            && HasAction("left")
            && HasAction("right");
    }

    size_t SelectAction(WorldGrid & grid) override;
};

} // namespace cse498
