/**
 * BFS-based AI agent that solves SokobanWorld puzzles automatically.
 * Scans the grid on each new level, runs BFS over the combined state space,
 * and replays the solution one action per tick via SelectAction().
 * @author Ahmed Labib
 */

#pragma once

#include <cassert>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498
{

    class SokobanAgent : public AgentBase
    {

    public:
        SokobanAgent(size_t id, const std::string &name, const WorldBase &world)
            : AgentBase(id, name, world) {}

        ~SokobanAgent() = default;

        /** Verifies that SokobanWorld registered all four directional actions. */
        bool Initialize() override;

        /** Returns the next precomputed action ID; re-solves automatically when the queue is empty. */
        size_t SelectAction(WorldGrid &grid) override;

    private:
        /** Precomputed action IDs to replay; built by Solve(), consumed each tick. */
        std::deque<size_t> solution_queue;

        /** Hash of the grid from the last solve; used to detect level changes. */
        size_t last_grid_hash = 0;

        /**
         * Compact, hashable snapshot of a Sokoban configuration.
         * Boulders are sorted so states with the same boulders compare equal regardless of insertion order.
         */
        struct State
        {
            int ax = 0;                                /** Agent column (x). */
            int ay = 0;                                /** Agent row (y). */
            std::vector<std::pair<int, int>> boulders; /** Sorted (col, row) boulder pairs. */

            bool operator==(const State &o) const
            {
                return ax == o.ax && ay == o.ay && boulders == o.boulders;
            }
        };

        /** Hashes a State by mixing agent position and boulder coordinates with distinct primes. */
        struct StateHash
        {
            size_t operator()(const State &s) const noexcept
            {
                size_t h = (static_cast<size_t>(s.ax) * 2654435761u) ^ (static_cast<size_t>(s.ay) * 40503u);
                for (auto &[bx, by] : s.boulders)
                {
                    h ^= (static_cast<size_t>(bx) * 2246822519u) ^ (static_cast<size_t>(by) * 3266489917u);
                    h = (h << 13) | (h >> 19); /** 13+19=32 bits; safe on 32-bit and 64-bit targets. */
                }
                return h;
            }
        };

        /** Cached cell-type IDs resolved from WorldGrid names; SIZE_MAX = not yet resolved. */
        size_t id_floor = SIZE_MAX;
        size_t id_wall = SIZE_MAX;
        size_t id_button = SIZE_MAX;
        size_t id_boulder = SIZE_MAX;
        size_t id_pressed = SIZE_MAX;
        size_t id_exit = SIZE_MAX;

        /** Grid dimensions; populated once per level inside Solve(). */
        int grid_width = 0;
        int grid_height = 0;

        /** Wall lookup indexed by [col + row * grid_width]; true = blocks movement. */
        std::vector<bool> wall_map;

        /** Hashes a pair of ints such that (a,b) and (b,a) produce distinct values. */
        struct PairHash
        {
            size_t operator()(const std::pair<int, int> &p) const noexcept
            {
                return std::hash<long long>{}(
                    (static_cast<long long>(p.first) << 32) | static_cast<unsigned int>(p.second));
            }
        };

        /** Button cell positions extracted from the grid scan; used by IsGoal() during BFS. */
        std::unordered_set<std::pair<int, int>, PairHash> button_cells;

        /** Total number of buttons (pressed + unpressed) on this level. */
        size_t total_buttons = 0;

        /** Populates id_* fields by querying WorldGrid cell names. */
        void CacheCellIDs(const WorldGrid &grid);

        /** Lightweight hash of every cell value in the grid; detects level changes. */
        size_t HashGrid(const WorldGrid &grid) const;

        /** Builds an initial BFS State from the live grid and the agent's position. */
        State ExtractState(const WorldGrid &grid) const;

        /** Returns true if the cell ID represents a boulder (boulder or pressed). */
        bool IsBoulder(size_t cell_id) const;

        /** Returns true if the cell ID is walkable (floor, button, or exit). */
        bool IsOpen(size_t cell_id) const;

        /** Returns true if every button cell has a boulder on it. */
        bool IsGoal(const State &s) const;

        /**
         * Attempts to apply a (dx,dy) move to a state.
         * Rejects moves into walls, out-of-bounds, or blocked boulder pushes.
         * Returns the updated State on success, or std::nullopt on an illegal move.
         */
        std::optional<State> ApplyMove(const State &s, int dx, int dy) const;

        /**
         * BFS over the state space; fills solution_queue on success.
         * Returns true if a solution was found and queued, false otherwise.
         */
        bool Solve(const WorldGrid &grid);

        /** Converts a (dx,dy) direction to its corresponding action name string. */
        static std::string DeltaToActionName(int dx, int dy);
    };

} // namespace cse498
