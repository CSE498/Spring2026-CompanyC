#pragma once

#include <string>

#include "../core/AgentBase.hpp"

namespace cse498 {

class EnemyAgent : public AgentBase {
private:
  bool horizontal = true; // patrol axis
  int direction = 1;      // 1 = right/down, -1 = left/up
  size_t vision_radius = 5;
  std::string target_name = "Player";

public:
  EnemyAgent(size_t id, const std::string &name, const WorldBase &world)
      : AgentBase(id, name, world) {
    symbol = 'X';
  }

  ~EnemyAgent() = default;

  EnemyAgent &SetHorizontal() {
    horizontal = true;
    return *this;
  }
  EnemyAgent &SetVertical() {
    horizontal = false;
    return *this;
  }
  EnemyAgent &ToggleDirection() {
    direction *= -1;
    return *this;
  }
  EnemyAgent &SetVisionRadius(size_t r) {
    vision_radius = r;
    return *this;
  }
  EnemyAgent &SetTargetName(const std::string &name) {
    target_name = name;
    return *this;
  }

  bool Initialize() override {
    return HasAction("up") && HasAction("down") && HasAction("left") &&
           HasAction("right");
    // attack is optional for now
  }

  size_t SelectAction(WorldGrid &grid) override;
};

} // namespace cse498
