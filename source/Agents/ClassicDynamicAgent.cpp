#include "ClassicDynamicAgent.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../core/WorldBase.hpp"
#include "../tools/BehaviorTree.hpp"
#include "../tools/CompositeNodes.hpp"
#include "../tools/LeafNodes.hpp"
#include "../tools/PathGenerator.hpp"
#include "../Worlds/DynamicWorld.hpp"

namespace cse498 {

/**
 * @class ClassicDynamicAgent
 * @brief Dynamic-world version of ClassicAgent.
 * @note works with current API version of DynamicWorld please contact Matthew Vazquez for API changes to world
 *
 * ClassicDynamicAgent uses local sensing, shared tile knowledge, resource
 * collection, path generation, and leader-only building logic to operate in
 * DynamicWorld. Most decision priority is handled inside Sense() so the agent
 * can strictly prefer building, collecting, resource seeking, exploration, and
 * fallback movement in that order.
 */

enum class Direction {
    Up,
    Down,
    Left,
    Right,
    UpLeft,
    UpRight,
    DownLeft,
    DownRight,
    None
};

namespace {

// -----------------------------------------------------------------------------
// Tunable behavior constants
// -----------------------------------------------------------------------------

constexpr int VISION_RADIUS = 2;

constexpr int MIN_PATH_WITH_MOVE = 2;
constexpr int MIN_PATH_WITH_SKIP = 3;
constexpr size_t FIRST_STEP_INDEX = 1;
constexpr size_t SKIP_STEP_INDEX = 2;

constexpr int TRAIL_LENGTH = 3;
constexpr int TRAIL_PENALTY = 50;
constexpr int CROWN_PENALTY = 10;
constexpr int BUILD_SITE_PENALTY = 8;
constexpr int NON_GRASS_PENALTY = 5;

constexpr int NEAR_EDGE_DISTANCE = 1;
constexpr int EDGE_AVOID_BUFFER = 2;
constexpr int INTERIOR_EDGE_BUFFER = 4;
constexpr int INTERIOR_DISTANCE_WEIGHT_DIVISOR = 4;

constexpr int TOWNHALL_RESOURCE_COST = 500;
constexpr int BASIC_BUILD_COST = 20;
constexpr int SPAWNER_RESOURCE_COST = 30;

constexpr int NO_RESOURCES = 0;
constexpr int INITIAL_STRUCTURE_LIMIT = 1;
constexpr int SECONDARY_STRUCTURE_LIMIT = 2;

// -----------------------------------------------------------------------------
// Cell and position helpers
// -----------------------------------------------------------------------------

// Returns true when two world positions refer to the same grid cell.
bool PositionsMatch(const WorldPosition& a, const WorldPosition& b) {
    return a.CellX() == b.CellX() && a.CellY() == b.CellY();
}

// Returns true when a position is inside the current grid dimensions.
bool IsInBounds(const WorldGrid& grid, const WorldPosition& p) {
    return p.CellX() >= 0 &&
           p.CellY() >= 0 &&
           p.CellX() < static_cast<int>(grid.GetWidth()) &&
           p.CellY() < static_cast<int>(grid.GetHeight());
}

// Returns true for collectible resource cell types in the dynamic world.
bool IsResourceCell(const std::string& cell_type) {
    return cell_type == "tree" ||
           cell_type == "stone" ||
           cell_type == "wheat";
}

// Converts a one-step path delta into the corresponding action direction.
Direction StepToDirection(const Point& a, const Point& b) {
    const int dx = static_cast<int>(b.x - a.x);
    const int dy = static_cast<int>(b.y - a.y);

    if (dx == 0 && dy == -1) return Direction::Up;
    if (dx == 0 && dy == 1)  return Direction::Down;
    if (dx == -1 && dy == 0) return Direction::Left;
    if (dx == 1 && dy == 0)  return Direction::Right;

    if (dx == -1 && dy == -1) return Direction::UpLeft;
    if (dx == 1 && dy == -1)  return Direction::UpRight;
    if (dx == -1 && dy == 1)  return Direction::DownLeft;
    if (dx == 1 && dy == 1)   return Direction::DownRight;

    return Direction::None;
}

// Converts a Direction enum value into the action-map string name.
std::string DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::Up:        return "up";
        case Direction::Down:      return "down";
        case Direction::Left:      return "left";
        case Direction::Right:     return "right";
        case Direction::UpLeft:    return "up_left";
        case Direction::UpRight:   return "up_right";
        case Direction::DownLeft:  return "down_left";
        case Direction::DownRight: return "down_right";
        default:                   return "";
    }
}

// Returns true if the current agent has a named action registered.
bool HasAction(const std::unordered_map<std::string, size_t>& action_map,
               const std::string& action_name) {
    return action_map.find(action_name) != action_map.end();
}

