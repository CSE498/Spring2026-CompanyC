/*
 * @file InteractionHeavyWorld.cpp
 * @author Truong Phan
 * @author Jose Hernandez
 *
 * This is the implementation file for the Interaction Heavy-Simulation world.
 * @brief A World that consists of various resources that agents can interact with (e.g., break, collect, pay, throw, etc.).
 * @note Status: FINAL
 */
#include "InteractionHeavyWorld.hpp"
#include "../Agents/GoblinAgent.hpp"
#include "../Agents/HunterAgent.hpp"
#include <algorithm>
#include <fstream>
#include <random>

namespace cse498
{
    // -------------------------------------------------------------------------
    // Constructor and setup
    // -------------------------------------------------------------------------

    InteractionHeavyWorld::InteractionHeavyWorld()
    {
        SetWorldResourceNames({"HP", "Stone", "Gold"});
        SyncResourceVector();

        ConfigureCellTypes();
        GenerateWorld();
    }

    void InteractionHeavyWorld::ConfigAgent(AgentBase &agent)
    {
        // Register all actions that agents can request in this world.
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("break_boulder", BREAK_BOULDER);
        agent.AddAction("collect", COLLECT);
        agent.AddAction("pay", PAY);
        agent.AddAction("print_inventory", PRINT_INVENTORY);
        agent.AddAction("throw_up", THROW_UP);
        agent.AddAction("throw_down", THROW_DOWN);
        agent.AddAction("throw_left", THROW_LEFT);
        agent.AddAction("throw_right", THROW_RIGHT);
    }

    // -------------------------------------------------------------------------
    // Resource getters and sync
    // -------------------------------------------------------------------------

    size_t InteractionHeavyWorld::GetStoneCount() const { return mStoneCount; }

    size_t InteractionHeavyWorld::GetGoldCount() const { return mGoldCount; }

    int InteractionHeavyWorld::GetPlayerHP() const { return mPlayerHP; }

    void InteractionHeavyWorld::SyncResourceVector()
    {
        // Keep the HUD/resource display in sync with the world's internal values.
        SetWorldResourceCount(RESOURCE_HP, mPlayerHP);
        SetWorldResourceCount(RESOURCE_STONE, static_cast<int>(mStoneCount));
        SetWorldResourceCount(RESOURCE_GOLD, static_cast<int>(mGoldCount));
    }

    // -------------------------------------------------------------------------
    // World generation
    // -------------------------------------------------------------------------

    void InteractionHeavyWorld::ConfigureCellTypes()
    {
        // Cell IDs are saved so the rest of the world can compare tiles quickly.
        // Structure cells
        mWallID = main_grid.AddCellType("wall", "Wall cell", '#', false);
        mFloorID = main_grid.AddCellType("floor", "Floor cell", ' ', true);
        mExitID = main_grid.AddCellType("exit", "Exit", 'X', true);
        mStartID = main_grid.AddCellType("start", "Start", 'S', true);

        // Resource cells
        mBoulderID = main_grid.AddCellType("boulder", "Boulder resource", 'O', false);
        mChestID = main_grid.AddCellType("chest", "Chest resource", 'C', false);

        // After action cells
        mMaterialID = main_grid.AddCellType("material", "Dropped material", 'M', true);
        mChestOpenID = main_grid.AddCellType("chest_open", "Opened chest", 'c', false);

        // Hunters are agents now, so the enemy tile is not used for combat.
        // mEnemyID = main_grid.AddCellType("enemy", "Hostile", 'H', false);
    }

    WorldPosition InteractionHeavyWorld::GetStartPosition() const { return mStartPosition; }

    std::vector<WorldPosition> InteractionHeavyWorld::GetHunterSpawnPositions() const
    {
        return mHunterSpawnPositions;
    }

    std::vector<WorldPosition> InteractionHeavyWorld::GetGoblinSpawnPositions() const
    {
        return mGoblinSpawnPositions;
    }

