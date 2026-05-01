/**
 * @file DynamicAgent.cpp
 * @author Ahmed Ezaz Labib
 **/

#include "DynamicAgent.hpp"
#include "../core/WorldBase.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
    constexpr int kSearchRadius = 50;
    constexpr int kRoamXSeed = 17;
    constexpr int kRoamYSeed = 31;
    constexpr int kBuildCountSeed = 7;
    constexpr int kBuildsPerPhase = 5;
    constexpr int kTownhallBuildCount = 1;

    constexpr size_t kBasicWoodCost = 20;
    constexpr size_t kBasicStoneCost = 20;
    constexpr size_t kBasicWheatCost = 20;
    constexpr size_t kBasicSteelCost = 20;
    constexpr size_t kTownhallWoodCost = 500;
    constexpr size_t kTownhallStoneCost = 500;
    constexpr size_t kTownhallSteelCost = 500;
    constexpr size_t kTownhallWheatCost = 500;
}

namespace cse498
{

    size_t DynamicAgent::pool_wood  = 0;
    size_t DynamicAgent::pool_stone = 0;
    size_t DynamicAgent::pool_wheat = 0;
    size_t DynamicAgent::pool_steel = 0;

    bool DynamicAgent::Initialize()
    {
        pool_wood = pool_stone = pool_wheat = pool_steel = 0;
        std::cout << "[" << GetName() << "] init leader=" << is_leader
                  << " ghost=" << is_ghost << std::endl;
        const bool has_moves = HasAction("up") && HasAction("down") &&
                               HasAction("left") && HasAction("right");
        if (is_ghost)
            return has_moves;
        return has_moves && HasAction("collect");
    }

    /** Syncs the static pool variables from world_global_counts so all agents share accurate totals. */
    void DynamicAgent::SyncPool()
    {
        const auto &counts = world.GetWorldGlobalCounts();
        auto read = [&](const std::string &key) -> size_t
        {
            auto it = counts.find(key);
            return (it != counts.end()) ? it->second : 0;
        };
        pool_wood  = read("wood");
        pool_stone = read("stone");
        pool_wheat = read("wheat");
        pool_steel = read("steel");
    }

    /** No-op; pool is synced from world_global_counts at the start of each SelectAction call. */
    void DynamicAgent::Notify(const std::string &message,
                              const std::string &msg_type)
    {
        (void)message;
        (void)msg_type;
    }

    /** Returns the action name that moves the agent one step toward the nearest cell of the given type. */
    std::string DynamicAgent::MoveTo(const std::string &cell,
                                     const WorldGrid &grid) const
    {
        const WorldPosition cur = GetLocation().AsWorldPosition();
        const int cx = static_cast<int>(cur.X());
        const int cy = static_cast<int>(cur.Y());

        int best = std::numeric_limits<int>::max(), bx = cx, by = cy;
        for (int dy = -kSearchRadius; dy <= kSearchRadius; ++dy)
        {
            for (int dx = -kSearchRadius; dx <= kSearchRadius; ++dx)
            {
                WorldPosition p(cx + dx, cy + dy);
                if (!grid.IsValid(p))
                    continue;
                if (grid.GetCellTypeName(grid[p]) != cell)
                    continue;
                int d = std::max(std::abs(dx), std::abs(dy));
                if (d < best) { best = d; bx = cx + dx; by = cy + dy; }
            }
        }
        if (best == std::numeric_limits<int>::max())
            return "";

        static const std::array<std::string, 8> kMoves = {
            "up", "down", "left", "right", "up_left", "up_right", "down_left", "down_right"};
        std::string bestMove;
        double bestDist = std::numeric_limits<double>::infinity();
        for (const auto &mv : kMoves)
        {
            if (!HasAction(mv)) continue;
            WorldPosition np = NextPos(mv);
            if (!grid.IsValid(np) || !grid.IsTraversable(grid[np])) continue;
            const double d = std::max(std::abs(np.X() - static_cast<double>(bx)),
                                      std::abs(np.Y() - static_cast<double>(by)));
            if (d < bestDist) { bestDist = d; bestMove = mv; }
        }
        return bestMove;
    }

    /** Returns the grid position resulting from taking the named action. */
    WorldPosition DynamicAgent::NextPos(const std::string &a) const
    {
        const WorldPosition c = GetLocation().AsWorldPosition();
        if (a == "up")         return c.Up();
        if (a == "down")       return c.Down();
        if (a == "left")       return c.Left();
        if (a == "right")      return c.Right();
        if (a == "up_left")    return c.Up().Left();
        if (a == "up_right")   return c.Up().Right();
        if (a == "down_left")  return c.Down().Left();
        if (a == "down_right") return c.Down().Right();
        return c;
    }

    /** Returns true if the shared pool meets the cost of the current build phase. */
    bool DynamicAgent::CanAfford() const
    {
        switch (build_phase)
        {
        case BuildPhase::Quarry:     return pool_wood >= kBasicWoodCost &&
                                            pool_stone >= kBasicStoneCost;
        case BuildPhase::Farm:       return pool_wood >= kBasicWoodCost &&
                                            pool_wheat >= kBasicWheatCost;
        case BuildPhase::Lumberyard: return pool_wood >= kBasicWoodCost &&
                                            pool_steel >= kBasicSteelCost;
        case BuildPhase::Townhall:   return pool_wood >= kTownhallWoodCost &&
                                            pool_stone >= kTownhallStoneCost &&
                                            pool_steel >= kTownhallSteelCost &&
                                            pool_wheat >= kTownhallWheatCost;
        default:                     return false;
        }
    }