// Returns true when this agent has at least one build action available.
bool CanBuild(const std::unordered_map<std::string, size_t>& action_map) {
    return HasAction(action_map, "build_lumberyard") ||
           HasAction(action_map, "build_quarry") ||
           HasAction(action_map, "build_farm") ||
           HasAction(action_map, "build_spawner") ||
           HasAction(action_map, "build_townhall");
}

// Counts already-discovered structures of a given type in shared knowledge.
int CountKnownStructure(const WorldGrid& grid,
                        const SharedKnowledge& knowledge,
                        const std::string& structure_name) {
    int count = 0;

    for (const auto& [pos, tile] : knowledge.tiles) {
        if (!tile.discovered) continue;
        if (!IsInBounds(grid, pos)) continue;

        const std::string type = grid.GetCellTypeName(grid[pos]);
        if (type == structure_name) {
            ++count;
        }
    }

    return count;
}

// Chooses the highest-priority build action the leader can currently afford.
std::optional<std::string> ChooseBuildAction(
    const DynamicWorld& dworld,
    const WorldGrid& grid,
    const SharedKnowledge& knowledge,
    bool standing_on_grass,
    bool can_build
) {
    if (!can_build || !standing_on_grass) {
        return std::nullopt;
    }

    const int wood  = dworld.GetGlobalCount("wood");
    const int stone = dworld.GetGlobalCount("stone");
    const int steel = dworld.GetGlobalCount("steel");
    const int wheat = dworld.GetGlobalCount("wheat");

    const int known_quarries    = CountKnownStructure(grid, knowledge, "quarry");
    const int known_lumberyards = CountKnownStructure(grid, knowledge, "lumberyard");
    const int known_farms       = CountKnownStructure(grid, knowledge, "farm");
    const int known_spawners    = CountKnownStructure(grid, knowledge, "spawner");
    const int known_townhalls   = CountKnownStructure(grid, knowledge, "townhall");

    if (known_townhalls < INITIAL_STRUCTURE_LIMIT &&
        wood >= TOWNHALL_RESOURCE_COST &&
        stone >= TOWNHALL_RESOURCE_COST &&
        steel >= TOWNHALL_RESOURCE_COST &&
        wheat >= TOWNHALL_RESOURCE_COST) {
        return "build_townhall";
    }

    // First priority: get one quarry online so steel can start appearing.
    if (known_quarries < INITIAL_STRUCTURE_LIMIT &&
        stone >= BASIC_BUILD_COST &&
        wood >= BASIC_BUILD_COST) {
        return "build_quarry";
    }

    // Then diversify the economy instead of repeatedly building quarries.
    if (known_farms < INITIAL_STRUCTURE_LIMIT &&
        wheat >= BASIC_BUILD_COST &&
        wood >= BASIC_BUILD_COST) {
        return "build_farm";
    }

    if (known_spawners < INITIAL_STRUCTURE_LIMIT &&
        stone >= SPAWNER_RESOURCE_COST &&
        wheat >= SPAWNER_RESOURCE_COST) {
        return "build_spawner";
    }

    if (known_lumberyards < INITIAL_STRUCTURE_LIMIT &&
        wood >= BASIC_BUILD_COST &&
        steel >= BASIC_BUILD_COST) {
        return "build_lumberyard";
    }

    // If steel is still unavailable, allow one additional quarry.
    if (known_quarries < SECONDARY_STRUCTURE_LIMIT &&
        steel == NO_RESOURCES &&
        stone >= BASIC_BUILD_COST &&
        wood >= BASIC_BUILD_COST) {
        return "build_quarry";
    }

    // Secondary build goals once each basic structure exists.
    if (known_farms < SECONDARY_STRUCTURE_LIMIT &&
        wheat >= BASIC_BUILD_COST &&
        wood >= BASIC_BUILD_COST) {
        return "build_farm";
    }

    if (known_spawners < SECONDARY_STRUCTURE_LIMIT &&
        stone >= SPAWNER_RESOURCE_COST &&
        wheat >= SPAWNER_RESOURCE_COST) {
        return "build_spawner";
    }

    if (known_lumberyards < SECONDARY_STRUCTURE_LIMIT &&
        wood >= BASIC_BUILD_COST &&
        steel >= BASIC_BUILD_COST) {
        return "build_lumberyard";
    }

    return std::nullopt;
}

