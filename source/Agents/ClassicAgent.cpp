#include "ClassicAgent.hpp"

#include <memory>
#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <algorithm>

#include "../tools/CompositeNodes.hpp"
#include "../tools/LeafNodes.hpp"
#include "../tools/BehaviorTree.hpp"
#include "../core/WorldBase.hpp"
#include "../tools/PathGenerator.hpp"

namespace cse498 {

enum class Direction {
    Up,
    Down,
    Left,
    Right,
    None
};

namespace {

bool IsInBounds(const WorldGrid& grid, const WorldPosition& p) {
    return p.CellX() < grid.GetWidth() &&
           p.CellY() < grid.GetHeight();
}

Direction StepToDirection(const Point& a, const Point& b) {
    const int dx = static_cast<int>(b.x - a.x);
    const int dy = static_cast<int>(b.y - a.y);

    if (dx == 0 && dy == -1) return Direction::Up;
    if (dx == 0 && dy == 1)  return Direction::Down;
    if (dx == -1 && dy == 0) return Direction::Left;
    if (dx == 1 && dy == 0)  return Direction::Right;

    return Direction::None;
}
// Convert the Enum back to a string for the Blackboard
std::string DirectionToString(Direction dir) {
    switch(dir) {
        case Direction::Up:    return "up";
        case Direction::Down:  return "down";
        case Direction::Left:  return "left";
        case Direction::Right: return "right";
        default:               return "";
    }
}

} // namespace


void ClassicAgent::BuildTree() {
    auto root = std::make_shared<Selector>("Root");

    // --------------------------------------------------
    // Branch 0: If we can afford a Townhall and are on grass -> Build it
    // --------------------------------------------------
    auto townhall_branch = std::make_shared<Sequence>("TownhallBranch");

    auto ready_to_build_townhall = std::make_shared<ConditionNode>(
        "ReadyToBuildTownhall",
        [](const Blackboard & bb) -> bool{
            auto afford_it = bb.find("can_build_townhall");
            bool can_afford = afford_it != bb.end() && std::get<bool>(afford_it->second);

            auto grass_it = bb.find("on_grass");
            bool on_grass = grass_it != bb.end() && std::get<bool>(grass_it->second);

            return can_afford && on_grass;
        }
    );

    auto build_townhall_action = std::make_shared<ActionNode>(
        "BuildTownhall",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_townhall");
            return Status::Success;
        }
    );

    townhall_branch->addChild(ready_to_build_townhall);
    townhall_branch->addChild(build_townhall_action);

    // --------------------------------------------------
    // Branch 0.5: If we need Steel, can afford a Quarry, and are on grass
    // --------------------------------------------------
    auto quarry_branch = std::make_shared<Sequence>("QuarryBranch");

    auto ready_to_build_quarry = std::make_shared<ConditionNode>(
        "ReadyToBuildQuarry",
        [](const Blackboard & bb) -> bool {
            auto afford_it = bb.find("can_build_quarry");
            bool can_afford = afford_it != bb.end() && std::get<bool>(afford_it->second);

            auto grass_it = bb.find("on_grass");
            bool on_grass = grass_it != bb.end() && std::get<bool>(grass_it->second);

            auto built_it = bb.find("has_built_quarry");
            bool already_built = built_it != bb.end() && std::get<bool>(built_it->second);

            return can_afford && on_grass && !already_built;
        }
    );