    std::vector<WorldPosition> InteractionHeavyWorld::GetPacerSpawnPositions() const
    {
        return mPacerSpawnPositions;
    }

    bool InteractionHeavyWorld::IsEnemyAt(const WorldPosition &pos) const
    {
        for (const auto &ptr : agent_set)
        {
            if (!ptr)
                continue;
            if (!IsHunterAgent(*ptr))
                continue;
            if (!IsHunterAlive(*ptr))
                continue;

            WorldPosition hunter_pos = ptr->GetLocation().AsWorldPosition();
            if (hunter_pos.SameCell(pos))
                return true;
        }
        return false;
    }

    void InteractionHeavyWorld::GenerateWorld()
    {
        // Load dungeon layout from text file.
        std::vector<std::string> dungeon_layout;

        std::ifstream infile("source/Worlds/interaction_world_maps/dungeon_map_small.txt");
        if (!infile)
        {
            std::cerr << "Error: Could not open dungeon_map_small.txt\n";
            return;
        }

        std::string line;
        while (std::getline(infile, line))
        {
            // Remove Windows line endings if the map file was edited elsewhere.
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            dungeon_layout.push_back(line);
        }

        // Set grid size based on layout and load the dungeon.
        size_t width = dungeon_layout[0].size();
        size_t height = dungeon_layout.size();

        main_grid.Resize(width, height, mFloorID);
        LoadDungeon(dungeon_layout);

        // Place random boulders after the map has been loaded.
        PlaceBoulders(1, 4);
    }

