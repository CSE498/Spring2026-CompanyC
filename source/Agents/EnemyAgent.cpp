#include "EnemyAgent.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../core/WorldBase.hpp"

namespace cse498 {

namespace {

/**
 * @brief Determine whether two world positions occupy the same cell.
 * @param a First position.
 * @param b Second position.
 * @return True if both positions refer to the same cell.
 */
bool SameCell(const WorldPosition &a, const WorldPosition &b) {
  return a.CellX() == b.CellX() && a.CellY() == b.CellY();
}

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

/**
 * @brief Determine whether two positions are orthogonally adjacent.
 * @param a First position.
 * @param b Second position.
 * @return True if the positions are adjacent.
 */
bool IsAdjacent(const WorldPosition &a, const WorldPosition &b) {
  return ManhattanDistance(a, b) == 1;
}

/**
 * @brief Determine whether a cell may be traversed by an enemy.
 * @param grid The world grid.
 * @param pos The position to test.
 * @return True if the cell is walkable.
 */
bool IsWalkableCell(const WorldGrid &grid, const WorldPosition &pos) {
  if (pos.CellX() >= grid.GetWidth() || pos.CellY() >= grid.GetHeight()) {
    return false;
  }

  size_t cell_id = grid[pos];
  std::string cell_type = grid.GetCellTypeName(cell_id);

  return cell_type != "wall" && cell_type != "blocked" &&
         cell_type != "goblin_block";
}

/**
 * @brief Convert a single-step path segment into an action string.
 * @param from Starting world position.
 * @param to_point Next path point.
 * @return The corresponding movement action, if valid.
 */
std::optional<std::string> ActionFromStep(const WorldPosition &from,
                                          const WorldPosition &to_pos) {
  const int to_x = static_cast<int>(to_pos.CellX());
  const int to_y = static_cast<int>(to_pos.CellY());

  const int from_x = static_cast<int>(from.CellX());
  const int from_y = static_cast<int>(from.CellY());

  const int dx = to_x - from_x;
  const int dy = to_y - from_y;

  if (dx == 1 && dy == 0)
    return "right";
  if (dx == -1 && dy == 0)
    return "left";
  if (dx == 0 && dy == 1)
    return "down";
  if (dx == 0 && dy == -1)
    return "up";

  return std::nullopt;
}

/**
 * @brief Choose a direct four-way chase action toward the player.
 * @param grid The world grid.
 * @param enemy_pos The enemy's current position.
 * @param player_pos The sensed player position.
 * @return The chase movement action if a valid step is available.
 */
std::optional<std::string>
GetDirectChaseAction(const WorldGrid &grid, const WorldPosition &enemy_pos,
                     const WorldPosition &player_pos) {
  const int enemy_x = static_cast<int>(enemy_pos.CellX());
  const int enemy_y = static_cast<int>(enemy_pos.CellY());

  const int player_x = static_cast<int>(player_pos.CellX());
  const int player_y = static_cast<int>(player_pos.CellY());

  const int dx = player_x - enemy_x;
  const int dy = player_y - enemy_y;

  std::vector<std::pair<std::string, WorldPosition>> options;

  auto try_add_move = [&](const std::string &action, int x, int y) {
    if (x < 0 || y < 0) {
      return;
    }

    options.push_back({action, WorldPosition(static_cast<size_t>(x),
                                             static_cast<size_t>(y))});
  };

  if (std::abs(dx) >= std::abs(dy)) {
    if (dx > 0) {
      try_add_move("right", enemy_x + 1, enemy_y);
    } else if (dx < 0) {
      try_add_move("left", enemy_x - 1, enemy_y);
    }

    if (dy > 0) {
      try_add_move("down", enemy_x, enemy_y + 1);
    } else if (dy < 0) {
      try_add_move("up", enemy_x, enemy_y - 1);
    }
  } else {
    if (dy > 0) {
      try_add_move("down", enemy_x, enemy_y + 1);
    } else if (dy < 0) {
      try_add_move("up", enemy_x, enemy_y - 1);
    }

    if (dx > 0) {
      try_add_move("right", enemy_x + 1, enemy_y);
    } else if (dx < 0) {
      try_add_move("left", enemy_x - 1, enemy_y);
    }
  }

  for (const auto &[action, next_pos] : options) {
    if (IsWalkableCell(grid, next_pos)) {
      return action;
    }
  }

  return std::nullopt;
}

/**
 * @brief Compute the next patrol position along the current patrol axis.
 * @param grid The world grid.
 * @param current The enemy's current position.
 * @param horizontal Whether patrol is horizontal.
 * @param direction Patrol direction.
 * @return The next patrol position if valid.
 */
std::optional<WorldPosition> GetPatrolNextPos(const WorldGrid &grid,
                                              const WorldPosition &current,
                                              bool horizontal, int direction) {
  int next_x = static_cast<int>(current.CellX());
  int next_y = static_cast<int>(current.CellY());

  if (horizontal)
    next_x += direction;
  else
    next_y += direction;

  if (next_x < 0 || next_y < 0) {
    return std::nullopt;
  }

  WorldPosition next_pos(static_cast<size_t>(next_x),
                         static_cast<size_t>(next_y));

  if (!IsWalkableCell(grid, next_pos)) {
    return std::nullopt;
  }

  return next_pos;
}

} // namespace

EnemyAgent::EnemyAgent(size_t id, const std::string &name,
                       const WorldBase &world)
    : AgentBase(id, name, world) {
  symbol = 'X';
}

/**
 * @brief Set this enemy to patrol horizontally.
 * @return Reference to this enemy agent.
 */
EnemyAgent &EnemyAgent::SetHorizontal() {
  horizontal = true;
  return *this;
}

/**
 * @brief Set this enemy to patrol vertically.
 * @return Reference to this enemy agent.
 */
EnemyAgent &EnemyAgent::SetVertical() {
  horizontal = false;
  return *this;
}

/**
 * @brief Reverse the current patrol direction.
 * @return Reference to this enemy agent.
 */
EnemyAgent &EnemyAgent::ToggleDirection() {
  direction *= -1;
  return *this;
}

/**
 * @brief Set the enemy's vision radius.
 * @param r Maximum Manhattan detection distance.
 * @return Reference to this enemy agent.
 */
EnemyAgent &EnemyAgent::SetVisionRadius(size_t r) {
  vision_radius = r;
  return *this;
}

/**
 * @brief Set the substring used to identify the player by name.
 * @param name Target name substring.
 * @return Reference to this enemy agent.
 */
EnemyAgent &EnemyAgent::SetTargetName(const std::string &name) {
  assert(!name.empty());
  target_name = name;
  return *this;
}

/**
 * @brief Verify that the enemy has the actions it needs to function.
 * @return True if required actions are available.
 */
bool EnemyAgent::Initialize() {
  return HasAction("up") && HasAction("down") && HasAction("left") &&
         HasAction("right");
}

/**
 * @brief Update the enemy's sensed state for the current tick.
 * @param grid The world grid visible to the agent.
 */
void EnemyAgent::Sense(WorldGrid &grid) {
  player_in_vision = false;
  player_adjacent = false;
  sensed_player_pos.reset();
  chase_action.reset();
  patrol_action.reset();

  if (target_name.empty()) {
    return;
  }

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

    sensed_player_pos = other.GetLocation().AsWorldPosition();
    break;
  }

