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
#include "../tools/DynamicTreeBuilder.hpp"

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
    auto default_root = std::make_shared<Selector>("BaseSurvivalRoot");

    auto wander_action = std::make_shared<ActionNode>(
        "DefaultWander",
        [](Blackboard & bb) -> Status {
            auto it = bb.find("chosen_action");
            if (it == bb.end() || std::get<std::string>(it->second) == "remain_still") {
                bb["chosen_action"].emplace<std::string>("up"); // Placeholder
            }
            return Status::Success;
        }
    );

    default_root->addChild(wander_action);
    tree.setRoot(default_root);
}


void ClassicAgent::Sense( WorldGrid& grid) {
    tree.setMemory<std::any>("grid", std::cref(grid));

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

    tree.setMemory<int>("wood_count", inventory.count("wood") ? inventory.at("wood") : 0);
    tree.setMemory<int>("stone_count", inventory.count("stone") ? inventory.at("stone") : 0);
    tree.setMemory<int>("steel_count", inventory.count("steel") ? inventory.at("steel") : 0);
    tree.setMemory<int>("wheat_count", inventory.count("wheat") ? inventory.at("wheat") : 0);

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
        const auto& bb = tree.getBlackboard();
        auto it = bb.find("chosen_action");
        if (it == bb.end() || std::get<std::string>(it->second) == "remain_still") {
            tree.setMemory("chosen_action", explore_move);
        }
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

size_t ClassicAgent::SelectAction(WorldGrid & grid) {
    tree.setMemory("chosen_action", std::string("remain_still"));
    Sense(grid);
    tree.update();
    return GetAction();
}

} // namespace cse498
