#pragma once

#include "../core/AgentBase.hpp"

namespace cse498 {

class PlayerAgent : public AgentBase {
public:
  PlayerAgent(size_t id, const std::string &name, const WorldBase &world)
      : AgentBase(id, name, world) {
    symbol = 'P';
  }

  bool Initialize() override {
    return HasAction("up") && HasAction("down") && HasAction("left") &&
           HasAction("right");
  }

  size_t SelectAction(WorldGrid &) override { return 0; }
};

} // namespace cse498
