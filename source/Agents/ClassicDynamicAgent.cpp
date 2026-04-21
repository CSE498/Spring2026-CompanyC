#include "ClassicDynamicAgent.hpp"

#include <algorithm>
#include <any>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../tools/CompositeNodes.hpp"
#include "../tools/LeafNodes.hpp"
#include "../tools/BehaviorTree.hpp"
#include "../tools/PathGenerator.hpp"
#include "../tools/DynamicTreeBuilder.hpp"

namespace cse498 {

namespace {

bool IsInBounds(const WorldGrid& grid, const WorldPosition& p) {
    return p.CellX() >= 0 &&
           p.CellY() >= 0 &&
           p.CellX() < static_cast<int>(grid.GetWidth()) &&
           p.CellY() < static_cast<int>(grid.GetHeight());
}

bool IsResourceType(const std::string& type) {
    return type == "tree" || type == "stone" || type == "wheat";
}

std::string StepToAction(int sx, int sy, int nx, int ny) {
    const int dx = nx - sx;
    const int dy = ny - sy;

    if (dx == 0 && dy == -1) return "up";
    if (dx == 0 && dy == 1)  return "down";
    if (dx == -1 && dy == 0) return "left";
    if (dx == 1 && dy == 0)  return "right";

    if (dx == -1 && dy == -1) return "up_left";
    if (dx == 1 && dy == -1)  return "up_right";
    if (dx == -1 && dy == 1)  return "down_left";
    if (dx == 1 && dy == 1)   return "down_right";

    return "";
}

WorldView BuildWorldViewFromKnowledge(const WorldGrid& grid, const SharedKnowledge& shared) {
    WorldView view(grid.GetWidth(), grid.GetHeight());

    for (const auto& [pos, tile] : shared.tiles) {
        if (tile.walkable_known && !tile.is_walkable) {
            view.SetBlocked(StateGridPosition(pos.CellX(), pos.CellY()));
        }
    }

    return view;
}

std::optional<std::string> PathToNearestKnownResource(
    const WorldGrid& grid,
    const SharedKnowledge& shared,
    const WorldPosition& my_pos
) {
    WorldView agent_world_view = BuildWorldViewFromKnowledge(grid, shared);

    PathGenerator generator;
    generator.SetWorldView(agent_world_view);

    StateGridPosition start(my_pos.CellX(), my_pos.CellY());

    std::optional<StateGridPosition> target_resource;
    int min_dist = 999999;

    for (const auto& [pos, tile] : shared.tiles) {
        if (!tile.has_resource) continue;

        int dist =
            std::abs(static_cast<int>(pos.CellX()) - static_cast<int>(my_pos.CellX())) +
            std::abs(static_cast<int>(pos.CellY()) - static_cast<int>(my_pos.CellY()));

        if (dist > 0 && dist < min_dist) {
            min_dist = dist;
            target_resource = StateGridPosition(pos.CellX(), pos.CellY());
        }
    }

    if (!target_resource.has_value()) {
        return std::nullopt;
    }

    WorldPath path = generator.GenerateShortestPath(start, *target_resource);
    if (path.size() < 2) {
        return std::nullopt;
    }

    std::string action = StepToAction(
        static_cast<int>(path[0].x),
        static_cast<int>(path[0].y),
        static_cast<int>(path[1].x),
        static_cast<int>(path[1].y)
    );

    if (action.empty()) {
        return std::nullopt;
    }

    return action;
}

std::optional<std::string> PathToExplore(
    const WorldGrid& grid,
    const SharedKnowledge& shared,
    const WorldPosition& my_pos
) {
    WorldView agent_world_view = BuildWorldViewFromKnowledge(grid, shared);

    PathGenerator generator;
    generator.SetWorldView(agent_world_view);

    StateGridPosition start(my_pos.CellX(), my_pos.CellY());
    WorldPath path = generator.GenerateExplorePath(start, shared, std::nullopt);

    if (path.size() < 2) {
        return std::nullopt;
    }

    std::string action = StepToAction(
        static_cast<int>(path[0].x),
        static_cast<int>(path[0].y),
        static_cast<int>(path[1].x),
        static_cast<int>(path[1].y)
    );

    if (action.empty()) {
        return std::nullopt;
    }

    return action;
}

std::optional<std::string> FindAdjacentResourceMove(
    const WorldGrid& grid,
    const WorldPosition& my_pos
) {
    const std::vector<WorldPosition> neighbors = {
        my_pos.Up(),
        my_pos.Down(),
        my_pos.Left(),
        my_pos.Right(),
        my_pos.Up().Left(),
        my_pos.Up().Right(),
        my_pos.Down().Left(),
        my_pos.Down().Right()
    };

    for (const auto& pos : neighbors) {
        if (!IsInBounds(grid, pos)) continue;

        const std::string type = grid.GetCellTypeName(grid[pos]);
        if (!IsResourceType(type)) continue;

        std::string move = StepToAction(
            static_cast<int>(my_pos.X()),
            static_cast<int>(my_pos.Y()),
            static_cast<int>(pos.X()),
            static_cast<int>(pos.Y())
        );

        if (!move.empty()) {
            return move;
        }
    }

    return std::nullopt;
}

} // namespace

void ClassicDynamicAgent::BuildTree() {
    auto root = std::make_shared<Selector>("DynamicRoot");

    // ==================================================
    // BUILD: TOWNHALL
    // ==================================================
    auto townhall_branch = std::make_shared<Sequence>("TownhallBranch");

    auto ready_townhall = std::make_shared<ConditionNode>(
        "ReadyTownhall",
        [](const Blackboard& bb) -> bool {
            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;
            int steel = bb.count("steel_count") ? std::get<int>(bb.at("steel_count")) : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            return on_grass &&
                   wood >= 500 && stone >= 500 && steel >= 500 && wheat >= 500;
        }
    );

    auto build_townhall = std::make_shared<ActionNode>(
        "BuildTownhall",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_townhall");
            return Status::Success;
        }
    );

