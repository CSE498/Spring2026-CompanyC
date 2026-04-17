#include "EnemyAgent.hpp"

#include <cmath>
#include <optional>
#include <string>

#include "../core/WorldBase.hpp"
#include "../tools/PathGenerator.hpp"

namespace cse498 {

namespace {

bool SameCell(const WorldPosition &a, const WorldPosition &b) {
  return a.CellX() == b.CellX() && a.CellY() == b.CellY();
}

int ManhattanDistance(const WorldPosition &a, const WorldPosition &b) {
  return std::abs(static_cast<int>(a.CellX()) - static_cast<int>(b.CellX())) +
         std::abs(static_cast<int>(a.CellY()) - static_cast<int>(b.CellY()));
}

bool IsAdjacent(const WorldPosition &a, const WorldPosition &b) {
  return ManhattanDistance(a, b) == 1;
}

bool IsWalkableCell(const WorldGrid &grid, const WorldPosition &pos) {
  if (pos.CellX() >= grid.GetWidth() || pos.CellY() >= grid.GetHeight()) {
    return false;
  }

  size_t cell_id = grid[pos];
  std::string cell_type = grid.GetCellTypeName(cell_id);

  return cell_type != "wall" && cell_type != "blocked" &&
         cell_type != "goblin_block";
}

std::optional<std::string> ActionFromStep(const WorldPosition &from,
                                          const Point &to_point) {
  const int to_x = static_cast<int>(to_point.x);
  const int to_y = static_cast<int>(to_point.y);

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

WorldView BuildWorldView(const WorldGrid &grid) {
  WorldView world_view(static_cast<int>(grid.GetWidth()),
                       static_cast<int>(grid.GetHeight()));

  for (size_t y = 0; y < grid.GetHeight(); ++y) {
    for (size_t x = 0; x < grid.GetWidth(); ++x) {
      WorldPosition pos(x, y);
      if (!IsWalkableCell(grid, pos)) {
        world_view.SetBlocked(
            StateGridPosition(static_cast<int>(x), static_cast<int>(y)));
      }
    }
  }

  return world_view;
}

} // namespace

size_t EnemyAgent::SelectAction(WorldGrid &grid) {
  const WorldPosition my_pos = GetLocation().AsWorldPosition();

  // 1) Find player target
  std::optional<WorldPosition> player_pos;

  std::vector<size_t> known_agents = world.GetKnownAgents(*this);
  for (size_t agent_id : known_agents) {
    if (agent_id == GetID())
      continue;

    const AgentBase &other = world.GetAgent(agent_id);

    // Temporary target-identification rule:
    // any agent whose name contains target_name
    if (other.GetName().find(target_name) != std::string::npos) {
      player_pos = other.GetLocation().AsWorldPosition();
      break;
    }
  }

  // 2) Chase if player is in vision radius
  if (player_pos.has_value()) {
    const int dist = ManhattanDistance(my_pos, *player_pos);

    if (dist <= static_cast<int>(vision_radius)) {
      // If adjacent, attack if supported by the world
      if (IsAdjacent(my_pos, *player_pos) && HasAction("attack")) {
        return GetActionID("attack");
      }

      // Otherwise pathfind toward the player
      WorldView world_view = BuildWorldView(grid);

      PathGenerator generator;
      generator.SetWorldView(world_view);

      WorldPath path = generator.GenerateShortestPath(
          StateGridPosition(static_cast<int>(my_pos.CellX()),
                            static_cast<int>(my_pos.CellY())),
          StateGridPosition(static_cast<int>(player_pos->CellX()),
                            static_cast<int>(player_pos->CellY())));

      if (path.size() >= 2) {
        auto action_name = ActionFromStep(my_pos, path[1]);
        if (action_name.has_value() && HasAction(*action_name)) {
          return GetActionID(*action_name);
        }
      }
    }
  }

  // 3) Patrol if not chasing
  auto next_pos = GetPatrolNextPos(grid, my_pos, horizontal, direction);

  if (!next_pos.has_value()) {
    // bounce / reverse direction
    direction *= -1;
    next_pos = GetPatrolNextPos(grid, my_pos, horizontal, direction);
  }

  if (next_pos.has_value()) {
    Point next_point{static_cast<double>(next_pos->CellX()),
                     static_cast<double>(next_pos->CellY())};

    auto action_name = ActionFromStep(my_pos, next_point);
    if (action_name.has_value() && HasAction(*action_name)) {
      return GetActionID(*action_name);
    }
  }

  // 4) No valid move
  return 0;
}

} // namespace cse498
