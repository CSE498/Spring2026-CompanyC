#include "ClassicDynamicAgent.hpp"

#include <memory>
#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <random>
#include <limits>
#include <algorithm>

#include "../tools/CompositeNodes.hpp"
#include "../tools/LeafNodes.hpp"
#include "../tools/BehaviorTree.hpp"
#include "../core/WorldBase.hpp"
#include "../tools/PathGenerator.hpp"
#include "../Worlds/DynamicWorld.hpp"

namespace cse498 {

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

int IntAbs(int x) {
    return (x < 0) ? -x : x;
}

bool PositionsMatch(const WorldPosition& a, const WorldPosition& b) {
    return a.CellX() == b.CellX() && a.CellY() == b.CellY();
}

bool IsInBounds(const WorldGrid& grid, const WorldPosition& p) {
    return p.CellX() >= 0 &&
           p.CellY() >= 0 &&
           p.CellX() < static_cast<int>(grid.GetWidth()) &&
           p.CellY() < static_cast<int>(grid.GetHeight());
}

bool IsResourceCell(const std::string& cell_type) {
    return cell_type == "tree" ||
           cell_type == "stone" ||
           cell_type == "wheat";
}

bool IsStructureCell(const std::string& cell_type) {
    return cell_type == "quarry" ||
           cell_type == "lumberyard" ||
           cell_type == "farm" ||
           cell_type == "spawner" ||
           cell_type == "townhall";
}

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

bool HasAction(const std::unordered_map<std::string, size_t>& action_map,
               const std::string& action_name) {
    return action_map.find(action_name) != action_map.end();
}

bool CanBuild(const std::unordered_map<std::string, size_t>& action_map) {
    return HasAction(action_map, "build_lumberyard") ||
           HasAction(action_map, "build_quarry") ||
           HasAction(action_map, "build_farm") ||
           HasAction(action_map, "build_spawner") ||
           HasAction(action_map, "build_townhall");
}

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

    const int known_quarries     = CountKnownStructure(grid, knowledge, "quarry");
    const int known_lumberyards  = CountKnownStructure(grid, knowledge, "lumberyard");
    const int known_farms        = CountKnownStructure(grid, knowledge, "farm");
    const int known_spawners     = CountKnownStructure(grid, knowledge, "spawner");
    const int known_townhalls    = CountKnownStructure(grid, knowledge, "townhall");

    if (known_townhalls < 1 &&
        wood >= 500 && stone >= 500 && steel >= 500 && wheat >= 500) {
        return "build_townhall";
    }

    // First priority: get at least one quarry online if we have no steel.
    if (known_quarries < 1 &&
        stone >= 20 && wood >= 20) {
        return "build_quarry";
    }

    // Then diversify instead of spamming quarries.
    if (known_farms < 1 &&
        wheat >= 20 && wood >= 20) {
        return "build_farm";
    }

    if (known_spawners < 1 &&
        stone >= 30 && wheat >= 30) {
        return "build_spawner";
    }

    if (known_lumberyards < 1 &&
        wood >= 20 && steel >= 20) {
        return "build_lumberyard";
    }

    // If still no steel is appearing, allow one more quarry max.
    if (known_quarries < 2 &&
        steel == 0 &&
        stone >= 20 && wood >= 20) {
        return "build_quarry";
    }

    // Sensible fallbacks once basics exist.
    if (known_farms < 2 &&
        wheat >= 20 && wood >= 20) {
        return "build_farm";
    }

    if (known_spawners < 2 &&
        stone >= 30 && wheat >= 30) {
        return "build_spawner";
    }

    if (known_lumberyards < 2 &&
        wood >= 20 && steel >= 20) {
        return "build_lumberyard";
    }

    return std::nullopt;
}

