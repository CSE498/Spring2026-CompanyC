#include "DynamicTreeBuilder.hpp"

#include "../tools/PathGenerator.hpp"
#include "WorldPath.hpp"
#include "BehaviorTree.hpp"
#include "CompositeNodes.hpp"
#include "LeafNodes.hpp"

#include <iostream>
#include <functional>
#include <any>

namespace cse498 {

std::shared_ptr<Node> DynamicTreeBuilder::Build(ClassicAgent* agent) {
    auto dynamic_root = std::make_shared<Selector>("DynamicRoot");

    // ==================================================
    // Branch 1. TOWNHALL BRANCH (Win Condition)
    // ==================================================
    auto townhall_branch = std::make_shared<Sequence>("TownhallBranch");

    auto ready_to_build_townhall = std::make_shared<ConditionNode>(
        "ReadyToBuildTownhall",
        [](const Blackboard & bb) -> bool {
            auto grass_it = bb.find("on_grass");
            bool on_grass = grass_it != bb.end() && std::get<bool>(grass_it->second);

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;
            int steel = bb.count("steel_count") ? std::get<int>(bb.at("steel_count")) : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            bool can_afford = (wood >= 500 && stone >= 500 && steel >= 500 && wheat >= 500);

            return can_afford && on_grass;
        }
    );

    auto build_townhall_action = std::make_shared<ActionNode>(
        "BuildTownhallAction",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_townhall");
            return Status::Success;
        }
    );

    townhall_branch->addChild(ready_to_build_townhall);
    townhall_branch->addChild(build_townhall_action);


    // ==================================================
    // Branch 2. QUARRY BRANCH
    // ==================================================
    auto quarry_branch = std::make_shared<Sequence>("QuarryBranch");

    auto ready_to_build_quarry = std::make_shared<ConditionNode>(
        "ReadyToBuildQuarry",
        [](const Blackboard & bb) -> bool {
            auto grass_it = bb.find("on_grass");
            bool on_grass = grass_it != bb.end() && std::get<bool>(grass_it->second);

            auto built_it = bb.find("has_built_quarry");
            bool already_built = built_it != bb.end() && std::get<bool>(built_it->second);

            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;

            bool can_afford = (wood >= 20 && stone >= 20);

            return can_afford && on_grass && !already_built;
        }
    );

    auto build_quarry_action = std::make_shared<ActionNode>(
        "BuildQuarryAction",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_quarry");
            bb["has_built_quarry"].emplace<bool>(true);
            return Status::Success;
        }
    );

    quarry_branch->addChild(ready_to_build_quarry);
    quarry_branch->addChild(build_quarry_action);


    dynamic_root->addChild(townhall_branch);
    dynamic_root->addChild(quarry_branch);

    // TODO: Add explore/attack/gather logic here

    return dynamic_root;
}

} // namespace cse498
