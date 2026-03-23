#include "ClassicAgent.hpp"

#include <memory>
#include <string>
#include <variant>
#include <cstdlib>

#include "../tools/CompositeNodes.hpp"
#include "../tools/LeafNodes.hpp"
#include "../tools/BehaviorTree.hpp"
#include "../core/WorldBase.hpp"

namespace cse498 {

void ClassicAgent::BuildTree() {
    auto root = std::make_shared<Selector>("Root");

    // --------------------------------------------------
    // Branch 1: If enemy nearby -> attack
    // --------------------------------------------------
    auto attack_branch = std::make_shared<Sequence>("AttackBranch");

    auto enemy_nearby = std::make_shared<ConditionNode>(
        "EnemyNearby",
        [](const Blackboard & bb) -> bool {
            auto it = bb.find("enemy_nearby"); // figure out how to detect enemy/ how many squares to look
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
            auto it = bb.find("material_nearby"); // figure out how to gather/ how many squares to look
            return it != bb.end() && std::get<bool>(it->second);
        }
    );

    auto gather_action = std::make_shared<ActionNode>(
        "Gather",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("gather");
            return Status::Success;
        }
    );

    gather_branch->addChild(material_nearby);
    gather_branch->addChild(gather_action);

    // --------------------------------------------------
    // Branch 3: Otherwise -> explore
    // For now explore just moves right.
    // Replace this later with smarter exploration logic.
    // --------------------------------------------------
    auto explore_action = std::make_shared<ActionNode>(
        "Explore",
        [](Blackboard & bb) -> Status {
            int r = rand() % 4;

            switch (r) {
                case 0: bb["chosen_action"].emplace<std::string>("up"); break;
                case 1: bb["chosen_action"].emplace<std::string>("down"); break;
                case 2: bb["chosen_action"].emplace<std::string>("left"); break;
                case 3: bb["chosen_action"].emplace<std::string>("right"); break;
            }

            return Status::Success;
        }
    );

    //root->addChild(attack_branch);
    //root->addChild(gather_branch);
    root->addChild(explore_action);

    tree.setRoot(root);
}


void ClassicAgent::Sense(const WorldGrid & grid) {
    bool enemy_nearby = false;
    bool material_nearby = false;

    // --------------------------------------------------
    // Placeholder sensing logic
    // Replace this with your actual grid inspection.
    //
    // For example:
    // - if an adjacent tile contains an enemy, set enemy_nearby = true
    // - if an adjacent tile contains a material/resource, set material_nearby = true
    // --------------------------------------------------

    WorldPosition m_pos = this->GetLocation().AsWorldPosition();
    (void)grid; // suppress unused warning until real sensing is added

    std::vector<size_t> visible_items = world.GetKnownItems(*this);
    std::vector<size_t> visible_agents = world.GetKnownAgents(*this);

    // Helper to check if two positions have exact same X and Y
    auto PositionsMatch = [](const WorldPosition& a, const WorldPosition& b) {
        return a.X() == b.X() && a.Y() == b.Y();
    };

    for (size_t item_id : visible_items) {
        const auto& item = world.GetItem(item_id);

        WorldPosition item_pos = item.GetLocation().AsWorldPosition();

        if (PositionsMatch(item_pos, m_pos.Up())   ||
            PositionsMatch(item_pos, m_pos.Down()) ||
            PositionsMatch(item_pos, m_pos.Left()) ||
            PositionsMatch(item_pos, m_pos.Right())){

            material_nearby = true;
            break;
        }
    }

    for (size_t agent_id : visible_agents){

        if(agent_id == this->GetID()){
            continue;
        }

        const auto& agent = world.GetAgent(agent_id);

        WorldPosition agent_pos = agent.GetLocation().AsWorldPosition();

        if (PositionsMatch(agent_pos, m_pos.Up()) ||
            PositionsMatch(agent_pos, m_pos.Down()) ||
            PositionsMatch(agent_pos, m_pos.Left()) ||
            PositionsMatch(agent_pos, m_pos.Right())){

            enemy_nearby = true;
            break;
        }
    }

    tree.setMemory("enemy_nearby", BBValue(std::in_place_type<bool>,enemy_nearby));
    tree.setMemory("material_nearby", BBValue(std::in_place_type<bool>,material_nearby));

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
