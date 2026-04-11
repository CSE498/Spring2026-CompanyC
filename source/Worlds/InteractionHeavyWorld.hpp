/**
 * @file InteractionHeavyWorld.hpp
 * @author Truong Phan
 *
 * This is the initial module for the Interaction Heavy-Simulation world.
 * @brief A World that consists of various resources that agents can interact with (e.g., break, collect, craft).
 * @note Status: PROPOSAL
 **/
#pragma once

#include <random>
#include <map>
#include "../core/WorldBase.hpp"

namespace cse498
{

    class InteractionHeavyWorld : public WorldBase
    {
    private:
        // Information Revolving around Boulder
        struct Boulder
        {
            int hp = 0; // randomized
            int stone = 0;
            int gold = 0;
        };
        std::map<std::pair<size_t, size_t>, Boulder> inventory;

        size_t mStoneCount = 0;
        size_t mGoldCount = 0;

    protected:
        // Action types for this world
        enum ActionType
        {
            REMAIN_STILL = 0,
            MOVE_UP,
            MOVE_DOWN,
            MOVE_LEFT,
            MOVE_RIGHT,
            BREAK,
            COLLECT,
            CRAFT,
            PRINT_INVENTORY
        };

        // Configure the agent's available actions in this world
        void ConfigAgent(AgentBase &agent) override
        {
            agent.AddAction("up", MOVE_UP);
            agent.AddAction("down", MOVE_DOWN);
            agent.AddAction("left", MOVE_LEFT);
            agent.AddAction("right", MOVE_RIGHT);
            agent.AddAction("break", BREAK);
            agent.AddAction("collect", COLLECT);
            agent.AddAction("craft", CRAFT);
            agent.AddAction("print_inventory", PRINT_INVENTORY);

        }

        // CellType IDs
        size_t mBorderID;
        size_t mWallID;
        size_t mFloorID;
        size_t mBoulderID;
        size_t mStoneID;
        size_t mIronOreID;
        size_t mGoldOreID;
        size_t mDiamondOreID;
        size_t mExitID;
        size_t mMaterialID;

        // Helper functions for world generation
        void ConfigureCellTypes()
        {
            mBorderID = main_grid.AddCellType("border", "Border cell", 'B', false);
            mWallID = main_grid.AddCellType("wall", "Wall cell", '#', false);
            mFloorID = main_grid.AddCellType("floor", "Floor cell", ' ', true);
            mBoulderID = main_grid.AddCellType("boulder", "Boulder resource", 'O', false);
            mStoneID = main_grid.AddCellType("stone", "Stone resource", 'S', false);
            mIronOreID = main_grid.AddCellType("iron_ore", "Iron ore resource", 'I', false);
            mGoldOreID = main_grid.AddCellType("gold_ore", "Gold ore resource", 'G', false);
            mDiamondOreID = main_grid.AddCellType("diamond_ore", "Diamond ore resource", 'D', false);
            mExitID = main_grid.AddCellType("exit", "Exit cell", 'E', true);
            mMaterialID = main_grid.AddCellType("material", "Dropped material", 'M', true);
        }

        // Helper function to randomly scatter resources in the world
        void ScatterResources(size_t num_stones, size_t num_iron_ores, size_t num_gold_ores, size_t num_diamond_ores)
        {
            std::random_device rd;

            std::mt19937 gen(rd());

            std::uniform_int_distribution<int> x_dist(1, static_cast<int>(main_grid.GetWidth()) - 2);
            std::uniform_int_distribution<int> y_dist(1, static_cast<int>(main_grid.GetHeight()) - 2);

            // Lambda function to place a specific resource type in the world
            auto place_resource = [&](size_t resource_id, size_t count)
            {
                for (size_t i = 0; i < count; ++i)
                {
                    int x, y;
                    do
                    {
                        x = x_dist(gen);
                        y = y_dist(gen);
                    } while (main_grid(x, y) != mWallID);
                    main_grid(x, y) = resource_id;
                }
            };

            place_resource(mStoneID, num_stones);
            place_resource(mIronOreID, num_iron_ores);
            place_resource(mGoldOreID, num_gold_ores);
            place_resource(mDiamondOreID, num_diamond_ores);
        }

