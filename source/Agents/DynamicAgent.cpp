/**
 * @file DynamicAgent.cpp
 * @author Ahmed Ezaz Labib
 **/

#include "DynamicAgent.hpp"
#include "../core/WorldBase.hpp"
#include <algorithm>
#include <climits>
#include <iostream>

namespace cse498
{

    size_t DynamicAgent::pool_wood = 0;
    size_t DynamicAgent::pool_stone = 0;
    size_t DynamicAgent::pool_wheat = 0;
    size_t DynamicAgent::pool_steel = 0;

    bool DynamicAgent::Initialize()
    {
        pool_wood = pool_stone = pool_wheat = pool_steel = 0;
        std::cout << "[" << GetName() << "] init leader=" << is_leader << std::endl;
        return HasAction("up") && HasAction("down") && HasAction("left") &&
               HasAction("right") && HasAction("collect");
    }

    /** Updates the shared resource pool and the agent's personal count when a resource message arrives. */
    void DynamicAgent::Notify(const std::string &message,
                              const std::string &msg_type)
    {
        if (msg_type != "resource")
            return;
        if (message == "wood")
        {
            ++wood;
            ++pool_wood;
            std::cout << "[Pool] wood=" << pool_wood
                      << " s=" << pool_stone
                      << " wh=" << pool_wheat
                      << " st=" << pool_steel << std::endl;
        }
        else if (message == "stone")
        {
            ++stone;
            ++pool_stone;
            std::cout << "[Pool] w=" << pool_wood
                      << " stone=" << pool_stone
                      << " wh=" << pool_wheat
                      << " st=" << pool_steel << std::endl;
        }
        else if (message == "wheat")
        {
            ++wheat;
            ++pool_wheat;
            std::cout << "[Pool] w=" << pool_wood
                      << " s=" << pool_stone
                      << " wheat=" << pool_wheat
                      << " st=" << pool_steel << std::endl;
        }
        else if (message == "steel")
        {
            ++steel;
            ++pool_steel;
            std::cout << "[Pool] w=" << pool_wood
                      << " s=" << pool_stone
                      << " wh=" << pool_wheat
                      << " steel=" << pool_steel << std::endl;
        }
    }

    /** Returns the action name that moves the agent one step toward the nearest cell of the given type. */
    std::string DynamicAgent::MoveTo(const std::string &cell,
                                     const WorldGrid &grid) const
    {
        const WorldPosition cur = GetLocation().AsWorldPosition();
        const int cx = static_cast<int>(cur.CellX());
        const int cy = static_cast<int>(cur.CellY());

        int best = INT_MAX, bx = cx, by = cy;
        for (int dy = -50; dy <= 50; ++dy)
        {
            for (int dx = -50; dx <= 50; ++dx)
            {
                WorldPosition p(cx + dx, cy + dy);
                if (!grid.IsValid(p))
                    continue;
                if (grid.GetCellTypeName(grid[p]) != cell)
                    continue;
                int d = std::max(std::abs(dx), std::abs(dy));
                if (d < best)
                {
                    best = d;
                    bx = cx + dx;
                    by = cy + dy;
                }
            }
        }
        if (best == INT_MAX)
            return "";

        static const std::vector<std::string> kMoves = {
            "up", "down", "left", "right", "up_left", "up_right", "down_left", "down_right"};
        std::string bestMove;
        int bestDist = INT_MAX;
        for (const auto &mv : kMoves)
        {
            if (!HasAction(mv))
                continue;
            WorldPosition np = NextPos(mv);
            if (!grid.IsValid(np) || !grid.IsTraversable(grid[np]))
                continue;
            int d = std::max(std::abs((int)np.CellX() - bx), std::abs((int)np.CellY() - by));
            if (d < bestDist)
            {
                bestDist = d;
                bestMove = mv;
            }
        }
        return bestMove;
    }