// Returns true when global inventory can afford at least one build action.
bool HasEnoughToBuildSomething(const DynamicWorld& dworld) {
    const int wood  = dworld.GetGlobalCount("wood");
    const int stone = dworld.GetGlobalCount("stone");
    const int steel = dworld.GetGlobalCount("steel");
    const int wheat = dworld.GetGlobalCount("wheat");

    return (wood >= TOWNHALL_RESOURCE_COST &&
            stone >= TOWNHALL_RESOURCE_COST &&
            steel >= TOWNHALL_RESOURCE_COST &&
            wheat >= TOWNHALL_RESOURCE_COST) ||

           (stone >= BASIC_BUILD_COST &&
            wood >= BASIC_BUILD_COST) ||

           (wood >= BASIC_BUILD_COST &&
            steel >= BASIC_BUILD_COST) ||

           (wheat >= BASIC_BUILD_COST &&
            wood >= BASIC_BUILD_COST) ||

           (stone >= SPAWNER_RESOURCE_COST &&
            wheat >= SPAWNER_RESOURCE_COST);
}

// Reads the recent-position trail from the behavior tree blackboard.
std::vector<WorldPosition> ReadTrail(const Blackboard& bb) {
    std::vector<WorldPosition> trail;

    for (int i = 0; i < TRAIL_LENGTH; ++i) {
        auto x_it = bb.find("trail" + std::to_string(i) + "_x");
        auto y_it = bb.find("trail" + std::to_string(i) + "_y");

        if (x_it != bb.end() && y_it != bb.end() &&
            std::holds_alternative<int>(x_it->second) &&
            std::holds_alternative<int>(y_it->second)) {
            trail.emplace_back(
                std::get<int>(x_it->second),
                std::get<int>(y_it->second)
            );
        }
    }

    return trail;
}

// Returns true if pos appears in the recent-position trail.
bool InTrail(const WorldPosition& pos, const std::vector<WorldPosition>& trail) {
    for (const auto& t : trail) {
        if (PositionsMatch(pos, t)) {
            return true;
        }
    }
    return false;
}

// Stores a short recent-position trail to discourage backtracking loops.
void PushTrail(BehaviorTree& tree, const WorldPosition& current_pos, const Blackboard& bb) {
    int old0x = current_pos.CellX();
    int old0y = current_pos.CellY();
    int old1x = current_pos.CellX();
    int old1y = current_pos.CellY();

    auto t0x = bb.find("trail0_x");
    auto t0y = bb.find("trail0_y");
    auto t1x = bb.find("trail1_x");
    auto t1y = bb.find("trail1_y");

    if (t0x != bb.end() && t0y != bb.end() &&
        std::holds_alternative<int>(t0x->second) &&
        std::holds_alternative<int>(t0y->second)) {
        old0x = std::get<int>(t0x->second);
        old0y = std::get<int>(t0y->second);
    }

    if (t1x != bb.end() && t1y != bb.end() &&
        std::holds_alternative<int>(t1x->second) &&
        std::holds_alternative<int>(t1y->second)) {
        old1x = std::get<int>(t1x->second);
        old1y = std::get<int>(t1y->second);
    }

    tree.setMemory("trail2_x", BBValue(std::in_place_type<int>, old1x));
    tree.setMemory("trail2_y", BBValue(std::in_place_type<int>, old1y));
    tree.setMemory("trail1_x", BBValue(std::in_place_type<int>, old0x));
    tree.setMemory("trail1_y", BBValue(std::in_place_type<int>, old0y));
    tree.setMemory("trail0_x", BBValue(std::in_place_type<int>, current_pos.CellX()));
    tree.setMemory("trail0_y", BBValue(std::in_place_type<int>, current_pos.CellY()));
}

// Clears the persistent build target from blackboard memory.
void ClearBuildTarget(BehaviorTree& tree) {
    tree.setMemory("has_build_target", BBValue(std::in_place_type<bool>, false));
}

// Reads the persistent build target, if one exists and has valid blackboard types.
std::optional<WorldPosition> ReadBuildTarget(const Blackboard& bb) {
    auto has_it = bb.find("has_build_target");
    auto x_it = bb.find("build_target_x");
    auto y_it = bb.find("build_target_y");

    if (has_it == bb.end() || x_it == bb.end() || y_it == bb.end()) {
        return std::nullopt;
    }

    if (!std::holds_alternative<bool>(has_it->second) ||
        !std::holds_alternative<int>(x_it->second) ||
        !std::holds_alternative<int>(y_it->second)) {
        return std::nullopt;
    }

    if (!std::get<bool>(has_it->second)) {
        return std::nullopt;
    }

    return WorldPosition(
        std::get<int>(x_it->second),
        std::get<int>(y_it->second)
    );
}

// Saves a persistent grass build target to blackboard memory.
void SaveBuildTarget(BehaviorTree& tree, const WorldPosition& pos) {
    tree.setMemory("has_build_target", BBValue(std::in_place_type<bool>, true));
    tree.setMemory("build_target_x", BBValue(std::in_place_type<int>, pos.CellX()));
    tree.setMemory("build_target_y", BBValue(std::in_place_type<int>, pos.CellY()));
}

