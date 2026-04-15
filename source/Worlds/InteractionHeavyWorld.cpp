/*
 * @file InteractionHeavyWorld.cpp
 * @author Truong Phan
 *
 * This is the implementation file for the Interaction Heavy-Simulation world.
 * @brief A World that consists of various resources that agents can interact with (e.g., break, collect, etc.).
 * @note Status: PROPOSAL
 */
#include "InteractionHeavyWorld.hpp"
#include <random>
#include <fstream>

namespace cse498
{
    size_t InteractionHeavyWorld::GetStoneCount() const { return mStoneCount; }

    size_t InteractionHeavyWorld::GetGoldCount() const { return mGoldCount; }

    WorldPosition InteractionHeavyWorld::GetStartPosition() const { return mStartPosition; }
    
    void InteractionHeavyWorld::ConfigAgent(AgentBase &agent)
    {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("break_boulder", BREAK_BOULDER);
        agent.AddAction("interact", INTERACT);
        agent.AddAction("print_inventory", PRINT_INVENTORY);
    }

    void InteractionHeavyWorld::ConfigureCellTypes()
    {
        // Structure cells
        mWallID = main_grid.AddCellType("wall", "Wall cell", '#', false);
        mFloorID = main_grid.AddCellType("floor", "Floor cell", ' ', true);
        mDoorID = main_grid.AddCellType("door", "Door", 'D', true);
        mExitID = main_grid.AddCellType("exit", "Exit", 'X', true);
        mStartID = main_grid.AddCellType("start", "Start", 'S', true);

        // Resource cells
        mBoulderID = main_grid.AddCellType("boulder", "Boulder resource", 'O', false);
        mChestID = main_grid.AddCellType("chest", "Chest resource", 'C', false);

        // After action cells
        mMaterialID = main_grid.AddCellType("material", "Dropped material", 'M', true);
        mChestOpenID = main_grid.AddCellType("chest_open", "Opened chest", 'c', false);
        mDoorOpenID = main_grid.AddCellType("door_open", "Opened door", 'd', true);

        // Special cells;
        mEnemyID = main_grid.AddCellType("enemy", "Hostile", 'H', false);
    }

    void InteractionHeavyWorld::GenerateWorld()
    {
        // Load dungeon layout from text file
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

        // Set grid size based on layout and load the dungeon
        size_t width = dungeon_layout[0].size();
        size_t height = dungeon_layout.size();

        main_grid.Resize(width, height, mFloorID);

        LoadDungeon(dungeon_layout);

        // Place boulders in the world
        PlaceBoulders();
    }

    void InteractionHeavyWorld::LoadDungeon(const std::vector<std::string>& dungeon_layout)
    {
        // Iterate through the layout and set cell types based on characters
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
                    case 'C':
                        main_grid[pos] = mChestID;
                        break;
                }
            }
        }
    }

    bool InteractionHeavyWorld::NearStartingPosition(const WorldPosition &pos) const
    {
        // Simple Manhattan distance check to keep resources away from the starting area
        size_t dx = std::abs((int)pos.CellX() - (int)mStartPosition.CellX());
        size_t dy = std::abs((int)pos.CellY() - (int)mStartPosition.CellY());

        if (dx + dy < 5)
            return true;

        return false;
    }

    WorldPosition InteractionHeavyWorld::GetRandomPosition() const
    {
        // Generate random positions until we find a valid floor cell that isn't near the starting position
        static std::mt19937 gen(std::random_device{}());

        std::uniform_int_distribution<size_t> x_dist(0, main_grid.GetWidth() - 1);
        std::uniform_int_distribution<size_t> y_dist(0, main_grid.GetHeight() - 1);

        while (true)
        {
            size_t x = x_dist(gen);
            size_t y = y_dist(gen);

            WorldPosition pos(x, y);

            if (main_grid[pos] == mFloorID && !pos.SameCell(mStartPosition) && !NearStartingPosition(pos))
                return pos;
        }
    }

    void InteractionHeavyWorld::PlaceBoulders()
    {
        // Randomly place boulders in the world, avoiding the starting area and ensuring they are on floor cells.
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dist(50, 70);

        int count = dist(gen);
        int placed = 0;

        while (placed < count)
        {
            auto pos = GetRandomPosition();

            if (main_grid[pos] == mFloorID && !pos.SameCell(mStartPosition) && !NearStartingPosition(pos))
            {
                main_grid[pos] = mBoulderID;
                ++placed;
            }
        }
    }

    void InteractionHeavyWorld::PrintInventory() const
    {
        std::cout << "\nCurrent Inventory:\n";
        std::cout << "Stone: " << mStoneCount << "\n";
        std::cout << "Gold: " << mGoldCount << "\n\n";
    }

    void InteractionHeavyWorld::BreakBoulder(size_t x, size_t y)
    {
        // Check the four adjacent cells for a boulder to break
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

    void InteractionHeavyWorld::Interact(size_t x, size_t y)
    {
        // Check the four adjacent cells for interactable objects (e.g., materials, chests, doors, enemies)
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

    int InteractionHeavyWorld::DoAction(AgentBase &agent, size_t action_id)
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

        // Check for win condition if stepping on exit
        if (main_grid[new_position] == mExitID)
        {
            if (agent.GetName() == "Player")
            {
                std::cout << "Congratulations! You've reached the exit with "
                    << mStoneCount << " stone and " << mGoldCount << " gold!\n";
                exit(0);
            }
        }

        return true;
    }

}