    /** Returns the grid position the agent would occupy after taking the named action. */
    WorldPosition DynamicAgent::NextPos(const std::string &a) const
    {
        const WorldPosition c = GetLocation().AsWorldPosition();
        if (a == "up")
            return c.Up();
        if (a == "down")
            return c.Down();
        if (a == "left")
            return c.Left();
        if (a == "right")
            return c.Right();
        if (a == "up_left")
            return c.Up().Left();
        if (a == "up_right")
            return c.Up().Right();
        if (a == "down_left")
            return c.Down().Left();
        if (a == "down_right")
            return c.Down().Right();
        return c;
    }

    /** Returns true if the shared pool has enough resources to build the current phase's structure. */
    bool DynamicAgent::CanAfford() const
    {
        const size_t w = pool_wood > 999999 ? 0 : pool_wood;
        const size_t s = pool_stone > 999999 ? 0 : pool_stone;
        const size_t wh = pool_wheat > 999999 ? 0 : pool_wheat;
        const size_t st = pool_steel > 999999 ? 0 : pool_steel;
        switch (build_phase)
        {
        case BuildPhase::Quarry:
            return w >= 20 && s >= 20;
        case BuildPhase::Farm:
            return w >= 20 && wh >= 20;
        case BuildPhase::Lumberyard:
            return w >= 20 && st >= 20;
        case BuildPhase::Townhall:
            return w >= 500 && s >= 500 && st >= 500 && wh >= 500;
        default:
            return false;
        }
    }

    /** Returns the action name corresponding to the current build phase. */
    std::string DynamicAgent::BuildAction() const
    {
        switch (build_phase)
        {
        case BuildPhase::Quarry:
            return "build_quarry";
        case BuildPhase::Farm:
            return "build_farm";
        case BuildPhase::Lumberyard:
            return "build_lumberyard";
        case BuildPhase::Townhall:
            return "build_townhall";
        default:
            return "";
        }
    }

    /** Returns the resource cell type this agent should collect based on specialty or pool deficit. */
    std::string DynamicAgent::WhatToCollect() const
    {
        switch (specialty)
        {
        case Specialty::Farmer:
            return "wheat";
        case Specialty::Miner:
            return "stone";
        case Specialty::Woodsman:
            return "tree";
        case Specialty::None:
        default:
            break;
        }

        const size_t w = pool_wood > 999999 ? 0 : pool_wood;
        const size_t s = pool_stone > 999999 ? 0 : pool_stone;
        const size_t wh = pool_wheat > 999999 ? 0 : pool_wheat;

        const size_t mn = std::min({w, s, wh});

        if (w == mn && w < 500)
            return "tree";
        if (s == mn && s < 500)
            return "stone";
        if (wh == mn && wh < 500)
            return "wheat";

        if (w < 500)
            return "tree";
        if (s < 500)
            return "stone";
        if (wh < 500)
            return "wheat";

        return "";
    }

