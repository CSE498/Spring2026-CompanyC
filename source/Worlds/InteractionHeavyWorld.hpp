/**
 * @file InteractionHeavyWorld.hpp
 * @author Truong Phan
 * @author Jose Hernandez
 *
 * @brief World with dungeon movement, resources, inventory, and hunter combat.
 * @note Status: PROPOSAL
 **/
#pragma once

#include "../core/WorldBase.hpp"
#include <map>
#include <vector>

namespace cse498
{
    class InteractionHeavyWorld : public WorldBase
    {
    private:
        // ---------------------------------------------------------------------
        // Player and combat state
        // ---------------------------------------------------------------------
        int mPlayerHP = 100;
        int mHunterDefaultHP = 10;
        int mThrowDamage = 10;
        int mThrowRange = 4;
        int mEnemyContactDamage = 1;
        bool mTimerStarted = false;

        // ---------------------------------------------------------------------
        // Resource and inventory state
        // ---------------------------------------------------------------------
        /**
         * @brief Resource bundle dropped by a broken boulder.
         */
        struct Boulder
        {
            int hp = 0;
            int stone = 0;
            int gold = 0;
        };

        std::map<std::pair<size_t, size_t>, Boulder> inventory;
        size_t mStoneCount = 0;
        size_t mGoldCount = 0;

        // ---------------------------------------------------------------------
        // Position state
        // ---------------------------------------------------------------------
        WorldPosition mStartPosition;
        std::vector<WorldPosition> mEnemySpawnPositions;

        // ---------------------------------------------------------------------
        // Hunter getters and helpers
        // ---------------------------------------------------------------------
        /**
         * @brief Get mutable HP storage for a hunter.
         * @param agent_id Hunter agent ID.
         * @return Reference to that hunter's HP.
         */
        int &GetHunterHP(size_t agent_id);

        /**
         * @brief Check whether an agent is a HunterAgent.
         * @param agent Agent to check.
         * @return True if the agent is a hunter.
         */
        bool IsHunterAgent(const AgentBase &agent) const;

        /**
         * @brief Check whether a hunter is still alive.
         * @param agent Hunter agent to check.
         * @return True if the hunter has HP remaining.
         */
        bool IsHunterAlive(const AgentBase &agent) const;

        /**
         * @brief Find a live hunter at a grid cell.
         * @param x Cell x coordinate.
         * @param y Cell y coordinate.
         * @param ignore Optional agent to ignore while searching.
         * @return Pointer to the hunter, or nullptr if none is found.
         */
        AgentBase *FindLiveHunterAt(size_t x, size_t y, const AgentBase *ignore = nullptr);

        /**
         * @brief Get the current player position.
         * @return Player position, or the start position if no player is found.
         */
        WorldPosition GetPlayerPosition() const;

        /**
         * @brief Move a defeated hunter outside the playable grid.
         * @param hunter Hunter agent to defeat.
         */
        void DefeatHunter(AgentBase &hunter);

        /**
         * @brief Get a position outside the grid.
         * @return Off-grid position used for defeated hunters.
         */
        WorldPosition GetOffGridPosition() const;

        std::map<size_t, int> hunter_hp;

    protected:
        // ---------------------------------------------------------------------
        // Action and resource indexes
        // ---------------------------------------------------------------------
        enum ActionType
        {
            REMAIN_STILL = 0,
            MOVE_UP,
            MOVE_DOWN,
            MOVE_LEFT,
            MOVE_RIGHT,
            COLLECT,
            PAY,
            THROW_UP,
            THROW_DOWN,
            THROW_LEFT,
            THROW_RIGHT,
            BREAK_BOULDER,
            PRINT_INVENTORY
        };

        enum ResourceIndex
        {
            RESOURCE_HP = 0,
            RESOURCE_STONE = 1,
            RESOURCE_GOLD = 2,
            RESOURCE_COUNT = 3
        };

        // ---------------------------------------------------------------------
        // Cell type IDs
        // ---------------------------------------------------------------------
        size_t mWallID;
        size_t mFloorID;
        size_t mDoorID;
        size_t mStartID;
        size_t mExitID;

        size_t mBoulderID;
        size_t mChestID;

        size_t mMaterialID;
        size_t mChestOpenID;
        size_t mDoorOpenID;

        size_t mEnemyID;