    townhall_branch->addChild(ready_townhall);
    townhall_branch->addChild(build_townhall);

    // ==================================================
    // BUILD: SPAWNER
    // ==================================================
    auto spawner_branch = std::make_shared<Sequence>("SpawnerBranch");

    auto ready_spawner = std::make_shared<ConditionNode>(
        "ReadySpawner",
        [](const Blackboard& bb) -> bool {
            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_spawner = bb.count("has_spawner") && std::get<bool>(bb.at("has_spawner"));

            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            return on_grass && !has_spawner && stone >= 30 && wheat >= 30;
        }
    );

    auto build_spawner = std::make_shared<ActionNode>(
        "BuildSpawner",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_spawner");
            return Status::Success;
        }
    );

    spawner_branch->addChild(ready_spawner);
    spawner_branch->addChild(build_spawner);

    // ==================================================
    // BUILD: QUARRY
    // ==================================================
    auto quarry_branch = std::make_shared<Sequence>("QuarryBranch");

    auto ready_quarry = std::make_shared<ConditionNode>(
        "ReadyQuarry",
        [](const Blackboard& bb) -> bool {
            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_quarry = bb.count("has_quarry") && std::get<bool>(bb.at("has_quarry"));

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;

            return on_grass && !has_quarry && wood >= 20 && stone >= 20;
        }
    );

    auto build_quarry = std::make_shared<ActionNode>(
        "BuildQuarry",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_quarry");
            return Status::Success;
        }
    );

    quarry_branch->addChild(ready_quarry);
    quarry_branch->addChild(build_quarry);

    // ==================================================
    // BUILD: LUMBERYARD
    // ==================================================
    auto lumberyard_branch = std::make_shared<Sequence>("LumberyardBranch");

    auto ready_lumberyard = std::make_shared<ConditionNode>(
        "ReadyLumberyard",
        [](const Blackboard& bb) -> bool {
            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_lumberyard = bb.count("has_lumberyard") && std::get<bool>(bb.at("has_lumberyard"));

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int steel = bb.count("steel_count") ? std::get<int>(bb.at("steel_count")) : 0;

            return on_grass && !has_lumberyard && wood >= 20 && steel >= 20;
        }
    );

    auto build_lumberyard = std::make_shared<ActionNode>(
        "BuildLumberyard",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_lumberyard");
            return Status::Success;
        }
    );

    lumberyard_branch->addChild(ready_lumberyard);
    lumberyard_branch->addChild(build_lumberyard);

    // ==================================================
    // BUILD: FARM
    // ==================================================
    auto farm_branch = std::make_shared<Sequence>("FarmBranch");

    auto ready_farm = std::make_shared<ConditionNode>(
        "ReadyFarm",
        [](const Blackboard& bb) -> bool {
            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_farm = bb.count("has_farm") && std::get<bool>(bb.at("has_farm"));

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            return on_grass && !has_farm && wood >= 20 && wheat >= 20;
        }
    );

    auto build_farm = std::make_shared<ActionNode>(
        "BuildFarm",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_farm");
            return Status::Success;
        }
    );

    farm_branch->addChild(ready_farm);
    farm_branch->addChild(build_farm);

    // ==================================================
    // COLLECT
    // ==================================================
    auto collect_branch = std::make_shared<Sequence>("CollectBranch");

    auto on_resource = std::make_shared<ConditionNode>(
        "OnResource",
        [](const Blackboard& bb) -> bool {
            return bb.count("on_resource") && std::get<bool>(bb.at("on_resource"));
        }
    );

    auto collect_action = std::make_shared<ActionNode>(
        "CollectAction",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>("collect");
            return Status::Success;
        }
    );

    collect_branch->addChild(on_resource);
    collect_branch->addChild(collect_action);

    // ==================================================
    // MOVE TO ADJACENT RESOURCE
    // ==================================================
    auto adjacent_branch = std::make_shared<Sequence>("AdjacentResourceBranch");

    auto has_adjacent_move = std::make_shared<ConditionNode>(
        "HasAdjacentMove",
        [](const Blackboard& bb) -> bool {
            return bb.count("adjacent_resource_move") &&
                   !std::get<std::string>(bb.at("adjacent_resource_move")).empty();
        }
    );

    auto do_adjacent_move = std::make_shared<ActionNode>(
        "DoAdjacentMove",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>(
                std::get<std::string>(bb.at("adjacent_resource_move"))
            );
            return Status::Success;
        }
    );

    adjacent_branch->addChild(has_adjacent_move);
    adjacent_branch->addChild(do_adjacent_move);

    // ==================================================
    // RESOURCE PATH MOVE
    // ==================================================
    auto resource_branch = std::make_shared<Sequence>("ResourceMoveBranch");

    auto has_resource_move = std::make_shared<ConditionNode>(
        "HasResourceMove",
        [](const Blackboard& bb) -> bool {
            return bb.count("resource_move") &&
                   !std::get<std::string>(bb.at("resource_move")).empty();
        }
    );

    auto do_resource_move = std::make_shared<ActionNode>(
        "DoResourceMove",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>(
                std::get<std::string>(bb.at("resource_move"))
            );
            return Status::Success;
        }
    );

    resource_branch->addChild(has_resource_move);
    resource_branch->addChild(do_resource_move);

    // ==================================================
    // EXPLORE MOVE
    // ==================================================
    auto explore_branch = std::make_shared<Sequence>("ExploreMoveBranch");

    auto has_explore_move = std::make_shared<ConditionNode>(
        "HasExploreMove",
        [](const Blackboard& bb) -> bool {
            return bb.count("explore_move") &&
                   !std::get<std::string>(bb.at("explore_move")).empty();
        }
    );

    auto do_explore_move = std::make_shared<ActionNode>(
        "DoExploreMove",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>(
                std::get<std::string>(bb.at("explore_move"))
            );
            return Status::Success;
        }
    );

    explore_branch->addChild(has_explore_move);
    explore_branch->addChild(do_explore_move);

    // ==================================================
    // FALLBACK
    // ==================================================
    auto fallback_branch = std::make_shared<Sequence>("FallbackBranch");

    auto has_fallback_move = std::make_shared<ConditionNode>(
        "HasFallbackMove",
        [](const Blackboard& bb) -> bool {
            return bb.count("fallback_move") &&
                   !std::get<std::string>(bb.at("fallback_move")).empty();
        }
    );

    auto do_fallback_move = std::make_shared<ActionNode>(
        "DoFallbackMove",
        [](Blackboard& bb) -> Status {
            bb["chosen_action"].emplace<std::string>(
                std::get<std::string>(bb.at("fallback_move"))
            );
            return Status::Success;
        }
    );

    fallback_branch->addChild(has_fallback_move);
    fallback_branch->addChild(do_fallback_move);

    root->addChild(townhall_branch);
    root->addChild(spawner_branch);
    root->addChild(quarry_branch);
    root->addChild(lumberyard_branch);
    root->addChild(farm_branch);
    root->addChild(collect_branch);
    root->addChild(adjacent_branch);
    root->addChild(resource_branch);
    root->addChild(explore_branch);
    root->addChild(fallback_branch);

    tree.setRoot(root);
}