    size_t DynamicAgent::SelectAction(WorldGrid &grid)
    {
        /** Picks a traversable roam direction, preferring cells of preferred_cell type if specified. */
        auto ChooseRoamMove = [&](const std::string &preferred_cell = "") -> std::string
        {
            static const std::vector<std::string> kMoves = {
                "up", "up_right", "right", "down_right",
                "down", "down_left", "left", "up_left"};

            const WorldPosition cur = GetLocation().AsWorldPosition();
            const int cx = static_cast<int>(cur.CellX());
            const int cy = static_cast<int>(cur.CellY());

            const int start = (cx * 17 + cy * 31 + phase_built_count * 7) %
                              static_cast<int>(kMoves.size());

            auto try_pass = [&](bool require_preferred) -> std::string
            {
                for (size_t i = 0; i < kMoves.size(); ++i)
                {
                    const std::string &mv = kMoves[(start + static_cast<int>(i)) % kMoves.size()];
                    if (!HasAction(mv))
                        continue;
                    WorldPosition np = NextPos(mv);
                    if (!grid.IsValid(np) || !grid.IsTraversable(grid[np]))
                        continue;
                    if (require_preferred && !preferred_cell.empty() &&
                        grid.GetCellTypeName(grid[np]) != preferred_cell)
                        continue;
                    return mv;
                }
                return "";
            };

            if (!preferred_cell.empty())
            {
                std::string mv = try_pass(true);
                if (!mv.empty())
                    return mv;
            }
            return try_pass(false);
        };

        std::cout << "[" << GetName() << "]"
                  << (is_leader ? " BUILDER" : " COLLECT")
                  << " phase=" << static_cast<int>(build_phase)
                  << " built=" << phase_built_count
                  << " pool(w=" << pool_wood
                  << " s=" << pool_stone
                  << " wh=" << pool_wheat
                  << " st=" << pool_steel << ")"
                  << " ar=" << action_result
                  << std::endl;

        /** Collector: move to the target resource cell and collect it. */
        if (!is_leader)
        {
            const std::string target = WhatToCollect();

            if (!target.empty())
            {
                const WorldPosition cur = GetLocation().AsWorldPosition();

                if (grid.GetCellTypeName(grid[cur]) == target && HasAction("collect"))
                    return GetActionID("collect");

                const std::string mv = MoveTo(target, grid);
                if (!mv.empty())
                    return GetActionID(mv);

                const std::string roam = ChooseRoamMove(target);
                if (!roam.empty())
                    return GetActionID(roam);
            }

            const std::string roam = ChooseRoamMove();
            if (!roam.empty())
                return GetActionID(roam);

            return 0;
        }

        /** Leader: resolve the result of the last build attempt. */
        if (waiting_for_build)
        {
            waiting_for_build = false;

            if (action_result == 1)
            {
                ++phase_built_count;

                std::cout << "[Builder] BUILT " << BuildAction()
                          << " count=" << phase_built_count
                          << " pool(w=" << pool_wood
                          << " s=" << pool_stone
                          << " wh=" << pool_wheat
                          << " st=" << pool_steel << ")" << std::endl;

                const int target = (build_phase == BuildPhase::Townhall) ? 1 : 5;
                if (phase_built_count >= target)
                {
                    /** Advance to the next build phase and reset the structure count. */
                    if (build_phase == BuildPhase::Quarry)
                        build_phase = BuildPhase::Farm;
                    else if (build_phase == BuildPhase::Farm)
                        build_phase = BuildPhase::Lumberyard;
                    else if (build_phase == BuildPhase::Lumberyard)
                        build_phase = BuildPhase::Townhall;
                    else if (build_phase == BuildPhase::Townhall)
                        build_phase = BuildPhase::Done;

                    phase_built_count = 0;

                    std::cout << "[Builder] PHASE COMPLETE -> now targeting "
                              << BuildAction() << std::endl;
                }
            }
            else
            {
                std::cout << "[Builder] BUILD FAILED, shuffling off tile" << std::endl;
                const std::string shuffle = ChooseRoamMove("grass");
                if (!shuffle.empty())
                    return GetActionID(shuffle);
            }
        }

        if (build_phase == BuildPhase::Done)
        {
            std::cout << "[Builder] ALL PHASES DONE" << std::endl;
            return 0;
        }

        /** Leader: if resources are sufficient, move to grass and fire the build action. */
        if (CanAfford())
        {
            const WorldPosition cur = GetLocation().AsWorldPosition();

            if (grid.GetCellTypeName(grid[cur]) == "grass")
            {
                waiting_for_build = true;
                std::cout << "[Builder] FIRING " << BuildAction()
                          << " pool(w=" << pool_wood
                          << " s=" << pool_stone
                          << " wh=" << pool_wheat
                          << " st=" << pool_steel << ")" << std::endl;
                return GetActionID(BuildAction());
            }

            const std::string mv = MoveTo("grass", grid);
            if (!mv.empty())
                return GetActionID(mv);
        }

        const std::string roam = ChooseRoamMove("grass");
        if (!roam.empty())
            return GetActionID(roam);

        return 0;
    }

} // namespace cse498
