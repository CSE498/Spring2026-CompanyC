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
#include <fstream>
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

        // Starting position for the player
        WorldPosition mStartPosition;

    protected:
        // Action types for this world
        enum ActionType
        {
            REMAIN_STILL = 0,
            MOVE_UP,
            MOVE_DOWN,
            MOVE_LEFT,
            MOVE_RIGHT,
            INTERACT,
            BREAK_BOULDER,
            PRINT_INVENTORY
        };

        // Configure the agent's available actions in this world
        void ConfigAgent(AgentBase &agent) override
        {
            agent.AddAction("up", MOVE_UP);
            agent.AddAction("down", MOVE_DOWN);
            agent.AddAction("left", MOVE_LEFT);
            agent.AddAction("right", MOVE_RIGHT);
            agent.AddAction("break_boulder", BREAK_BOULDER);
            agent.AddAction("interact", INTERACT);
            agent.AddAction("print_inventory", PRINT_INVENTORY);
        }

        // CellType IDs
        size_t mWallID;
        size_t mFloorID;

        // Resource cells
        size_t mBoulderID;
        size_t mChestID;

        // After action cells
        size_t mMaterialID;
        size_t mChestOpenID;
        size_t mDoorOpenID;

        // Special cells
        size_t mDoorID;
        size_t mStartID;
        size_t mEnemyID;
        size_t mExitID;

        // Helper functions for world generation
        void ConfigureCellTypes()
        {
            // Structure cells
            mWallID = main_grid.AddCellType("wall", "Wall cell", '#', false);
            mFloorID = main_grid.AddCellType("floor", "Floor cell", ' ', true);

            // Resource cells
            mBoulderID = main_grid.AddCellType("boulder", "Boulder resource", 'O', false);
            mChestID = main_grid.AddCellType("chest", "Chest resource", 'C', false);

            // After action cells
            mMaterialID = main_grid.AddCellType("material", "Dropped material", 'M', true);
            mChestOpenID = main_grid.AddCellType("chest_open", "Opened chest", 'c', false);
            mDoorOpenID = main_grid.AddCellType("door_open", "Opened door", 'd', true);

            // Special cells
            mDoorID = main_grid.AddCellType("door", "Door", 'D', true);
            mExitID = main_grid.AddCellType("exit", "Exit", 'X', true);
            mStartID = main_grid.AddCellType("start", "Start", 'S', true);
            mEnemyID = main_grid.AddCellType("enemy", "Hostile", 'H', false);
        }

        // Generate the world from a text file
        void GenerateWorld()
        {
            std::vector<std::string> dungeon_layout;

            std::ifstream infile("assets/interaction_world_maps/dungeon_map.txt");
            if (!infile)
            {
                std::cerr << "Error: Could not open dungeon_map.txt\n";
                return;
            }

            std::string line;
            while (std::getline(infile, line))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();
                dungeon_layout.push_back(line);
            }

            size_t width = dungeon_layout[0].size();
            size_t height = dungeon_layout.size();

            main_grid.Resize(width, height, mFloorID);
            LoadDungeon(dungeon_layout);

            AddAgent<TrashInterface>("Interface")
                .SetSymbol('@')
                .SetLocation(mStartPosition);
        }

        void LoadDungeon(const std::vector<std::string>& dungeon_layout)
        {
            for (size_t y = 0; y < dungeon_layout.size(); ++y)
            {
                for (size_t x = 0; x < dungeon_layout[y].size(); ++x)
                {
                    char c = dungeon_layout[y][x];
                    WorldPosition pos(x, y);

                    switch (c)
                    {
                        case '#':
                            main_grid[pos] = mWallID;
                            break;
                        case ' ':
                            main_grid[pos] = mFloorID;
                            break;
                        case 'D':
                            main_grid[pos] = mDoorID;
                            break;
                        case 'X':
                            main_grid[pos] = mExitID;
                            break;
                        case 'S':
                            main_grid[pos] = mFloorID;
                            mStartPosition = pos;
                            break;
                        case 'H':
                        {
                            static std::mt19937 gen(std::random_device{}());
                            std::uniform_int_distribution<int> rnd(0, 1);

                            auto& agent = AddAgent<PacingAgent>("Pacer_" + std::to_string(x) + 
                                                                    "_" + std::to_string(y));
                            agent.SetLocation(pos);

                            if (rnd(gen) == 0)
                                agent.SetHorizontal();
                            else
                                agent.SetVertical();

                            main_grid[pos] = mFloorID;
                            break;
                        }
                        case 'O':
                            main_grid[pos] = mBoulderID;
                            break;
                        case 'C':
                            main_grid[pos] = mChestID;
                            break;
                    }
                }
            }
        }

    public:
        InteractionHeavyWorld()
        {
            ConfigureCellTypes();
            GenerateWorld();
        }

        size_t GetStoneCount() const { return mStoneCount; }
        size_t GetGoldCount() const { return mGoldCount; }

        void PrintInventory() const
        {
            std::cout << "\nCurrent Inventory:\n";
            std::cout << "Stone: " << mStoneCount << "\n";
            std::cout << "Gold: " << mGoldCount << "\n\n";
        }

        void BreakBoulder(size_t x, size_t y)
        {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<int> stone_dist(0, 10);
            std::uniform_int_distribution<int> gold_dist(0, 1);

            WorldPosition center(x, y);

            std::vector<WorldPosition> neighbors = {
                center.Up(),
                center.Down(),
                center.Left(),
                center.Right()
            };

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

        // Single interaction handler — checks adjacent tiles and responds to whatever is there
        void Interact(size_t x, size_t y)
        {
            WorldPosition center(x, y);

            std::vector<WorldPosition> neighbors = {
                center,
                center.Up(),
                center.Down(),
                center.Left(),
                center.Right()
            };

            for (const auto& pos : neighbors)
            {
                if (!main_grid.IsValid(pos)) continue;

                size_t tile = main_grid[pos];

                // Collect dropped materials
                if (tile == mMaterialID)
                {
                    auto it = inventory.find({pos.CellX(), pos.CellY()});
                    if (it == inventory.end()) continue;
                    mStoneCount += it->second.stone;
                    mGoldCount  += it->second.gold;
                    inventory.erase(it);
                    main_grid[pos] = mFloorID;
                    return;
                }

                // Open a chest
                if (tile == mChestID)
                {   
                    size_t gold_found = 2;

                    mGoldCount += gold_found;
                    main_grid[pos] = mChestOpenID;
                    return;
                }

                // Open a door
                if (tile == mDoorID)
                {
                    size_t required_gold = 1;

                    if (mGoldCount >= required_gold)
                    {
                        mGoldCount -= required_gold;
                        main_grid[pos] = mDoorOpenID;
                    }
                    return;
                }

                // Enemy interaction (placeholder)
                if (tile == mEnemyID)
                {
                    // Placeholder
                    return;
                }
            }
        }

        int DoAction(AgentBase &agent, size_t action_id) override
        {
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
            case BREAK_BOULDER:
                BreakBoulder(cur_position.CellX(), cur_position.CellY());
                return true;
            case INTERACT:
                Interact(cur_position.CellX(), cur_position.CellY());
                return true;
            case PRINT_INVENTORY:
                PrintInventory();
                return true;
            }

            if (!main_grid.IsValid(new_position))
                return false;

            if (main_grid[new_position] == mWallID || 
                main_grid[new_position] == mBoulderID ||
                main_grid[new_position] == mEnemyID || 
                main_grid[new_position] == mDoorID || 
                main_grid[new_position] == mChestID || 
                main_grid[new_position] == mChestOpenID)
                return false;

            agent.SetLocation(new_position);

            if (main_grid[new_position] == mExitID)
            {
                std::cout << "Congratulations! You've reached the exit with " 
                    << mStoneCount << " stone and " << mGoldCount << " gold!\n";
                exit(0);
            }

            return true;
        }
    };
}