    void InteractionHeavyWorld::LoadDungeon(const std::vector<std::string> &dungeon_layout)
    {
        // Convert map characters into cell type IDs on the grid.
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
                case 'X':
                    main_grid[pos] = mExitID;
                    break;
                case 'S':
                    // The start marker becomes floor so the player can stand there.
                    main_grid[pos] = mFloorID;
                    mStartPosition = pos;
                    break;
                case 'C':
                    main_grid[pos] = mChestID;
                    break;
                case 'H':
                    // 'H' marks a goblin spawn in the map file. The cell stays as floor,
                    // and web_main later reads these saved positions to place GoblinAgents.
                    main_grid[pos] = mFloorID;
                    mHunterSpawnPositions.push_back(pos);
                    break;
                case 'G':
                    // 'G' marks a goblin spawn in the map file. The cell stays as floor,
                    // and web_main later reads these saved positions to place GoblinAgents.
                    main_grid[pos] = mFloorID;
                    mGoblinSpawnPositions.push_back(pos);
                    break;
                case 'P':
                    main_grid[pos] = mFloorID;
                    mPacerSpawnPositions.push_back(pos);
                    break;
                }
            }
        }
    }

    bool InteractionHeavyWorld::NearStartingPosition(const WorldPosition &pos) const
    {
        // Keep random resources away from the starting area.
        size_t dx = std::abs((int)pos.CellX() - (int)mStartPosition.CellX());
        size_t dy = std::abs((int)pos.CellY() - (int)mStartPosition.CellY());

        if (dx + dy < 5)
            return true;

        return false;
    }

    WorldPosition InteractionHeavyWorld::GetRandomPosition() const
    {
        // Keep trying random cells until a safe floor cell is found.
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

    void InteractionHeavyWorld::PlaceBoulders(int minBoulders, int maxBoulders)
    {
        // Place boulders on floor cells away from the start.
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dist(minBoulders, maxBoulders);

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

    // -------------------------------------------------------------------------
    // Resource actions
    // -------------------------------------------------------------------------

    void InteractionHeavyWorld::PrintInventory() const
    {
        std::cout << "\nCurrent Stats:\n";
        std::cout << "HP: " << mPlayerHP << "\n";
        std::cout << "Stone: " << mStoneCount << "\n";
        std::cout << "Gold: " << mGoldCount << "\n\n";
    }

    void InteractionHeavyWorld::BreakBoulder(size_t x, size_t y)
    {
        // Check adjacent cells for one boulder to break.
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> stone_dist(0, 15);
        std::uniform_int_distribution<int> gold_dist(0, 4);

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

                // Broken boulders leave a material tile that stores its loot here.
                main_grid[pos] = mMaterialID;
                inventory[{pos.CellX(), pos.CellY()}] = {0, stone_found, gold_found};

                return;
            }
        }
    }

    void InteractionHeavyWorld::Collect(size_t x, size_t y)
    {
        // Check current and adjacent cells for interactable objects.
        WorldPosition center(x, y);

        std::vector<WorldPosition> neighbors = {
            center,
            center.Up(),
            center.Down(),
            center.Left(),
            center.Right()};

        for (const auto &pos : neighbors)
        {
            if (!main_grid.IsValid(pos))
                continue;

            size_t tile = main_grid[pos];

            // Collect dropped materials.
            if (tile == mMaterialID)
            {
                auto it = inventory.find({pos.CellX(), pos.CellY()});
                if (it == inventory.end())
                    continue;

                // Transfer dropped resources into the player inventory.
                mStoneCount += it->second.stone;
                mGoldCount += it->second.gold;
                inventory.erase(it);
                main_grid[pos] = mFloorID;
                SyncResourceVector();
                return;
            }

            // Open a chest.
            if (tile == mChestID)
            {
                size_t gold_found = 4;

                // Chests currently give a fixed gold reward.
                mGoldCount += gold_found;
                main_grid[pos] = mChestOpenID;
                SyncResourceVector();
                return;
            }
        }
    }

    void InteractionHeavyWorld::Pay(size_t x, size_t y)
    {
        WorldPosition center(x, y);

        std::vector<WorldPosition> neighbors = {
            center,
            center.Up(),
            center.Down(),
            center.Left(),
            center.Right()};

        for (const auto &pos : neighbors)
        {
            for (auto &ptr : agent_set)
            {
                if (!ptr)
                    continue;

                auto *goblin = dynamic_cast<GoblinAgent *>(ptr.get());
                if (!goblin)
                    continue;

                if (!ptr->GetLocation().AsWorldPosition().SameCell(pos))
                    continue;

                size_t required_gold = 1;

                if (mGoldCount >= required_gold)
                {
                    mGoldCount -= required_gold;
                    goblin->ClearBlocking();
                    goblin->SetLocation(GetOffGridPosition());

                    std::cout << "The goblin steps aside.\n";
                    SyncResourceVector();
                }
                else
                {
                    std::cout << "You do not have enough gold to pay the goblin.\n";
                }
                return;
            }
        }

        std::cout << "There is no goblin nearby to pay.\n";
    }

    void InteractionHeavyWorld::ThrowStone(size_t x, size_t y, int dx, int dy)
    {
        if (mStoneCount == 0)
        {
            std::cout << "No stones to throw.\n";
            return;
        }

        WorldPosition pos(x, y);

        for (int step = 1; step <= mThrowRange; ++step)
        {
            WorldPosition target(pos.CellX() + dx * step, pos.CellY() + dy * step);

            if (!main_grid.IsValid(target))
                return;

            size_t tile = main_grid[target];

            // Solid tiles stop the thrown stone before it reaches anything behind them.
            if (tile == mWallID || tile == mBoulderID || tile == mChestID)
            {
                return;
            }

            AgentBase *hunter = FindLiveHunterAt(target.CellX(), target.CellY());
            if (hunter != nullptr)
            {
                // Hunters are tracked as agents, so projectile hits check agent location.
                mStoneCount--;
                SyncResourceVector();

                int &hp = GetHunterHP(hunter->GetID());
                hp -= mThrowDamage;
                if (hp < 0)
                    hp = 0;

                std::cout << "Hit " << hunter->GetName()
                          << " for " << mThrowDamage
                          << " damage. HP: " << hp << "\n";

                if (hp <= 0)
                {
                    hp = 0;
                    std::cout << hunter->GetName() << " defeated.\n";
                    DefeatHunter(*hunter);
                }
                return;
            }
        }

        mStoneCount--;
        SyncResourceVector();
        std::cout << "Stone thrown and missed.\n";
    }

    // -------------------------------------------------------------------------
    // Combat and hunter helpers
    // -------------------------------------------------------------------------

    void InteractionHeavyWorld::ApplyEnemyContactDamage(const WorldPosition &player_pos)
    {
        for (auto &ptr : agent_set)
        {
            if (!ptr)
                continue;
            if (!IsHunterAgent(*ptr))
                continue;
            if (!IsHunterAlive(*ptr))
                continue;

            WorldPosition hunter_pos = ptr->GetLocation().AsWorldPosition();

            int dx = std::abs((int)hunter_pos.CellX() - (int)player_pos.CellX());
            int dy = std::abs((int)hunter_pos.CellY() - (int)player_pos.CellY());

            if (dx + dy == 1)
            {
                // Contact damage happens when a live hunter is directly adjacent.
                mPlayerHP -= mEnemyContactDamage;
                if (mPlayerHP < 0)
                    mPlayerHP = 0;
                SyncResourceVector();

                std::cout << ptr->GetName() << " hit you! Player HP: " << mPlayerHP << "\n";

                if (mPlayerHP <= 0)
                {
                    if (mTimerStarted)
                    {
                        GetTimer().Stop("Game::Session");
                        mTimerStarted = false;
                    }
                    std::cout << "You died.\n";
                    exit(0);
                }
                return;
            }
        }
    }

    int &InteractionHeavyWorld::GetHunterHP(size_t agent_id)
    {
        // Create HP on first access so newly added hunters work automatically.
        auto [it, inserted] = hunter_hp.emplace(agent_id, mHunterDefaultHP);
        return it->second;
    }

    bool InteractionHeavyWorld::IsHunterAgent(const AgentBase &agent) const
    {
        return dynamic_cast<const HunterAgent *>(&agent) != nullptr;
    }

    bool InteractionHeavyWorld::IsHunterAlive(const AgentBase &agent) const
    {
        if (!IsHunterAgent(agent))
            return false;

        auto it = hunter_hp.find(agent.GetID());
        if (it == hunter_hp.end())
            return true;
        return it->second > 0;
    }

    AgentBase *InteractionHeavyWorld::FindLiveHunterAt(size_t x, size_t y, const AgentBase *ignore)
    {
        for (auto &ptr : agent_set)
        {
            if (!ptr)
                continue;
            if (ignore && ptr.get() == ignore)
                continue;
            if (!IsHunterAgent(*ptr))
                continue;
            if (!IsHunterAlive(*ptr))
                continue;

            WorldPosition pos = ptr->GetLocation().AsWorldPosition();
            if (pos.CellX() == x && pos.CellY() == y)
            {
                return ptr.get();
            }
        }
        return nullptr;
    }

    WorldPosition InteractionHeavyWorld::GetPlayerPosition() const
    {
        // The player is the non-hunter interface agent in this world.
        for (const auto &ptr : agent_set)
        {
            if (!ptr)
                continue;
            if (!IsHunterAgent(*ptr))
            {
                return ptr->GetLocation().AsWorldPosition();
            }
        }
        return mStartPosition;
    }

    AgentBase *InteractionHeavyWorld::FindBlockingAgentAt(size_t x, size_t y, const AgentBase *ignore)
    {
        for (auto &ptr : agent_set)
        {
            if (!ptr)
                continue;
            if (ignore && ptr.get() == ignore)
                continue;

            // Dead hunters should not block.
            if (IsHunterAgent(*ptr) && !IsHunterAlive(*ptr))
                continue;

            // Live agents block movement so actors cannot stack on one cell.
            WorldPosition pos = ptr->GetLocation().AsWorldPosition();
            if (pos.CellX() == x && pos.CellY() == y)
            {
                return ptr.get();
            }
        }
        return nullptr;
    }

    WorldPosition InteractionHeavyWorld::GetOffGridPosition() const
    {
        return WorldPosition(main_grid.GetWidth() + 10, main_grid.GetHeight() + 10);
    }

    void InteractionHeavyWorld::DefeatHunter(AgentBase &hunter)
    {
        // Move defeated hunters out of the map so they stop blocking and rendering.
        hunter_hp[hunter.GetID()] = 0;
        hunter.SetLocation(GetOffGridPosition());
    }

    // -------------------------------------------------------------------------
    // Main action handling
    // -------------------------------------------------------------------------

    int InteractionHeavyWorld::DoAction(AgentBase &agent, size_t action_id)
    {
        if (!mTimerStarted && agent.GetName() == "Player")
        {
            if (action_id == MOVE_UP ||
                action_id == MOVE_DOWN ||
                action_id == MOVE_LEFT ||
                action_id == MOVE_RIGHT)
            {
                GetTimer().Start("Game::Session");
                mTimerStarted = true;
            }
        }

        WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
        WorldPosition new_position;

        // Dead hunters do not get turns.
        if (IsHunterAgent(agent))
        {
            GetHunterHP(agent.GetID());
            if (!IsHunterAlive(agent))
            {
                return false;
            }
        }

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
        case COLLECT:
            Collect(cur_position.CellX(), cur_position.CellY());
            return true;
        case PAY:
            Pay(cur_position.CellX(), cur_position.CellY());
            return true;
        case PRINT_INVENTORY:
            PrintInventory();
            return true;
        case THROW_UP:
            ThrowStone(cur_position.CellX(), cur_position.CellY(), 0, -1);
            return true;
        case THROW_DOWN:
            ThrowStone(cur_position.CellX(), cur_position.CellY(), 0, 1);
            return true;
        case THROW_LEFT:
            ThrowStone(cur_position.CellX(), cur_position.CellY(), -1, 0);
            return true;
        case THROW_RIGHT:
            ThrowStone(cur_position.CellX(), cur_position.CellY(), 1, 0);
            return true;
        }

        if (!main_grid.IsValid(new_position))
            return false;

        auto notify_failed_move = [&agent, action_id, this]()
        {
            if (!IsHunterAgent(agent))
                return;

            // Hunters use failed movement feedback to avoid repeating bad moves.
            switch (action_id)
            {
            case MOVE_UP:
                agent.Notify("up", "action_failed");
                break;
            case MOVE_DOWN:
                agent.Notify("down", "action_failed");
                break;
            case MOVE_LEFT:
                agent.Notify("left", "action_failed");
                break;
            case MOVE_RIGHT:
                agent.Notify("right", "action_failed");
                break;
            default:
                break;
            }
        };

        if (main_grid[new_position] == mWallID ||
            main_grid[new_position] == mBoulderID ||
            main_grid[new_position] == mChestID ||
            main_grid[new_position] == mChestOpenID)
        {
            notify_failed_move();
            return false;
        }

        AgentBase *blocking_agent = FindBlockingAgentAt(new_position.CellX(), new_position.CellY(), &agent);
        if (blocking_agent != nullptr)
        {
            notify_failed_move();
            if (IsHunterAgent(agent) && !IsHunterAgent(*blocking_agent))
            {
                // Hunters cannot step onto the player, so blocked contact still hurts.
                ApplyEnemyContactDamage(GetPlayerPosition());
            }
            return false;
        }

        agent.SetLocation(new_position);

        ApplyEnemyContactDamage(GetPlayerPosition());

        // Check for win condition if stepping on exit.
        if (main_grid[new_position] == mExitID)
        {
            if (agent.GetName() == "Player")
            {
                if (mTimerStarted)
                {
                    GetTimer().Stop("Game::Session");
                    mTimerStarted = false;
                }
                std::cout << "Congratulations! You've reached the exit with "
                          << mStoneCount << " stone and " << mGoldCount << " gold!\n";
                exit(0);
            }
        }

        return true;
    }

}
