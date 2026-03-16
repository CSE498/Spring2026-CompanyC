#pragma once

#include <cassert>
#include <string>

#include "../core/AgentBase.hpp"
#include "../tools/BehaviorTree.hpp"

namespace cse498 {

class ClassicAgent : public AgentBase {
protected:
    BehaviorTree tree;
    bool vertical = true;   ///< Currently unused in this version, but kept for future movement logic.
    bool reverse = false;   ///< Currently unused in this version, but kept for future movement logic.

    void BuildTree();
    void Sense(const WorldGrid & grid);
    size_t GetAction() const;

public:
    ClassicAgent(size_t id, const std::string & name, const WorldBase & world)
        : AgentBase(id, name, world) { }

    ~ClassicAgent() = default;

    ClassicAgent & SetHorizontal() { vertical = false; return *this; }
    ClassicAgent & SetVertical()   { vertical = true;  return *this; }
    ClassicAgent & ToggleDirection() { reverse = !reverse; return *this; }

    bool Initialize() override {
        BuildTree();

        // Add/remove required actions depending on what your world supports.
        return HasAction("attack")
            && HasAction("gather")
            && HasAction("up")
            && HasAction("down")
            && HasAction("left")
            && HasAction("right");
    }

    size_t SelectAction(const WorldGrid & grid) override {
        Sense(grid);
        tree.update();
        return GetAction();
    }
};

} // namespace cse498