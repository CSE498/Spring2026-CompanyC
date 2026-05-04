/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Implementation of SokobanAgent.
 * @author Ahmed Ezaz Labib
 **/

#include "SokobanAgent.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <unordered_map>

namespace cse498
{

    bool SokobanAgent::Initialize()
    {
        return HasAction("up") && HasAction("down") && HasAction("left") && HasAction("right");
    }

    size_t SokobanAgent::SelectAction(WorldGrid &grid)
    {
        /** Re-solve if the grid hash changed, indicating a new level was loaded. */
        const size_t current_hash = HashGrid(grid);
        if (current_hash != last_grid_hash)
        {
            solution_queue.clear();
            last_grid_hash = current_hash;
            CacheCellIDs(grid);
        }

        /** Play back queued actions; clear and re-solve if the world rejected a move. */
        if (!solution_queue.empty())
        {
            if (action_result == 0)
            {
                solution_queue.clear();
            }
            else
            {
                const size_t next = solution_queue.front();
                solution_queue.pop_front();
                return next;
            }
        }

        /** Queue is empty or was cleared; compute a fresh solution. */
        if (!Solve(grid))
        {
            std::cerr << "[SokobanAgent] No solution found for current level.\n";
            return 0;
        }

        if (!solution_queue.empty())
        {
            const size_t next = solution_queue.front();
            solution_queue.pop_front();
            return next;
        }

        return 0; /** Degenerate case: already on goal with zero moves needed. */
    }

    void SokobanAgent::CacheCellIDs(const WorldGrid &grid)
    {
        id_floor = grid.GetCellTypeID("floor");
        id_wall = grid.GetCellTypeID("wall");
        id_button = grid.GetCellTypeID("button");
        id_boulder = grid.GetCellTypeID("boulder");
        id_pressed = grid.GetCellTypeID("pressed");
        id_exit = grid.GetCellTypeID("exit");
    }