void ClassicDynamicAgent::Sense(WorldGrid& grid) {
    tree.setMemory<std::any>("grid", std::cref(grid));
    tree.setMemory("chosen_action", std::string("remain_still"));
    tree.setMemory("adjacent_resource_move", std::string(""));
    tree.setMemory("resource_move", std::string(""));
    tree.setMemory("explore_move", std::string(""));
    tree.setMemory("fallback_move", std::string(""));

    WorldPosition my_pos = this->GetLocation().AsWorldPosition();
    auto& shared = grid.GetSharedKnowledge();

    bool on_resource = false;
    bool on_grass = false;
    bool material_nearby = false;
    bool enemy_nearby = false;
    int current_turn = -1;

    // Reset local 3x3 temporary flags.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.currently_visible = false;
            tile.has_enemy = false;
        }
    }

    // Fill local 3x3 knowledge.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            WorldPosition p(my_pos.CellX() + dx, my_pos.CellY() + dy);
            if (!IsInBounds(grid, p)) continue;

            TileKnowledge& tile = shared.GetTile(p);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.walkable_known = true;
            tile.last_seen_turn = current_turn;

            std::string cell_type = grid.GetCellTypeName(grid[p]);
            tile.is_walkable = (cell_type != "wall" && cell_type != "blocked");

            if (IsResourceType(cell_type)) {
                tile.has_resource = true;
            }

            if (PositionsMatch(p, my_pos)) {
                if (cell_type == "grass") on_grass = true;
                if (IsResourceType(cell_type)) on_resource = true;
            }
        }
    }

    // Visible items.
    std::vector<size_t> visible_items = world.GetKnownItems(*this);
    for (size_t item_id : visible_items) {
        const auto& item = world.GetItem(item_id);
        WorldPosition item_pos = item.GetLocation().AsWorldPosition();

        if (!IsInBounds(grid, item_pos)) continue;

        TileKnowledge& tile = shared.GetTile(item_pos);
        tile.discovered = true;
        tile.currently_visible = true;
        tile.has_resource = true;
        tile.last_seen_turn = current_turn;

        if (PositionsMatch(item_pos, my_pos)) {
            on_resource = true;
        }

        const int dx = item_pos.CellX() - my_pos.CellX();
        const int dy = item_pos.CellY() - my_pos.CellY();
        if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 && !(dx == 0 && dy == 0)) {
            material_nearby = true;
        }
    }

    // Visible agents.
    std::vector<size_t> visible_agents = world.GetKnownAgents(*this);
    for (size_t agent_id : visible_agents) {
        if (agent_id == this->GetID()) continue;

        const auto& agent = world.GetAgent(agent_id);
        WorldPosition agent_pos = agent.GetLocation().AsWorldPosition();

        if (!IsInBounds(grid, agent_pos)) continue;

        TileKnowledge& tile = shared.GetTile(agent_pos);
        tile.discovered = true;
        tile.currently_visible = true;
        tile.has_enemy = true;
        tile.last_seen_turn = current_turn;

        const int dx = agent_pos.CellX() - my_pos.CellX();
        const int dy = agent_pos.CellY() - my_pos.CellY();
        if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 && !(dx == 0 && dy == 0)) {
            enemy_nearby = true;
        }
    }

    tree.setMemory("on_grass", on_grass);
    tree.setMemory("on_resource", on_resource);
    tree.setMemory("material_nearby", material_nearby);
    tree.setMemory("enemy_nearby", enemy_nearby);

    const auto& inventory = world.GetWorldGlobalCounts();
    tree.setMemory<int>("wood_count", inventory.count("wood") ? inventory.at("wood") : 0);
    tree.setMemory<int>("stone_count", inventory.count("stone") ? inventory.at("stone") : 0);
    tree.setMemory<int>("steel_count", inventory.count("steel") ? inventory.at("steel") : 0);
    tree.setMemory<int>("wheat_count", inventory.count("wheat") ? inventory.at("wheat") : 0);

    // These can be improved if you expose exact world/building state.
    tree.setMemory("has_spawner", false);
    tree.setMemory("has_quarry", false);
    tree.setMemory("has_lumberyard", false);
    tree.setMemory("has_farm", false);

    // Precompute adjacent resource move.
    if (auto adjacent = FindAdjacentResourceMove(grid, my_pos); adjacent.has_value()) {
        tree.setMemory("adjacent_resource_move", *adjacent);
    }

    // Precompute resource path move.
    if (auto resource_move = PathToNearestKnownResource(grid, shared, my_pos); resource_move.has_value()) {
        tree.setMemory("resource_move", *resource_move);
    }

    // Precompute explore move.
    if (auto explore_move = PathToExplore(grid, shared, my_pos); explore_move.has_value()) {
        tree.setMemory("explore_move", *explore_move);
    }

    // Very simple fallback.
    if (!std::get<std::string>(tree.getBlackboard().at("explore_move")).empty()) {
        tree.setMemory("fallback_move", std::get<std::string>(tree.getBlackboard().at("explore_move")));
    } else {
        tree.setMemory("fallback_move", std::string("up"));
    }
}

} // namespace cse498