// Converts a WorldPath point at idx into a WorldPosition.
WorldPosition NextStepWorldPos(const WorldPath& path, size_t idx = FIRST_STEP_INDEX) {
    return WorldPosition(
        static_cast<int>(path[idx].x),
        static_cast<int>(path[idx].y)
    );
}

// Finds a known walkable tile away from edges to help agents escape edge loops.
std::optional<WorldPosition> FindInteriorTarget(
    const WorldGrid& grid,
    const SharedKnowledge& knowledge,
    const WorldPosition& start_pos
) {
    std::vector<WorldPosition> candidates;

    const int cx = static_cast<int>(grid.GetWidth()) / 2;
    const int cy = static_cast<int>(grid.GetHeight()) / 2;

    for (const auto& [pos, tile] : knowledge.tiles) {
        if (!tile.walkable_known || !tile.is_walkable) continue;
        if (!tile.discovered) continue;
        if (!IsInBounds(grid, pos)) continue;
        if (PositionsMatch(pos, start_pos)) continue;

        const int px = static_cast<int>(pos.CellX());
        const int py = static_cast<int>(pos.CellY());

        const int edge_dist = std::min({
            px,
            py,
            static_cast<int>(grid.GetWidth()) - 1 - px,
            static_cast<int>(grid.GetHeight()) - 1 - py
        });

        if (edge_dist < INTERIOR_EDGE_BUFFER) continue;

        candidates.push_back(pos);
    }

    if (candidates.empty()) {
        return std::nullopt;
    }

    auto best = candidates.front();
    int best_score = std::numeric_limits<int>::max();

    for (const auto& pos : candidates) {
        const int px = static_cast<int>(pos.CellX());
        const int py = static_cast<int>(pos.CellY());
        const int sx = static_cast<int>(start_pos.CellX());
        const int sy = static_cast<int>(start_pos.CellY());

        const int dist_to_center = std::abs(px - cx) + std::abs(py - cy);
        const int dist_from_start = std::abs(px - sx) + std::abs(py - sy);
        const int score = dist_to_center + dist_from_start / INTERIOR_DISTANCE_WEIGHT_DIVISOR;

        if (score < best_score) {
            best_score = score;
            best = pos;
        }
    }

    return best;
}

// Returns positions currently occupied by all other agents.
std::vector<WorldPosition> GetOccupiedPositions(const WorldBase& world, size_t self_id) {
    std::vector<WorldPosition> occupied;

    for (size_t i = 0; i < world.GetNumAgents(); ++i) {
        const auto& other = world.GetAgent(i);
        if (other.GetID() == self_id) continue;
        occupied.push_back(other.GetLocation().AsWorldPosition());
    }

    return occupied;
}

// Returns true if pos is occupied by another agent.
bool IsOccupied(const WorldPosition& pos, const std::vector<WorldPosition>& occupied) {
    for (const auto& o : occupied) {
        if (PositionsMatch(pos, o)) return true;
    }
    return false;
}

// Counts nearby occupied cells to discourage agents from clustering together.
int CountOccupiedNeighbors(const WorldPosition& pos, const std::vector<WorldPosition>& occupied) {
    int count = 0;
    const int px = static_cast<int>(pos.CellX());
    const int py = static_cast<int>(pos.CellY());

    for (const auto& o : occupied) {
        const int ox = static_cast<int>(o.CellX());
        const int oy = static_cast<int>(o.CellY());

        if (std::abs(ox - px) <= NEAR_EDGE_DISTANCE &&
            std::abs(oy - py) <= NEAR_EDGE_DISTANCE) {
            ++count;
        }
    }
    return count;
}

// Builds all adjacent movement options, rotated by agent id to reduce crowding.
std::vector<std::pair<std::string, WorldPosition>>
BuildOrderedMoves(const WorldPosition& my_pos, size_t agent_id) {
    std::vector<std::pair<std::string, WorldPosition>> moves = {
        {"up", my_pos.Up()},
        {"down", my_pos.Down()},
        {"left", my_pos.Left()},
        {"right", my_pos.Right()},
        {"up_left", my_pos.Up().Left()},
        {"up_right", my_pos.Up().Right()},
        {"down_left", my_pos.Down().Left()},
        {"down_right", my_pos.Down().Right()}
    };

    const size_t rotation = agent_id % moves.size();
    std::rotate(moves.begin(), moves.begin() + rotation, moves.end());
    return moves;
}