    size_t SokobanAgent::HashGrid(const WorldGrid &grid) const
    {
        const int w = static_cast<int>(grid.GetWidth());
        const int h = static_cast<int>(grid.GetHeight());
        size_t h_val = static_cast<size_t>(w) * 1000003u + static_cast<size_t>(h);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const size_t cell = grid[WorldPosition{x, y}];
                h_val ^= cell * 2654435761u;
                h_val = (h_val << 5) | (h_val >> 27); /** 5+27=32 bits; safe on 32/64-bit targets. */
            }
        }
        return h_val;
    }

    SokobanAgent::State
    SokobanAgent::ExtractState(const WorldGrid &grid) const
    {
        State s;

        const WorldPosition agent_wp = GetLocation().AsWorldPosition();
        s.ax = static_cast<int>(agent_wp.CellX());
        s.ay = static_cast<int>(agent_wp.CellY());

        const int w = static_cast<int>(grid.GetWidth());
        const int h = static_cast<int>(grid.GetHeight());

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                if (IsBoulder(grid[WorldPosition{x, y}]))
                {
                    s.boulders.emplace_back(x, y);
                }
            }
        }

        /** Sort boulders for consistent hashing and equality checks. */
        std::sort(s.boulders.begin(), s.boulders.end());
        return s;
    }

    bool SokobanAgent::IsBoulder(size_t cell_id) const
    {
        return cell_id == id_boulder || cell_id == id_pressed;
    }

    bool SokobanAgent::IsOpen(size_t cell_id) const
    {
        return cell_id == id_floor || cell_id == id_button || cell_id == id_exit;
    }

    bool SokobanAgent::IsGoal(const State &s) const
    {
        if (total_buttons == 0)
            return true;

        size_t covered = 0;
        for (auto &[bx, by] : s.boulders)
        {
            if (button_cells.count({bx, by}))
            {
                ++covered;
            }
        }
        return covered == total_buttons;
    }

    std::optional<SokobanAgent::State>
    SokobanAgent::ApplyMove(const State &s, int dx, int dy) const
    {
        const int new_ax = s.ax + dx;
        const int new_ay = s.ay + dy;

        /** Reject if the agent destination is out of bounds or a wall. */
        if (new_ax < 0 || new_ay < 0 || new_ax >= grid_width || new_ay >= grid_height)
            return std::nullopt;

        if (wall_map[static_cast<size_t>(new_ax + new_ay * grid_width)])
            return std::nullopt;

        State next = s;

        /** If a boulder occupies the destination, attempt to push it one step further. */
        auto boulder_it = std::find(next.boulders.begin(), next.boulders.end(),
                                    std::make_pair(new_ax, new_ay));
        if (boulder_it != next.boulders.end())
        {
            const int tgt_x = new_ax + dx;
            const int tgt_y = new_ay + dy;

            /** Reject if the boulder's push target is out of bounds, a wall, or occupied. */
            if (tgt_x < 0 || tgt_y < 0 || tgt_x >= grid_width || tgt_y >= grid_height)
                return std::nullopt;

            if (wall_map[static_cast<size_t>(tgt_x + tgt_y * grid_width)])
                return std::nullopt;

            if (std::find(next.boulders.begin(), next.boulders.end(),
                          std::make_pair(tgt_x, tgt_y)) != next.boulders.end())
                return std::nullopt;

            *boulder_it = {tgt_x, tgt_y};
            std::sort(next.boulders.begin(), next.boulders.end()); /** Keep boulders sorted. */
        }

        next.ax = new_ax;
        next.ay = new_ay;
        return next;
    }

    bool SokobanAgent::Solve(const WorldGrid &grid)
    {
        /** Build wall map and button set from the live grid. */
        grid_width = static_cast<int>(grid.GetWidth());
        grid_height = static_cast<int>(grid.GetHeight());

        wall_map.assign(static_cast<size_t>(grid_width * grid_height), false);
        button_cells.clear();
        total_buttons = 0;

        for (int y = 0; y < grid_height; ++y)
        {
            for (int x = 0; x < grid_width; ++x)
            {
                const size_t cell = grid[WorldPosition{x, y}];
                if (cell == id_wall)
                    wall_map[static_cast<size_t>(x + y * grid_width)] = true;

                /** Count both bare and already-pressed buttons toward the total. */
                if (cell == id_button || cell == id_pressed)
                {
                    button_cells.insert({x, y});
                    ++total_buttons;
                }
            }
        }

        const State initial = ExtractState(grid);

        if (IsGoal(initial))
            return true; /** Already solved; nothing to queue. */

        /** parent_map stores each state's predecessor and the action that produced it. */
        using ParentEntry = std::pair<State, std::string>;
        std::unordered_map<State, ParentEntry, StateHash> parent_map;

        std::queue<State> frontier;
        frontier.push(initial);
        parent_map[initial] = {initial, ""}; /** Sentinel: initial state has no parent. */

        constexpr int DX[4] = {0, 0, -1, 1};
        constexpr int DY[4] = {-1, 1, 0, 0};
        const std::string NAMES[4] = {"up", "down", "left", "right"};

        State goal_state;
        bool found = false;

        while (!frontier.empty() && !found)
        {
            const State cur = frontier.front();
            frontier.pop();

            for (int i = 0; i < 4; ++i)
            {
                auto maybe = ApplyMove(cur, DX[i], DY[i]);
                if (!maybe)
                    continue;

                State &next = *maybe;
                if (parent_map.count(next))
                    continue;

                parent_map[next] = {cur, NAMES[i]};

                if (IsGoal(next))
                {
                    goal_state = next;
                    found = true;
                    break;
                }

                frontier.push(next);
            }
        }

        if (!found)
            return false;

        /** Reconstruct the action sequence by walking back through parent_map. */
        std::vector<std::string> action_names;
        State cur = goal_state;

        while (true)
        {
            auto &[parent, action_name] = parent_map.at(cur);
            if (action_name.empty())
                break;
            action_names.push_back(action_name);
            cur = parent;
        }

        std::reverse(action_names.begin(), action_names.end());

        solution_queue.clear();
        for (const auto &name : action_names)
        {
            assert(HasAction(name));
            solution_queue.push_back(GetActionID(name));
        }

        return true;
    }

    std::string SokobanAgent::DeltaToActionName(int dx, int dy)
    {
        if (dx == 0 && dy == -1)
            return "up";
        if (dx == 0 && dy == 1)
            return "down";
        if (dx == -1 && dy == 0)
            return "left";
        if (dx == 1 && dy == 0)
            return "right";
        return "";
    }

} // namespace cse498
