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
    // Branch 2. SPAWNER (Expansion)
    // ==================================================
    auto spawner_branch = std::make_shared<Sequence>("SpawnerBranch");

    auto ready_to_build_spawner = std::make_shared<ConditionNode>(
        "ReadyToBuildSpawner",
        [](const Blackboard & bb) -> bool {
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_built = bb.count("has_spawner") && std::get<bool>(bb.at("has_spawner"));

            return (stone >= 30 && wheat >= 30) && !has_built && on_grass;
        }
    );

    auto build_spawner_action = std::make_shared<ActionNode>(
        "BuildSpawnerAction",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_spawner");
            return Status::Success;
        }
    );
    spawner_branch->addChild(ready_to_build_spawner);
    spawner_branch->addChild(build_spawner_action);

    // ===========================================================
    // Branch 3. QUARRY (Economy: Costs Wood/Stone -> Gives Steel)
    // ===========================================================
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

    // ==================================================
    // Branch 4. LUMBERYARD (Economy: Costs Wood/Steel -> Gives Wood)
    // ==================================================
    auto lumberyard_branch = std::make_shared<Sequence>("LumberyardBranch");

    auto ready_to_build_lumberyard = std::make_shared<ConditionNode>(
        "ReadyToBuildLumberyard",
        [](const Blackboard & bb) -> bool {
            int wood = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int steel = bb.count("steel_count") ? std::get<int>(bb.at("steel_count")) : 0;

            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_built = bb.count("has_lumberyard") && std::get<bool>(bb.at("has_lumberyard"));

            return (wood >= 20 && steel >= 20) && !has_built && on_grass;
        }
    );

    auto build_lumberyard_action = std::make_shared<ActionNode>(
        "BuildLumberyardAction",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_lumberyard");
            return Status::Success;
        }
    );
    lumberyard_branch->addChild(ready_to_build_lumberyard);
    lumberyard_branch->addChild(build_lumberyard_action);

    // ==================================================
    // Branch 5. FARM (Economy: Costs Wood/Wheat -> Gives Wheat)
    // ==================================================
    auto farm_branch = std::make_shared<Sequence>("FarmBranch");

    auto ready_to_build_farm = std::make_shared<ConditionNode>(
        "ReadyToBuildFarm",
        [](const Blackboard & bb) -> bool {
            int wood  = bb.count("wood_count")  ? std::get<int>(bb.at("wood_count"))  : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            bool has_built = bb.count("has_farm") && std::get<bool>(bb.at("has_farm"));

            return (wood >= 20 && wheat >= 20) && !has_built && on_grass;
        }
    );

    auto build_farm_action = std::make_shared<ActionNode>(
        "BuildFarmAction",
        [](Blackboard & bb) -> Status {
            bb["chosen_action"].emplace<std::string>("build_farm");
            return Status::Success;
        }
    );
    farm_branch->addChild(ready_to_build_farm);
    farm_branch->addChild(build_farm_action);

    // ==================================================
    // Branch 6. SEEK GRASS BRANCH
    // ==================================================
    auto seek_grass_branch = std::make_shared<Sequence>("SeekGrassBranch");

    auto need_to_seek_grass = std::make_shared<ConditionNode>(
        "NeedToSeekGrass",
        [](const Blackboard & bb) -> bool {
            int wood = bb.count("wood_count") ? std::get<int>(bb.at("wood_count")) : 0;
            int stone = bb.count("stone_count") ? std::get<int>(bb.at("stone_count")) : 0;
            int steel = bb.count("steel_count") ? std::get<int>(bb.at("steel_count")) : 0;
            int wheat = bb.count("wheat_count") ? std::get<int>(bb.at("wheat_count")) : 0;

            bool on_grass = bb.count("on_grass") && std::get<bool>(bb.at("on_grass"));
            if (on_grass) return false;

            bool can_townhall   = (wood >= 500 && stone >= 500 && steel >= 500 && wheat >= 500) && (!bb.count("has_townhall") || !std::get<bool>(bb.at("has_townhall")));
            bool can_spawner    = (stone >= 30 && wheat >= 30) && (!bb.count("has_spawner") || !std::get<bool>(bb.at("has_spawner")));
            bool can_quarry     = (wood >= 20 && stone >= 20) && (!bb.count("has_quarry") || !std::get<bool>(bb.at("has_quarry")));
            bool can_lumberyard = (wood >= 20 && steel >= 20) && (!bb.count("has_lumberyard") || !std::get<bool>(bb.at("has_lumberyard")));
            bool can_farm       = (wood >= 20 && wheat >= 20) && (!bb.count("has_farm") || !std::get<bool>(bb.at("has_farm")));

            return can_townhall || can_spawner || can_quarry || can_lumberyard || can_farm;
        }
    );

    auto seek_grass_action = std::make_shared<ActionNode>(
        "SeekGrassAction",
        [agent](Blackboard & bb) -> Status {
            WorldPosition my_pos = agent->GetLocation().AsWorldPosition();
            int sx = static_cast<int>(my_pos.X());
            int sy = static_cast<int>(my_pos.Y());

            const WorldGrid& grid = std::any_cast<std::reference_wrapper<const WorldGrid>>(std::get<std::any>(bb.at("grid"))).get();
            WorldPosition target_grass(-1, -1);
            bool found = false;

            for (int radius = 1; radius <= 15 && !found; ++radius) {
                for (int dy = -radius; dy <= radius && !found; ++dy) {
                    for (int dx = -radius; dx <= radius && !found; ++dx) {
                        if (abs(dx) != radius && abs(dy) != radius) continue;

                        int nx = sx + dx;
                        int ny = sy + dy;

                        if (!grid.IsValid(nx, ny)) continue;

                        WorldPosition check_pos(nx, ny);
                        if (grid.GetCellTypeName(grid[check_pos]) == "grass") {
                            target_grass = check_pos;
                            found = true;
                        }
                    }
                }
            }
            if (found) {
                WorldView view(grid.GetWidth(), grid.GetHeight());
                for (size_t y = 0; y < grid.GetHeight(); ++y) {
                    for (size_t x = 0; x < grid.GetWidth(); ++x) {
                        WorldPosition wp(x, y);
                        if (!grid.IsTraversable(grid[wp])) {
                            view.SetBlocked(StateGridPosition(x, y));
                        }
                    }
                }
                PathGenerator path_gen;
                path_gen.SetWorldView(view);
                WorldPath path = path_gen.GenerateShortestPath(
                    StateGridPosition(sx, sy),
                    StateGridPosition(target_grass.X(), target_grass.Y())
                );

                if (path.size() > 1) {
                    int next_x = static_cast<int>(path[1].x);
                    int next_y = static_cast<int>(path[1].y);

                    std::string move_action = "remain_still";
                    if (next_x > sx)      move_action = "move_right";
                    else if (next_x < sx) move_action = "move_left";
                    else if (next_y > sy) move_action = "move_down";
                    else if (next_y < sy) move_action = "move_up";

                    bb["chosen_action"].emplace<std::string>(move_action);
                    return Status::Success;
                }
            }
            return Status::Failure;
        }
    );
    seek_grass_branch->addChild(need_to_seek_grass);
    seek_grass_branch->addChild(seek_grass_action);


    dynamic_root->addChild(townhall_branch);
    dynamic_root->addChild(spawner_branch);
    dynamic_root->addChild(quarry_branch);
    dynamic_root->addChild(lumberyard_branch);
    dynamic_root->addChild(farm_branch);
    dynamic_root->addChild(seek_grass_branch);

    return dynamic_root;
}

} // namespace cse498