// Finds the best discovered grass tile for future building.
std::optional<WorldPosition> FindNearestGrassBuildSite(
    const WorldGrid& grid,
    const SharedKnowledge& shared,
    const WorldPosition& my_pos,
    const std::vector<WorldPosition>& occupied
) {
    std::optional<WorldPosition> best;
    int best_score = std::numeric_limits<int>::max();

    for (const auto& [pos, tile] : shared.tiles) {
        if (!tile.discovered) continue;
        if (!tile.walkable_known || !tile.is_walkable) continue;
        if (!IsInBounds(grid, pos)) continue;
        if (IsOccupied(pos, occupied)) continue;
        if (PositionsMatch(pos, my_pos)) continue;

        const std::string type = grid.GetCellTypeName(grid[pos]);
        if (type != "grass") continue;

        const int dist =
            std::abs(static_cast<int>(pos.CellX()) - static_cast<int>(my_pos.CellX())) +
            std::abs(static_cast<int>(pos.CellY()) - static_cast<int>(my_pos.CellY()));

        const int crowd_penalty = CountOccupiedNeighbors(pos, occupied) * BUILD_SITE_PENALTY;
        const int score = dist + crowd_penalty;

        if (score < best_score) {
            best_score = score;
            best = pos;
        }
    }

    return best;
}

} // namespace

void ClassicDynamicAgent::BuildTree() {
    auto root = std::make_shared<Selector>("Root");
    tree.setRoot(root);
}