        // ---------------------------------------------------------------------
        // Agent and world setup helpers
        // ---------------------------------------------------------------------
        /**
         * @brief Find an agent blocking a grid cell.
         * @param x Cell x coordinate.
         * @param y Cell y coordinate.
         * @param ignore Optional agent to ignore while searching.
         * @return Pointer to the blocking agent, or nullptr if none is found.
         */
        AgentBase *FindBlockingAgentAt(size_t x, size_t y, const AgentBase *ignore = nullptr);

        /**
         * @brief Configure an agent with actions available in this world.
         * @param agent Agent to configure.
         */
        void ConfigAgent(AgentBase &agent) override;

        /**
         * @brief Configure the world cell types.
         */
        void ConfigureCellTypes();

        /**
         * @brief Generate the dungeon and place resources.
         */
        void GenerateWorld();

        /**
         * @brief Load dungeon cells from text rows.
         * @param dungeon_layout Text map where each character is one cell.
         */
        void LoadDungeon(const std::vector<std::string> &dungeon_layout);

    public:
        // ---------------------------------------------------------------------
        // Constructor
        // ---------------------------------------------------------------------
        /**
         * @brief Construct the interaction-heavy world.
         */
        InteractionHeavyWorld();

        // ---------------------------------------------------------------------
        // Resource getters
        // ---------------------------------------------------------------------
        /**
         * @brief Get the player's stone count.
         * @return Number of stones in inventory.
         */
        size_t GetStoneCount() const;

        /**
         * @brief Get the player's gold count.
         * @return Number of gold pieces in inventory.
         */
        size_t GetGoldCount() const;

        /**
         * @brief Get the player's HP.
         * @return Current player HP.
         */
        int GetPlayerHP() const;

        // ---------------------------------------------------------------------
        // Position getters
        // ---------------------------------------------------------------------
        /**
         * @brief Get the player start position.
         * @return Start position loaded from the dungeon map.
         */
        WorldPosition GetStartPosition() const;

        /**
         * @brief Get a random valid floor position away from the start.
         * @return Random floor position.
         */
        WorldPosition GetRandomPosition() const;

        /**
         * @brief Get hunter spawn positions loaded from the map.
         * @return Vector of enemy spawn positions.
         */
        std::vector<WorldPosition> GetEnemySpawnPositions() const;

        /**
         * @brief Check whether a live hunter is at a position.
         * @param pos Position to check.
         * @return True if a live hunter is on that cell.
         */
        bool IsEnemyAt(const WorldPosition &pos) const;

        /**
         * @brief Check whether a position is near the start.
         * @param pos Position to check.
         * @return True if the position is close to the start.
         */
        bool NearStartingPosition(const WorldPosition &pos) const;

        // ---------------------------------------------------------------------
        // Resource actions
        // ---------------------------------------------------------------------
        /**
         * @brief Place random boulders in the dungeon.
         * @param minBoulders Minimum number of boulders to create.
         * @param maxBoulders Maximum number of boulders to create.
         */
        void PlaceBoulders(int minBoulders, int maxBoulders);

        /**
         * @brief Print the player's current inventory and HP.
         */
        void PrintInventory() const;

        /**
         * @brief Break an adjacent boulder, if one exists.
         * @param x Player cell x coordinate.
         * @param y Player cell y coordinate.
         */
        void BreakBoulder(size_t x, size_t y);

        /**
         * @brief Collect nearby objects.
         * @param x Player cell x coordinate.
         * @param y Player cell y coordinate.
         */
        void Collect(size_t x, size_t y);

        /**
         * @brief Pay to open door (goblin).
         * @param x Player cell x coordinate.
         * @param y Player cell y coordinate.
         */
        void Pay(size_t x, size_t y);

        /**
         * @brief Throw a stone in a direction.
         * @param x Starting cell x coordinate.
         * @param y Starting cell y coordinate.
         * @param dx Direction x delta.
         * @param dy Direction y delta.
         */
        void ThrowStone(size_t x, size_t y, int dx, int dy);

        // ---------------------------------------------------------------------
        // Combat and action handling
        // ---------------------------------------------------------------------
        /**
         * @brief Apply hunter contact damage to the player.
         * @param player_pos Current player position.
         */
        void ApplyEnemyContactDamage(const WorldPosition &player_pos);

        /**
         * @brief Execute one action for an agent.
         * @param agent Agent performing the action.
         * @param action_id Action ID selected by the agent.
         * @return Nonzero on success, zero on failure.
         */
        int DoAction(AgentBase &agent, size_t action_id) override;

        // ---------------------------------------------------------------------
        // Resource sync
        // ---------------------------------------------------------------------
        /**
         * @brief Sync HP, stone, and gold values with WorldBase resources.
         */
        void SyncResourceVector();
    };
}