    auto build_quarry_action = std::make_shared<ActionNode>(
        "BuildQuarry",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_quarry");

            bb["has_built_quarry"].emplace<bool>(true);
            return Status::Success;
        }
    );

    quarry_branch->addChild(ready_to_build_quarry);
    quarry_branch->addChild(build_quarry_action);

    // --------------------------------------------------
    // Branch 1: If enemy nearby -> attack
    // --------------------------------------------------
    auto attack_branch = std::make_shared<Sequence>("AttackBranch");

    auto enemy_nearby = std::make_shared<ConditionNode>(
        "EnemyNearby",
        [](const Blackboard & bb) -> bool {
            auto it = bb.find("enemy_nearby");
            return it != bb.end() && std::get<bool>(it->second);
        }
    );

    auto attack_action = std::make_shared<ActionNode>(
        "Attack",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("attack");
            return Status::Success;
        }
    );

    attack_branch->addChild(enemy_nearby);
    attack_branch->addChild(attack_action);

    // --------------------------------------------------
    // Branch 2: If material nearby -> gather
    // --------------------------------------------------
    auto gather_branch = std::make_shared<Sequence>("GatherBranch");

    auto material_nearby = std::make_shared<ConditionNode>(
        "MaterialNearby",
        [](const Blackboard & bb) -> bool {
            auto it = bb.find("material_nearby");
            return it != bb.end() && std::get<bool>(it->second);
        }
    );

    auto gather_action = std::make_shared<ActionNode>(
        "Gather",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("collect");
            return Status::Success;
        }
    );

    gather_branch->addChild(material_nearby);
    gather_branch->addChild(gather_action);

    // --------------------------------------------------
    // Branch 3: Otherwise -> explore
    // Explore now expects Sense() to have already placed
    // a valid movement string into "chosen_action".
    // --------------------------------------------------
    auto explore_action = std::make_shared<ActionNode>(
        "Explore",
        [](Blackboard & bb) -> Status {
            auto it = bb.find("chosen_action");
            if (it == bb.end()) return Status::Failure;

            if (!std::holds_alternative<std::string>(it->second)) {
                return Status::Failure;
            }

            const std::string& action = std::get<std::string>(it->second);

            if (action == "up" ||
                action == "down" ||
                action == "left" ||
                action == "right") {
                return Status::Success;
            }

            return Status::Failure;
        }
    );
    root->addChild(townhall_branch);
    root->addChild(quarry_branch);
    root->addChild(gather_branch);
    root->addChild(explore_action);

    tree.setRoot(root);
}