void ClassicDynamicAgent::Sense(WorldGrid& grid) {
    tree.setMemory("chosen_action", BBValue(std::in_place_type<std::string>, ""));

    WorldPosition my_pos = this->GetLocation().AsWorldPosition();
    Blackboard bb = tree.getBlackboard();
    auto trail = ReadTrail(bb);
    auto& shared = grid.GetSharedKnowledge();
    auto occupied = GetOccupiedPositions(world, this->GetID());

    bool material_on_current_tile = false;
    bool standing_on_grass = false;
    int current_turn = 0;

    if (const auto* dworld_for_turn = dynamic_cast<const DynamicWorld*>(&world)) {
        current_turn = static_cast<int>(dworld_for_turn->GetUpdateCounter());
    }

    // Clear stale visibility/enemy flags in the local sensor window before
    // repopulating current observations.
    for (int dy = -VISION_RADIUS; dy <= VISION_RADIUS; ++dy) {
        for (int dx = -VISION_RADIUS; dx <= VISION_RADIUS; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.currently_visible = false;
            tile.has_enemy = false;
        }
    }

    // Sense nearby terrain and update shared knowledge for discovered cells.
    for (int dy = -VISION_RADIUS; dy <= VISION_RADIUS; ++dy) {
        for (int dx = -VISION_RADIUS; dx <= VISION_RADIUS; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.walkable_known = true;
            tile.last_seen_turn = current_turn;

            const std::string type = grid.GetCellTypeName(grid[p]);

            tile.is_walkable =
                (type != "wall" &&
                 type != "blocked" &&
                 type != "quarry" &&
                 type != "lumberyard" &&
                 type != "farm" &&
                 type != "spawner" &&
                 type != "townhall");

            tile.has_resource = IsResourceCell(type);

            if (PositionsMatch(my_pos, p)) {
                if (type == "grass") {
                    standing_on_grass = true;
                }
                if (tile.has_resource) {
                    material_on_current_tile = true;
                }
            }
        }
    }

    // Items reported by the world are also treated as visible resources.
    std::vector<size_t> visible_items = world.GetKnownItems(*this);
    for (size_t item_id : visible_items) {
        const auto& item = world.GetItem(item_id);
        WorldPosition item_pos = item.GetLocation().AsWorldPosition();
        if (!IsInBounds(grid, item_pos)) continue;

        TileKnowledge& tile = shared.GetTile(item_pos);
        tile.discovered = true;
        tile.currently_visible = true;
        tile.has_resource = true;
        tile.walkable_known = true;
        tile.is_walkable = true;
        tile.last_seen_turn = current_turn;

        if (PositionsMatch(my_pos, item_pos)) {
            material_on_current_tile = true;
        }
    }

    const auto adjacent_moves = BuildOrderedMoves(my_pos, this->GetID());
    PathGenerator gen;
    StateGridPosition start(my_pos.CellX(), my_pos.CellY());

    const bool can_build = CanBuild(action_map);

    // std::cout << "[Sense] agent=" << GetName()
    //           << " id=" << GetID()
    //           << " can_build=" << can_build
    //           << " on_grass=" << standing_on_grass
    //           << " material_here=" << material_on_current_tile
    //           << std::endl;

    // if (GetName() == "Leader") {
    //     std::cout << "[Leader action_map contents]" << std::endl;
    //     for (const auto& [name, id] : action_map) {
    //         std::cout << "  " << name << " -> " << id << std::endl;
    //     }
    // }

    // Leader/build-capable agents prioritize building when inventory is ready.
    if (can_build) {
        if (const auto* dworld = dynamic_cast<const DynamicWorld*>(&world)) {
            // std::cout << "[Builder " << GetID() << "]"
            //           << " grass=" << standing_on_grass
            //           << " wood=" << dworld->GetGlobalCount("wood")
            //           << " stone=" << dworld->GetGlobalCount("stone")
            //           << " steel=" << dworld->GetGlobalCount("steel")
            //           << " wheat=" << dworld->GetGlobalCount("wheat")
            //           << std::endl;

            const bool ready_to_build = HasEnoughToBuildSomething(*dworld);

            if (ready_to_build) {
                auto build_action = ChooseBuildAction(*dworld, grid, shared, standing_on_grass, can_build);

                if (build_action.has_value()) {
                    // std::cout << "[Builder " << GetID() << "] choosing "
                    //           << *build_action << std::endl;

                    ClearBuildTarget(tree);
                    tree.setMemory("chosen_action",
                        BBValue(std::in_place_type<std::string>, *build_action));
                    PushTrail(tree, my_pos, bb);
                    return;
                }

                // Keep or acquire a persistent grass build target.
                auto build_target = ReadBuildTarget(bb);

                bool target_invalid = false;
                if (build_target.has_value()) {
                    if (!IsInBounds(grid, *build_target)) {
                        target_invalid = true;
                    } else {
                        const std::string type = grid.GetCellTypeName(grid[*build_target]);
                        if (type != "grass") {
                            target_invalid = true;
                        }
                    }
                }

                if (!build_target.has_value() || target_invalid) {
                    build_target = FindNearestGrassBuildSite(grid, shared, my_pos, occupied);
                    if (build_target.has_value()) {
                        SaveBuildTarget(tree, *build_target);
                        // std::cout << "[Builder " << GetID()
                        //           << "] new build target=("
                        //           << build_target->CellX() << ","
                        //           << build_target->CellY() << ")"
                        //           << std::endl;
                    } else {
                        ClearBuildTarget(tree);
                    }
                }

                // Prefer nearby grass when moving toward a build target.
                std::optional<std::string> best_grass_move;
                int best_grass_score = std::numeric_limits<int>::max();

                for (const auto& [move_name, pos] : adjacent_moves) {
                    if (!IsInBounds(grid, pos)) continue;
                    if (IsOccupied(pos, occupied)) continue;

                    const std::string type = grid.GetCellTypeName(grid[pos]);
                    if (type != "grass") continue;

                    int score = 0;
                    if (InTrail(pos, trail)) score += TRAIL_PENALTY;
                    score += CountOccupiedNeighbors(pos, occupied) * CROWN_PENALTY;

                    if (build_target.has_value()) {
                        const int dx = static_cast<int>(pos.CellX()) -
                                       static_cast<int>(build_target->CellX());
                        const int dy = static_cast<int>(pos.CellY()) -
                                       static_cast<int>(build_target->CellY());

                        score += std::abs(dx) + std::abs(dy);
                    }

                    if (score < best_grass_score) {
                        best_grass_score = score;
                        best_grass_move = move_name;
                    }
                }

                if (best_grass_move.has_value()) {
                    // std::cout << "[Builder " << GetID()
                    //           << "] moving to adjacent grass to build via "
                    //           << *best_grass_move << std::endl;

                    tree.setMemory("chosen_action",
                        BBValue(std::in_place_type<std::string>, *best_grass_move));
                    PushTrail(tree, my_pos, bb);
                    return;
                }

                // Otherwise path to the remembered grass build target.
                if (build_target.has_value()) {
                    WorldPath build_path = gen.GeneratePathToKnownTile(
                        start,
                        StateGridPosition(build_target->CellX(), build_target->CellY()),
                        shared,
                        std::nullopt
                    );

                    if (build_path.size() >= MIN_PATH_WITH_MOVE) {
                        size_t next_index = FIRST_STEP_INDEX;
                        WorldPosition next_wp = NextStepWorldPos(build_path, next_index);

                        if (InTrail(next_wp, trail) && build_path.size() >= MIN_PATH_WITH_SKIP) {
                            WorldPosition alt = NextStepWorldPos(build_path, SKIP_STEP_INDEX);
                            if (!IsOccupied(alt, occupied) && !InTrail(alt, trail)) {
                                next_index = SKIP_STEP_INDEX;
                                next_wp = alt;
                            }
                        }

                        if (IsOccupied(next_wp, occupied) && build_path.size() >= MIN_PATH_WITH_SKIP) {
                            WorldPosition alt = NextStepWorldPos(build_path, SKIP_STEP_INDEX);
                            if (!IsOccupied(alt, occupied)) {
                                next_index = SKIP_STEP_INDEX;
                                next_wp = alt;
                            }
                        }

                        if (!IsOccupied(next_wp, occupied)) {
                            std::string move = DirectionToString(
                                StepToDirection(build_path[0], build_path[next_index])
                            );

                            if (!move.empty()) {
                                // std::cout << "[Builder " << GetID()
                                //           << "] pathing to build target via "
                                //           << move << std::endl;

                                tree.setMemory("chosen_action",
                                    BBValue(std::in_place_type<std::string>, move));
                                PushTrail(tree, my_pos, bb);
                                return;
                            }
                        }
                    }

                    // If the target is not reachable anymore, try again next turn.
                    ClearBuildTarget(tree);
                }

                // Last-resort movement while in build mode.
                std::optional<std::string> emergency_build_move;
                int emergency_build_score = std::numeric_limits<int>::max();

                for (const auto& [move_name, pos] : adjacent_moves) {
                    if (!IsInBounds(grid, pos)) continue;
                    if (IsOccupied(pos, occupied)) continue;

                    TileKnowledge& tile = shared.GetTile(pos);
                    if (!(tile.walkable_known && tile.is_walkable)) continue;

                    int score = 0;
                    if (InTrail(pos, trail)) score += TRAIL_PENALTY;
                    score += CountOccupiedNeighbors(pos, occupied) * CROWN_PENALTY;

                    const std::string type = grid.GetCellTypeName(grid[pos]);
                    if (type != "grass") score += NON_GRASS_PENALTY;

                    if (score < emergency_build_score) {
                        emergency_build_score = score;
                        emergency_build_move = move_name;
                    }
                }

                if (emergency_build_move.has_value()) {
                    // std::cout << "[Builder " << GetID()
                    //           << "] emergency build-mode move via "
                    //           << *emergency_build_move << std::endl;

                    tree.setMemory("chosen_action",
                        BBValue(std::in_place_type<std::string>, *emergency_build_move));
                    PushTrail(tree, my_pos, bb);
                    return;
                }
            } else {
                // Not ready to build yet; clear stale target.
                ClearBuildTarget(tree);
            }
        }
    }

    // Collect immediately if standing on a resource.
    if (material_on_current_tile) {
        tree.setMemory("chosen_action",
            BBValue(std::in_place_type<std::string>, "collect"));
        PushTrail(tree, my_pos, bb);
        return;
    }

    // Prefer adjacent visible resources before computing a longer path.
    std::optional<std::string> fallback_adjacent_move;
    int fallback_score = std::numeric_limits<int>::max();

    for (const auto& [move_name, pos] : adjacent_moves) {
        if (!IsInBounds(grid, pos)) continue;
        if (IsOccupied(pos, occupied)) continue;

        TileKnowledge& tile = shared.GetTile(pos);
        if (!(tile.has_resource && tile.walkable_known && tile.is_walkable)) {
            continue;
        }

        int score = 0;
        if (InTrail(pos, trail)) score += TRAIL_PENALTY;
        score += CountOccupiedNeighbors(pos, occupied) * CROWN_PENALTY;

        if (score < fallback_score) {
            fallback_score = score;
            fallback_adjacent_move = move_name;
        }
    }

    if (fallback_adjacent_move.has_value()) {
        tree.setMemory("chosen_action",
            BBValue(std::in_place_type<std::string>, *fallback_adjacent_move));
        PushTrail(tree, my_pos, bb);
        return;
    }

    // Path priority: known resource, unexplored frontier, then interior target.
    WorldPath chosen_path;

    WorldPath resource_path = gen.GenerateResourcePath(start, shared, std::nullopt);
    if (resource_path.size() >= MIN_PATH_WITH_MOVE) {
        chosen_path = resource_path;
    }

    if (chosen_path.size() < MIN_PATH_WITH_MOVE) {
        WorldPath explore_path = gen.GenerateExplorePath(
            start,
            shared,
            static_cast<int>(grid.GetWidth()),
            static_cast<int>(grid.GetHeight()),
            std::nullopt
        );

        if (explore_path.size() >= MIN_PATH_WITH_MOVE) {
            chosen_path = explore_path;
        }
    }

    if (chosen_path.size() < MIN_PATH_WITH_MOVE) {
        auto target = FindInteriorTarget(grid, shared, my_pos);
        if (target.has_value()) {
            WorldPath interior_path = gen.GeneratePathToKnownTile(
                start,
                StateGridPosition(target->CellX(), target->CellY()),
                shared,
                std::nullopt
            );

            if (interior_path.size() >= MIN_PATH_WITH_MOVE) {
                chosen_path = interior_path;
            }
        }
    }

    if (chosen_path.size() >= MIN_PATH_WITH_MOVE) {
        size_t next_index = FIRST_STEP_INDEX;

        auto IsNearEdge = [&](const WorldPosition& p) {
            return p.CellX() <= NEAR_EDGE_DISTANCE ||
                   p.CellY() <= NEAR_EDGE_DISTANCE ||
                   p.CellX() >= static_cast<int>(grid.GetWidth()) - EDGE_AVOID_BUFFER ||
                   p.CellY() >= static_cast<int>(grid.GetHeight()) - EDGE_AVOID_BUFFER;
        };

        if (InTrail(NextStepWorldPos(chosen_path, FIRST_STEP_INDEX), trail) &&
            chosen_path.size() >= MIN_PATH_WITH_SKIP) {
            WorldPosition alt = NextStepWorldPos(chosen_path, SKIP_STEP_INDEX);
            if (!InTrail(alt, trail) && !IsOccupied(alt, occupied)) {
                next_index = SKIP_STEP_INDEX;
            }
        }

        WorldPosition next_wp = NextStepWorldPos(chosen_path, next_index);

        if (IsOccupied(next_wp, occupied) && chosen_path.size() >= MIN_PATH_WITH_SKIP) {
            WorldPosition alt = NextStepWorldPos(chosen_path, SKIP_STEP_INDEX);
            if (!IsOccupied(alt, occupied)) {
                next_index = SKIP_STEP_INDEX;
                next_wp = alt;
            }
        }

        if (IsNearEdge(my_pos) && IsNearEdge(next_wp) && chosen_path.size() >= MIN_PATH_WITH_SKIP) {
            WorldPosition alt = NextStepWorldPos(chosen_path, SKIP_STEP_INDEX);
            if (!InTrail(alt, trail) && !IsOccupied(alt, occupied)) {
                next_index = SKIP_STEP_INDEX;
                next_wp = alt;
            }
        }

        if (!IsOccupied(next_wp, occupied)) {
            const Point& a = chosen_path[0];
            Point b = chosen_path[next_index];

            std::string move = DirectionToString(StepToDirection(a, b));
            if (!move.empty()) {
                tree.setMemory("chosen_action",
                    BBValue(std::in_place_type<std::string>, move));
                PushTrail(tree, my_pos, bb);
                return;
            }
        }
    }

    // Deterministic emergency fallback: choose the lowest-score walkable move.
    std::optional<std::string> emergency_fallback;
    int emergency_score = std::numeric_limits<int>::max();

    for (const auto& [move_name, pos] : adjacent_moves) {
        if (!IsInBounds(grid, pos)) continue;
        if (IsOccupied(pos, occupied)) continue;

        TileKnowledge& tile = shared.GetTile(pos);
        if (!(tile.walkable_known && tile.is_walkable)) {
            continue;
        }

        int score = 0;
        if (InTrail(pos, trail)) score += TRAIL_PENALTY;
        score += CountOccupiedNeighbors(pos, occupied) * CROWN_PENALTY;

        if (score < emergency_score) {
            emergency_score = score;
            emergency_fallback = move_name;
        }
    }

    if (emergency_fallback.has_value()) {
        tree.setMemory("chosen_action",
            BBValue(std::in_place_type<std::string>, *emergency_fallback));
        PushTrail(tree, my_pos, bb);
        return;
    }

    // Random valid fallback prevents the leader from getting permanently stuck
    // when all preferred movement heuristics fail.
    std::vector<std::string> random_valid_moves;

    for (const auto& [move_name, pos] : adjacent_moves) {
        if (!IsInBounds(grid, pos)) continue;
        if (IsOccupied(pos, occupied)) continue;

        TileKnowledge& tile = shared.GetTile(pos);
        if (!(tile.walkable_known && tile.is_walkable)) continue;

        random_valid_moves.push_back(move_name);
    }

    if (!random_valid_moves.empty()) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, random_valid_moves.size() - 1);

        tree.setMemory("chosen_action",
            BBValue(std::in_place_type<std::string>, random_valid_moves[dist(rng)]));
        PushTrail(tree, my_pos, bb);
        return;
    }

    // Absolute last resort if truly boxed in.
    tree.setMemory("chosen_action",
        BBValue(std::in_place_type<std::string>, "remain_still"));
}

size_t ClassicDynamicAgent::GetAction() const {
    Blackboard bb = tree.getBlackboard();

    auto it = bb.find("chosen_action");
    if (it == bb.end()) return 0;
    if (!std::holds_alternative<std::string>(it->second)) return 0;

    const std::string& action_name = std::get<std::string>(it->second);
    auto action_it = action_map.find(action_name);
    if (action_it == action_map.end()) return 0;

    return action_it->second;
}

} // namespace cse498