    /** Returns the action name for the current build phase. */
    std::string DynamicAgent::BuildAction() const
    {
        switch (build_phase)
        {
        case BuildPhase::Quarry:     return "build_quarry";
        case BuildPhase::Farm:       return "build_farm";
        case BuildPhase::Lumberyard: return "build_lumberyard";
        case BuildPhase::Townhall:   return "build_townhall";
        default:                     return "";
        }
    }

    /** Returns the resource cell type this agent should collect based on specialty or pool deficit. */
    std::string DynamicAgent::WhatToCollect() const
    {
        switch (specialty)
        {
        case Specialty::Farmer:   return "wheat";
        case Specialty::Miner:    return "stone";
        case Specialty::Woodsman: return "tree";
        case Specialty::None:
        default:                  break;
        }

        const size_t mn = std::min({pool_wood, pool_stone, pool_wheat});

        if (pool_wood  == mn && pool_wood  < kTownhallWoodCost) return "tree";
        if (pool_stone == mn && pool_stone < kTownhallStoneCost) return "stone";
        if (pool_wheat == mn && pool_wheat < kTownhallWheatCost) return "wheat";

        if (pool_wood  < kTownhallWoodCost) return "tree";
        if (pool_stone < kTownhallStoneCost) return "stone";
        if (pool_wheat < kTownhallWheatCost) return "wheat";

        return "";
    }

    size_t DynamicAgent::SelectAction(WorldGrid &grid)
    {
        /** Sync the shared pool from world_global_counts before any decisions. */
        SyncPool();

        /** Picks a traversable roam direction, preferring cells of preferred_cell type if specified. */
        auto ChooseRoamMove = [&](const std::string &preferred_cell = "") -> std::string
        {
            static const std::array<std::string, 8> kMoves = {
                "up", "up_right", "right", "down_right",
                "down", "down_left", "left", "up_left"};

            const WorldPosition cur = GetLocation().AsWorldPosition();
            const int cx = static_cast<int>(cur.X());
            const int cy = static_cast<int>(cur.Y());
            const int start = (cx * kRoamXSeed + cy * kRoamYSeed + phase_built_count * kBuildCountSeed) %
                              static_cast<int>(kMoves.size());

            auto try_pass = [&](bool require_preferred) -> std::string
            {
                for (size_t i = 0; i < kMoves.size(); ++i)
                {
                    const std::string &mv = kMoves[(start + static_cast<int>(i)) % kMoves.size()];
                    if (!HasAction(mv)) continue;
                    WorldPosition np = NextPos(mv);
                    if (!grid.IsValid(np) || !grid.IsTraversable(grid[np])) continue;
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
                if (!mv.empty()) return mv;
            }
            return try_pass(false);
        };

        /** Leader logs all resource counts once per tick. */
        ++mTickCount;
        if (is_leader)
        {
            std::cout << "[Resources tick=" << mTickCount << "] "
                      << "w=" << pool_wood << " s=" << pool_stone
                      << " wh=" << pool_wheat << " st=" << pool_steel << std::endl;
        }

        /** Ghost: only moves, never collects or builds. */
        if (is_ghost)
        {
            const std::string mv = ChooseRoamMove();
            if (!mv.empty()) return GetActionID(mv);
            return 0;
        }

        std::cout << "[" << GetName() << "]"
                  << (is_leader ? " BUILDER" : " COLLECT")
                  << " phase=" << static_cast<int>(build_phase)
                  << " built=" << phase_built_count
                  << " pool(w=" << pool_wood
                  << " s=" << pool_stone
                  << " wh=" << pool_wheat
                  << " st=" << pool_steel << ")"
                  << " ar=" << action_result << std::endl;

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
                if (!mv.empty()) return GetActionID(mv);
                const std::string roam = ChooseRoamMove(target);
                if (!roam.empty()) return GetActionID(roam);
            }
            const std::string roam = ChooseRoamMove();
            if (!roam.empty()) return GetActionID(roam);
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
                          << " pool(w=" << pool_wood << " s=" << pool_stone
                          << " wh=" << pool_wheat << " st=" << pool_steel << ")" << std::endl;

                const int target = (build_phase == BuildPhase::Townhall) ? kTownhallBuildCount : kBuildsPerPhase;
                if (phase_built_count >= target)
                {
                    /** Advance to the next build phase and reset the structure count. */
                    if      (build_phase == BuildPhase::Quarry)     build_phase = BuildPhase::Farm;
                    else if (build_phase == BuildPhase::Farm)        build_phase = BuildPhase::Lumberyard;
                    else if (build_phase == BuildPhase::Lumberyard)  build_phase = BuildPhase::Townhall;
                    else if (build_phase == BuildPhase::Townhall)    build_phase = BuildPhase::Done;
                    phase_built_count = 0;
                    std::cout << "[Builder] PHASE COMPLETE -> now targeting "
                              << BuildAction() << std::endl;
                }
            }
            else
            {
                std::cout << "[Builder] BUILD FAILED, shuffling off tile" << std::endl;
                const std::string shuffle = ChooseRoamMove("grass");
                if (!shuffle.empty()) return GetActionID(shuffle);
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
                          << " pool(w=" << pool_wood << " s=" << pool_stone
                          << " wh=" << pool_wheat << " st=" << pool_steel << ")" << std::endl;
                return GetActionID(BuildAction());
            }
            const std::string mv = MoveTo("grass", grid);
            if (!mv.empty()) return GetActionID(mv);
        }

        const std::string roam = ChooseRoamMove("grass");
        if (!roam.empty()) return GetActionID(roam);
        return 0;
    }

} // namespace cse498