void ClassicAgent::Sense( WorldGrid& grid) {
    bool enemy_nearby = false;
    bool material_nearby = false;

    WorldPosition m_pos = this->GetLocation().AsWorldPosition();
    auto& shared = grid.GetSharedKnowledge();

    int current_turn = -1;

    // Reset local 3x3 visibility/resource/enemy flags before refilling.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            WorldPosition p(m_pos.CellX() + dx, m_pos.CellY() + dy);

            if (!IsInBounds(grid, p)) {
                continue;
            }

            TileKnowledge& tile = shared.GetTile(p);
            tile.currently_visible = false;
            tile.has_enemy = false;
            tile.has_resource = false;
        }
    }

    // Mark 3x3 area as seen/discovered.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            WorldPosition p(m_pos.CellX() + dx, m_pos.CellY() + dy);

            if (!IsInBounds(grid, p)) {
                continue;
            }

            TileKnowledge& tile = shared.GetTile(p);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.walkable_known = true;
            tile.last_seen_turn = current_turn;

            size_t cell_id = grid[p];
            std::string cell_type = grid.GetCellTypeName(cell_id);

            tile.is_walkable = (cell_type != "wall" && cell_type != "blocked");
        }
    }

    std::vector<size_t> visible_items = world.GetKnownItems(*this);
    std::vector<size_t> visible_agents = world.GetKnownAgents(*this);

    auto PositionsMatch = [](const WorldPosition& a, const WorldPosition& b) {
        return a.X() == b.X() && a.Y() == b.Y();
    };

    // Update shared knowledge for visible resources.
    for (size_t item_id : visible_items) {
        const auto& item = world.GetItem(item_id);
        WorldPosition item_pos = item.GetLocation().AsWorldPosition();

        int dx = item_pos.CellX() - m_pos.CellX();
        int dy = item_pos.CellY() - m_pos.CellY();

        if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 && IsInBounds(grid, item_pos)) {
            TileKnowledge& tile = shared.GetTile(item_pos);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.has_resource = true;
            tile.last_seen_turn = current_turn;
        }

        if (PositionsMatch(item_pos, m_pos.Up())   ||
            PositionsMatch(item_pos, m_pos.Down()) ||
            PositionsMatch(item_pos, m_pos.Left()) ||
            PositionsMatch(item_pos, m_pos.Right())) {
            material_nearby = true;
        }
    }

    // Update shared knowledge for visible agents.
    for (size_t agent_id : visible_agents) {
        if (agent_id == this->GetID()) {
            continue;
        }

        const auto& agent = world.GetAgent(agent_id);
        WorldPosition agent_pos = agent.GetLocation().AsWorldPosition();

        int dx = agent_pos.CellX() - m_pos.CellX();
        int dy = agent_pos.CellY() - m_pos.CellY();

        if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 && IsInBounds(grid, agent_pos)) {
            TileKnowledge& tile = shared.GetTile(agent_pos);
            tile.discovered = true;
            tile.currently_visible = true;
            tile.has_enemy = true;
            tile.last_seen_turn = current_turn;
        }

        if (PositionsMatch(agent_pos, m_pos.Up())   ||
            PositionsMatch(agent_pos, m_pos.Down()) ||
            PositionsMatch(agent_pos, m_pos.Left()) ||
            PositionsMatch(agent_pos, m_pos.Right())) {
            enemy_nearby = true;
        }
    }

    tree.setMemory("enemy_nearby", enemy_nearby);
    tree.setMemory("material_nearby", material_nearby);

    const auto& inventory = world.GetWorldGlobalCounts();

    bool can_build_townhall = false;
    if (inventory.count("wood") && inventory.at("wood") >= 500 &&
        inventory.count("stone") && inventory.at("stone") >= 500 &&
        inventory.count("steel") && inventory.at("steel") >= 500 &&
        inventory.count("wheat") && inventory.at("wheat") >= 500) {
        can_build_townhall = true;
    }

    tree.setMemory("can_build_townhall", can_build_townhall);

    // --------------------------------------------------
    // Compute explore move from shared knowledge.
    // --------------------------------------------------
    WorldView agent_world_view(grid.GetWidth(), grid.GetHeight());

    for (const auto& [pos, tile] : shared.tiles){
        if (tile.walkable_known && !tile.is_walkable){
            agent_world_view.SetBlocked(StateGridPosition(pos.CellX(), pos.CellY()));
        }
    }

    PathGenerator generator;
    generator.SetWorldView(agent_world_view);

    StateGridPosition start(m_pos.CellX(), m_pos.CellY());

    std::optional<StateGridPosition> target_resource;
    int min_dist = 999999;

    for(const auto& [pos,tile] : shared.tiles){
        if(tile.has_resource){
            int dist = std::abs(static_cast<int>(pos.CellX() - m_pos.CellX())) +
                       std::abs(static_cast<int>(pos.CellY() - m_pos.CellY()));

            if (dist > 0 && dist < min_dist){
                min_dist = dist;
                target_resource = StateGridPosition(pos.CellX(), pos.CellY());
            }
        }
    }
    WorldPath path;

    if (target_resource.has_value()){
        path = generator.GenerateShortestPath(start, target_resource.value());
    }

    if (path.size() < 2){
        path = generator.GenerateExplorePath(start, shared, std::nullopt);
    }

    std::string explore_move;

    if (path.size() >= 2) {
        const Point& a = path[0];
        const Point& b = path[1];

        Direction dir = StepToDirection(a,b);
        explore_move = DirectionToString(dir);
    }

    if (!explore_move.empty()) {
        tree.setMemory(
            "chosen_action",
            BBValue(std::in_place_type<std::string>, explore_move)
        );
    }
}


size_t ClassicAgent::GetAction() const {
    Blackboard bb = tree.getBlackboard();

    auto it = bb.find("chosen_action");
    if (it == bb.end()) {
        return 0;
    }

    const std::string & action_name = std::get<std::string>(it->second);

    auto action_it = action_map.find(action_name);
    if (action_it == action_map.end()) {
        return 0;
    }

    return action_it->second;
}

} // namespace cse498
