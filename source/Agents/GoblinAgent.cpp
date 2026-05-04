#include "GoblinAgent.hpp"

#include <cmath>
#include <vector>

#include "../core/WorldBase.hpp"

namespace cse498 {

/**
 * GoblinAgent represents a stationary toll-style enemy in InteractionHeavyWorld.
 * It blocks nearby progress until the player gets adjacent and pays the required gold.
 */

namespace {

/**
 * @brief Compute the Manhattan distance between two positions.
 * @param a First position.
 * @param b Second position.
 * @return The Manhattan distance.
 */
int ManhattanDistance(const WorldPosition &a, const WorldPosition &b) {
  return std::abs(static_cast<int>(a.CellX()) - static_cast<int>(b.CellX())) +
         std::abs(static_cast<int>(a.CellY()) - static_cast<int>(b.CellY()));
}

} // namespace

GoblinAgent::GoblinAgent(size_t id, const std::string &name,
                         const WorldBase &world)
    : AgentBase(id, name, world) {
  symbol = 'G';
}

GoblinAgent &GoblinAgent::SetBlocking(bool value) {
  blocking = value;
  return *this;
}

GoblinAgent &GoblinAgent::ClearBlocking() {
  blocking = false;
  return *this;
}

bool GoblinAgent::IsBlocking() const { return blocking; }

bool GoblinAgent::IsPlayerAdjacent() const { return player_adjacent; }

bool GoblinAgent::CanBePaid() const { return blocking && player_adjacent; }

GoblinAgent &GoblinAgent::SetTargetName(const std::string &name) {
  target_name = name;
  return *this;
}

bool GoblinAgent::Initialize() { return true; }

void GoblinAgent::Sense(WorldGrid & /*grid*/) {
  player_adjacent = false;

  const WorldPosition my_pos = GetLocation().AsWorldPosition();
  std::vector<size_t> known_agents = world.GetKnownAgents(*this);

  for (size_t agent_id : known_agents) {
    if (agent_id == GetID()) {
      continue;
    }

    const AgentBase &other = world.GetAgent(agent_id);

    if (other.GetName().find(target_name) == std::string::npos) {
      continue;
    }

    const WorldPosition other_pos = other.GetLocation().AsWorldPosition();

    if (ManhattanDistance(my_pos, other_pos) == 1) {
      player_adjacent = true;
      return;
    }
  }
}

size_t GoblinAgent::SelectAction(WorldGrid &grid) {
  Sense(grid);
  return 0;
}

} // namespace cse498