bool HasEnoughToBuildSomething(const DynamicWorld& dworld) {
    const int wood  = dworld.GetGlobalCount("wood");
    const int stone = dworld.GetGlobalCount("stone");
    const int steel = dworld.GetGlobalCount("steel");
    const int wheat = dworld.GetGlobalCount("wheat");

    return (wood >= 500 && stone >= 500 && steel >= 500 && wheat >= 500) ||
           (stone >= 20 && wood >= 20) ||
           (wood >= 20 && steel >= 20) ||
           (wheat >= 20 && wood >= 20) ||
           (stone >= 30 && wheat >= 30);
}

std::vector<WorldPosition> ReadTrail(const Blackboard& bb) {
    std::vector<WorldPosition> trail;

    for (int i = 0; i < 3; ++i) {
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

bool InTrail(const WorldPosition& pos, const std::vector<WorldPosition>& trail) {
    for (const auto& t : trail) {
        if (PositionsMatch(pos, t)) {
            return true;
        }
    }
    return false;
}

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

void ClearBuildTarget(BehaviorTree& tree) {
    tree.setMemory("has_build_target", BBValue(std::in_place_type<bool>, false));
}

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

void SaveBuildTarget(BehaviorTree& tree, const WorldPosition& pos) {
    tree.setMemory("has_build_target", BBValue(std::in_place_type<bool>, true));
    tree.setMemory("build_target_x", BBValue(std::in_place_type<int>, pos.CellX()));
    tree.setMemory("build_target_y", BBValue(std::in_place_type<int>, pos.CellY()));
}

WorldPosition NextStepWorldPos(const WorldPath& path, size_t idx = 1) {
    return WorldPosition(
        static_cast<int>(path[idx].x),
        static_cast<int>(path[idx].y)
    );
}

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

        int edge_dist = std::min({
            px,
            py,
            static_cast<int>(grid.GetWidth()) - 1 - px,
            static_cast<int>(grid.GetHeight()) - 1 - py
        });

        if (edge_dist < 4) continue;

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

        int dist_to_center = IntAbs(px - cx) + IntAbs(py - cy);
        int dist_from_start = IntAbs(px - sx) + IntAbs(py - sy);

        int score = dist_to_center + dist_from_start / 4;

        if (score < best_score) {
            best_score = score;
            best = pos;
        }
    }

    return best;
}

std::vector<WorldPosition> GetOccupiedPositions(const WorldBase& world, size_t self_id) {
    std::vector<WorldPosition> occupied;

    for (size_t i = 0; i < world.GetNumAgents(); ++i) {
        const auto& other = world.GetAgent(i);
        if (other.GetID() == self_id) continue;
        occupied.push_back(other.GetLocation().AsWorldPosition());
    }

    return occupied;
}

bool IsOccupied(const WorldPosition& pos, const std::vector<WorldPosition>& occupied) {
    for (const auto& o : occupied) {
        if (PositionsMatch(pos, o)) return true;
    }
    return false;
}