        void GenerateWorld(size_t width, size_t height)
        {
            main_grid.Resize(width, height, mFloorID);

            main_grid.Load(std::vector<std::string>{
                "#########################",
                "#      #######          #",
                "#  ###       ###   #### #",
                "# ##   O        ## ##   #",
                "# #   #####   O  ###    #",
                "# #  ##   ##      #  ## #",
                "# # ##     ##  ####  #  #",
                "#   ##  ####   O ##     #",
                "###      ##       ###   #",
                "#    ###    ###     ##  #",
                "#   ##   O    ##    ##  #",
                "#   ##  ####   ###      #",
                "#      O ####    ##  ####",
                "#  ###        O  ##     #",
                "# ##   #######   ###    #",
                "# #   ##     ##     ##  #",
                "#     ##     ##  O  ##  #",
                "###    #######      ##  #",
                "#        ##   O     ##  #",
                "#   ####    ####    ##  #",
                "#  ##   O      ##       #",
                "#   ###      ###   #### #",
                "#        ####   O       #",
                "#   O                O  #",
                "######################E##"});

            ScatterResources(30, 20, 15, 5);
        };

    public:
        InteractionHeavyWorld()
        {
            ConfigureCellTypes();
            GenerateWorld(25, 25);
        }

        size_t GetStoneCount() const { return mStoneCount; }
        size_t GetGoldCount() const { return mGoldCount; }

        void PrintInventory() const
        {
            std::cout << "\nCurrent Inventory:\n";
            std::cout << "Stone: " << mStoneCount << "\n";
            std::cout << "Gold: " << mGoldCount << "\n\n";
        }

        void Collect(size_t x, size_t y)
        {
            WorldPosition pos(x, y);

            if (!main_grid.IsValid(pos))
                return;
            if (main_grid[pos] != mMaterialID)
                return;

            auto key = std::make_pair(x, y);
            auto it = inventory.find(key);
            if (it == inventory.end())
                return;

            mStoneCount += it->second.stone;
            mGoldCount += it->second.gold;

            inventory.erase(it);
            main_grid[pos] = mFloorID;
        }

        void Break(size_t x, size_t y)
        {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> stone_dist(0, 10);
            std::uniform_int_distribution<int> gold_dist(0, 1);

            WorldPosition center(x, y);

            std::vector<WorldPosition> neighbors = {
                center.Up(),
                center.Down(),
                center.Left(),
                center.Right()};

            for (const auto &pos : neighbors)
            {
                if (!main_grid.IsValid(pos))
                    continue;

                if (main_grid[pos] == mBoulderID)
                {
                    int stone_found = stone_dist(gen);
                    int gold_found = gold_dist(gen);

                    main_grid[pos] = mMaterialID;
                    inventory[{pos.CellX(), pos.CellY()}] = {0, stone_found, gold_found};

                    return; // only break one boulder per action
                }
            }
        }

        void Craft(size_t agent_id)
        {
            // Placeholder for crafting logic
        }

        /// Allow the agents to move around the maze.
        int DoAction(AgentBase &agent, size_t action_id) override
        {
            // Determine where the agent is trying to move.
            WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
            WorldPosition new_position;
            switch (action_id)
            {
            case REMAIN_STILL:
                new_position = cur_position;
                break;
            case MOVE_UP:
                new_position = cur_position.Up();
                break;
            case MOVE_DOWN:
                new_position = cur_position.Down();
                break;
            case MOVE_LEFT:
                new_position = cur_position.Left();
                break;
            case MOVE_RIGHT:
                new_position = cur_position.Right();
                break;
            // Additional actions for interacting with resources could be added here
            case BREAK:
                Break(cur_position.CellX(), cur_position.CellY());
                return true;

            case COLLECT:
                Collect(cur_position.CellX(), cur_position.CellY());
                return true;
            case PRINT_INVENTORY: ///CAN BE DELETED JUST for DEBUG/TESTING
                PrintInventory();
                return true;
            case CRAFT:
                Craft(agent.GetID());
                break;
            }

            // Don't let the agent move off the world or into a wall or boulder.
            if (!main_grid.IsValid(new_position))
            {
                return false;
            }
            if (main_grid[new_position] == mWallID || main_grid[new_position] == mBoulderID)
            {
                return false;
            }

            // Set the agent to its new postion.
            agent.SetLocation(new_position);

            return true;
        }
    };
}