  if (sensed_player_pos.has_value()) {
    const int dist = ManhattanDistance(my_pos, *sensed_player_pos);
    player_in_vision = dist <= static_cast<int>(vision_radius);
    player_adjacent = IsAdjacent(my_pos, *sensed_player_pos);

    if (player_in_vision && !player_adjacent) {
      chase_action = GetDirectChaseAction(grid, my_pos, *sensed_player_pos);
    }
  }

  auto next_pos = GetPatrolNextPos(grid, my_pos, horizontal, direction);

  if (!next_pos.has_value()) {
    direction *= -1;
    next_pos = GetPatrolNextPos(grid, my_pos, horizontal, direction);
  }

  if (next_pos.has_value()) {
    patrol_action = ActionFromStep(my_pos, *next_pos);
  }
}

/**
 * @brief Select the next action for this enemy.
 * @param grid The current world grid.
 * @return The chosen action ID.
 */
size_t EnemyAgent::SelectAction(WorldGrid &grid) {
  Sense(grid);

  if (player_adjacent && HasAction("attack")) {
    return GetActionID("attack");
  }

  if (player_in_vision && chase_action.has_value() &&
      HasAction(*chase_action)) {
    return GetActionID(*chase_action);
  }

  if (patrol_action.has_value() && HasAction(*patrol_action)) {
    return GetActionID(*patrol_action);
  }

  return 0;
}

} // namespace cse498