int CountOccupiedNeighbors(const WorldPosition& pos, const std::vector<WorldPosition>& occupied) {
    int count = 0;
    const int px = static_cast<int>(pos.CellX());
    const int py = static_cast<int>(pos.CellY());

    for (const auto& o : occupied) {
        const int ox = static_cast<int>(o.CellX());
        const int oy = static_cast<int>(o.CellY());

        if (IntAbs(ox - px) <= 1 && IntAbs(oy - py) <= 1) {
            ++count;
        }
    }
    return count;
}

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

    const size_t rot = agent_id % moves.size();
    std::rotate(moves.begin(), moves.begin() + rot, moves.end());
    return moves;
}

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
            IntAbs(static_cast<int>(pos.CellX()) - static_cast<int>(my_pos.CellX())) +
            IntAbs(static_cast<int>(pos.CellY()) - static_cast<int>(my_pos.CellY()));

        const int crowd_penalty = CountOccupiedNeighbors(pos, occupied) * 8;
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

    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.currently_visible = false;
            tile.has_enemy = false;
        }
    }

    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.walkable_known = true;
            tile.last_seen_turn = current_turn;

            std::string type = grid.GetCellTypeName(grid[p]);

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

    std::cout << "[Sense] agent=" << GetName()
              << " id=" << GetID()
              << " can_build=" << can_build
              << " on_grass=" << standing_on_grass
              << " material_here=" << material_on_current_tile
              << std::endl;

    if (GetName() == "Leader") {
        std::cout << "[Leader action_map contents]" << std::endl;
        for (const auto& [name, id] : action_map) {
            std::cout << "  " << name << " -> " << id << std::endl;
        }
    }

    if (can_build) {
        if (const auto* dworld = dynamic_cast<const DynamicWorld*>(&world)) {
            std::cout << "[Builder " << GetID() << "]"
                      << " grass=" << standing_on_grass
                      << " wood=" << dworld->GetGlobalCount("wood")
                      << " stone=" << dworld->GetGlobalCount("stone")
                      << " steel=" << dworld->GetGlobalCount("steel")
                      << " wheat=" << dworld->GetGlobalCount("wheat")
                      << std::endl;

            const bool ready_to_build = HasEnoughToBuildSomething(*dworld);

            // If we are ready to build, stay in build mode and do NOT fall back to collect logic.
            if (ready_to_build) {
                auto build_action = ChooseBuildAction(*dworld, grid, shared, standing_on_grass, can_build);

                if (build_action.has_value()) {
                    std::cout << "[Builder " << GetID() << "] choosing "
                              << *build_action << std::endl;

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
                        std::cout << "[Builder " << GetID()
                                  << "] new build target=("
                                  << build_target->CellX() << ","
                                  << build_target->CellY() << ")"
                                  << std::endl;
                    } else {
                        ClearBuildTarget(tree);
                    }
                }

                // Adjacent grass shortcut.
                std::optional<std::string> best_grass_move;
                int best_grass_score = std::numeric_limits<int>::max();

                for (const auto& [move_name, pos] : adjacent_moves) {
                    if (!IsInBounds(grid, pos)) continue;
                    if (IsOccupied(pos, occupied)) continue;

                    const std::string type = grid.GetCellTypeName(grid[pos]);
                    if (type != "grass") continue;

                    int score = 0;
                    if (InTrail(pos, trail)) score += 50;
                    score += CountOccupiedNeighbors(pos, occupied) * 10;

                    if (build_target.has_value()) {
                        score += IntAbs(pos.CellX() - build_target->CellX()) +
                                 IntAbs(pos.CellY() - build_target->CellY());
                    }

                    if (score < best_grass_score) {
                        best_grass_score = score;
                        best_grass_move = move_name;
                    }
                }

                if (best_grass_move.has_value()) {
                    std::cout << "[Builder " << GetID()
                              << "] moving to adjacent grass to build via "
                              << *best_grass_move << std::endl;

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

                    if (build_path.size() >= 2) {
                        size_t next_index = 1;
                        WorldPosition next_wp = NextStepWorldPos(build_path, next_index);

                        if (InTrail(next_wp, trail) && build_path.size() >= 3) {
                            WorldPosition alt = NextStepWorldPos(build_path, 2);
                            if (!IsOccupied(alt, occupied) && !InTrail(alt, trail)) {
                                next_index = 2;
                                next_wp = alt;
                            }
                        }

                        if (IsOccupied(next_wp, occupied) && build_path.size() >= 3) {
                            WorldPosition alt = NextStepWorldPos(build_path, 2);
                            if (!IsOccupied(alt, occupied)) {
                                next_index = 2;
                                next_wp = alt;
                            }
                        }

                        if (!IsOccupied(next_wp, occupied)) {
                            std::string move = DirectionToString(
                                StepToDirection(build_path[0], build_path[next_index])
                            );

                            if (!move.empty()) {
                                std::cout << "[Builder " << GetID()
                                          << "] pathing to build target via "
                                          << move << std::endl;

                                tree.setMemory("chosen_action",
                                    BBValue(std::in_place_type<std::string>, move));
                                PushTrail(tree, my_pos, bb);
                                return;
                            }
                        }
                    }

                    // If the target is not reachable anymore, clear it and try again next turn.
                    ClearBuildTarget(tree);
                }

                // Last-resort movement while in build mode: move to any safe walkable grass-adjacent/walkable tile.
                std::optional<std::string> emergency_build_move;
                int emergency_build_score = std::numeric_limits<int>::max();

                for (const auto& [move_name, pos] : adjacent_moves) {
                    if (!IsInBounds(grid, pos)) continue;
                    if (IsOccupied(pos, occupied)) continue;

                    TileKnowledge& tile = shared.GetTile(pos);
                    if (!(tile.walkable_known && tile.is_walkable)) continue;

                    int score = 0;
                    if (InTrail(pos, trail)) score += 50;
                    score += CountOccupiedNeighbors(pos, occupied) * 10;

                    const std::string type = grid.GetCellTypeName(grid[pos]);
                    if (type != "grass") score += 5;

                    if (score < emergency_build_score) {
                        emergency_build_score = score;
                        emergency_build_move = move_name;
                    }
                }

                if (emergency_build_move.has_value()) {
                    std::cout << "[Builder " << GetID()
                              << "] emergency build-mode move via "
                              << *emergency_build_move << std::endl;

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

    if (material_on_current_tile) {
        tree.setMemory("chosen_action",
            BBValue(std::in_place_type<std::string>, "collect"));
        PushTrail(tree, my_pos, bb);
        return;
    }

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
        if (InTrail(pos, trail)) score += 50;
        score += CountOccupiedNeighbors(pos, occupied) * 10;

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

    WorldPath chosen_path;

    WorldPath resource_path = gen.GenerateResourcePath(start, shared, std::nullopt);
    if (resource_path.size() >= 2) {
        chosen_path = resource_path;
    }

    if (chosen_path.size() < 2) {
        WorldPath explore_path = gen.GenerateExplorePath(
            start,
            shared,
            static_cast<int>(grid.GetWidth()),
            static_cast<int>(grid.GetHeight()),
            std::nullopt
        );

        if (explore_path.size() >= 2) {
            chosen_path = explore_path;
        }
    }

    if (chosen_path.size() < 2) {
        auto target = FindInteriorTarget(grid, shared, my_pos);
        if (target.has_value()) {
            WorldPath interior_path = gen.GeneratePathToKnownTile(
                start,
                StateGridPosition(target->CellX(), target->CellY()),
                shared,
                std::nullopt
            );

            if (interior_path.size() >= 2) {
                chosen_path = interior_path;
            }
        }
    }

    if (chosen_path.size() >= 2) {
        size_t next_index = 1;

        auto IsNearEdge = [&](const WorldPosition& p) {
            return p.CellX() <= 1 ||
                   p.CellY() <= 1 ||
                   p.CellX() >= static_cast<int>(grid.GetWidth()) - 2 ||
                   p.CellY() >= static_cast<int>(grid.GetHeight()) - 2;
        };

        if (InTrail(NextStepWorldPos(chosen_path, 1), trail) && chosen_path.size() >= 3) {
            WorldPosition alt = NextStepWorldPos(chosen_path, 2);
            if (!InTrail(alt, trail) && !IsOccupied(alt, occupied)) {
                next_index = 2;
            }
        }

        WorldPosition next_wp = NextStepWorldPos(chosen_path, next_index);

        if (IsOccupied(next_wp, occupied) && chosen_path.size() >= 3) {
            WorldPosition alt = NextStepWorldPos(chosen_path, 2);
            if (!IsOccupied(alt, occupied)) {
                next_index = 2;
                next_wp = alt;
            }
        }

        if (IsNearEdge(my_pos) && IsNearEdge(next_wp) && chosen_path.size() >= 3) {
            WorldPosition alt = NextStepWorldPos(chosen_path, 2);
            if (!InTrail(alt, trail) && !IsOccupied(alt, occupied)) {
                next_index = 2;
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
        if (InTrail(pos, trail)) score += 50;
        score += CountOccupiedNeighbors(pos, occupied) * 10;

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

    tree.setMemory("chosen_action",
        BBValue(std::in_place_type<std::string>, "down"));
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