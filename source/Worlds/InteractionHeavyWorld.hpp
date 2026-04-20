/**
 * @file InteractionHeavyWorld.hpp
 * @author Truong Phan
 * @author Jose Hernandez
 *
 * This is the initial module for the Interaction Heavy-Simulation world.
 * @brief A World that consists of various resources that agents can interact with (e.g., break, collect, etc.).
 * @note Status: PROPOSAL
 **/
#pragma once

#include "../core/WorldBase.hpp"
#include <map>

namespace cse498
{
    class InteractionHeavyWorld : public WorldBase
    {
    private:
        // Attibutes
        int mPlayerHP = 100;
        int mEnemyDefaultHP = 100;
        int mThrowDamage = 10;
        int mThrowRange = 4;
        int mEnemyContactDamage = 1;

        // enemy tile position -> hp
        std::map<std::pair<size_t, size_t>, int> enemy_hp;
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

        WorldPosition mStartPosition;

        std::vector<WorldPosition> mEnemySpawnPositions;

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
            THROW_UP,
            THROW_DOWN,
            THROW_LEFT,
            THROW_RIGHT,
            BREAK_BOULDER,
            PRINT_INVENTORY
        };
        //Respective Indexes for each resource
        enum ResourceIndex
        {
            RESOURCE_HP = 0,
            RESOURCE_STONE = 1,
            RESOURCE_GOLD = 2,
            RESOURCE_COUNT = 3
        };

        // CellType IDs
        size_t mWallID;
        size_t mFloorID;
        size_t mDoorID;
        size_t mStartID;
        size_t mExitID;

        // Resource cells
        size_t mBoulderID;
        size_t mChestID;

        // After action cells
        size_t mMaterialID;
        size_t mChestOpenID;
        size_t mDoorOpenID;

        // Special cells
        size_t mEnemyID;

        /**
         * @brief Configure an agent with the actions available in this world.
         * @param agent The agent to configure.
         */
        void ConfigAgent(AgentBase &agent) override;

        /**
         * @brief Configure the cell types in this world.
         */
        void ConfigureCellTypes();

        /**
         * @brief Generate the world layout and populate it with resources.
         */
        void GenerateWorld();

        /**
         * @brief Load a dungeon layout from a vector of strings using a text file.
         * @param dungeon_layout A vector of strings representing the dungeon layout,
         * where each character corresponds to a cell type.
         */
        void LoadDungeon(const std::vector<std::string> &dungeon_layout);

    public:
        // Constructor
        InteractionHeavyWorld();

        // Getters for inventory counts and positions
        size_t GetStoneCount() const;
        size_t GetGoldCount() const;
        WorldPosition GetStartPosition() const;
        WorldPosition GetRandomPosition() const;
        std::vector<WorldPosition> GetEnemySpawnPositions() const;
        bool IsEnemyAt(const WorldPosition &pos) const;
        int GetPlayerHP() const;

        /**
         * @brief Determine if a position is near the starting position.
         * @param pos The position to check.
         */
        bool NearStartingPosition(const WorldPosition &pos) const;

        /**
         * @brief Determine if a position is near the spawn area.
         * @param minBoulders The minimum number of boulders to create.
         * @param maxBoulders The maximum number of boulders to create.
         */
        void PlaceBoulders(int minBoulders, int maxBoulders);

        /**
         * @brief Print the inventory contents.
         */
        void PrintInventory() const;

        /**
         * @brief Break a boulder at the specified location, if it exists.
         * @param x The x-coordinate of the boulder to break.
         * @param y The y-coordinate of the boulder to break.
         */
        void BreakBoulder(size_t x, size_t y);

        /**
         * @brief Interact with the cell at the specified location, if possible.
         * @param x The x-coordinate of the cell to interact with.
         * @param y The y-coordinate of the cell to interact with.
         */
        void Interact(size_t x, size_t y);

        /**
         * @brief Override of the DoAction function to handle the specific actions available in this world.
         * @param agent The agent performing the action.
         */
        int DoAction(AgentBase &agent, size_t action_id) override;

        void ThrowStone(size_t x, size_t y, int dx, int dy);
        void ApplyEnemyContactDamage(const WorldPosition &player_pos);
        void SyncResourceVector(); ///helps sync the interaction world with world base
    };
}