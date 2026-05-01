/**
 * @file HunterAgent.cpp
 * @author Ahmed Ezaz Labib, Shashank Papani
 *
 * Hunter enemy agent for InteractionHeavyWorld.
 **/

#include "HunterAgent.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>

namespace cse498
{

    bool HunterAgent::Initialize()
    {
        return SupportsAction("up") &&
               SupportsAction("down") &&
               SupportsAction("left") &&
               SupportsAction("right");
    }

    /**
     * Handles three message types:
     *   "target_position" — parses "x,y" and switches to Chase.
     *   "action_failed"   — increments the failure counter for the given direction.
     *   "reset"           — returns the hunter to Roam and clears target state.
     */
    void HunterAgent::Notify(const std::string &message,
                             const std::string &msg_type)
    {
        if (msg_type == "target_position")
        {
            /** Parse "x,y" and inject the target position directly. */
            const size_t comma = message.find(',');
            if (comma != std::string::npos)
            {
                mTargetX = std::atoi(message.substr(0, comma).c_str());
                mTargetY = std::atoi(message.substr(comma + 1).c_str());
                mState = State::Chase;
                mChaseMemory = mChaseMemoryTicks;
            }
        }
        else if (msg_type == "action_failed")
        {
            if (message == "up")         ++fail_up;
            else if (message == "down")  ++fail_down;
            else if (message == "left")  ++fail_left;
            else if (message == "right") ++fail_right;
        }
        else if (msg_type == "reset")
        {
            mState = State::Roam;
            mTargetX = -1;
            mTargetY = -1;
            mChaseMemory = 0;
        }
    }

    size_t HunterAgent::SelectAction(WorldGrid &grid)
    {
        /** Scan for the nearest target within chase radius and update state accordingly. */
        int scan_x = -1, scan_y = -1;
        const bool target_visible = ScanForTarget(grid, scan_x, scan_y, mChaseRadius);

        if (target_visible)
        {
            mTargetX = scan_x;
            mTargetY = scan_y;
            mChaseMemory = mChaseMemoryTicks;

            const WorldPosition cur = GetLocation().AsWorldPosition();
            const int cx = static_cast<int>(cur.CellX());
            const int cy = static_cast<int>(cur.CellY());
            const int dist = std::max(std::abs(cx - mTargetX), std::abs(cy - mTargetY));

            if (mState == State::Roam && dist <= mDetectRadius)
                mState = State::Alert;
            else if (mState == State::Alert || mState == State::Chase)
                mState = State::Chase;

            /** If Roam and target is beyond detect radius, remain in Roam until within mDetectRadius. */
        }
        else
        {
            /** Target not visible; count down chase memory before returning to Roam. */
            if (mState == State::Chase || mState == State::Alert)
            {
                if (mChaseMemory > 0)
                {
                    --mChaseMemory;
                    mState = State::Chase;
                }
                else
                {
                    mState = State::Roam;
                    mTargetX = -1;
                    mTargetY = -1;
                }
            }
        }

        std::string chosen;

        switch (mState)
        {
        case State::Alert:
            /** One-tick hesitation: hold position, then transition to Chase next tick. */
            mState = State::Chase;
            if (!last_action.empty() && IsMoveValid(last_action, grid))
                chosen = last_action;
            else
            {
                static const std::array<std::string, 4> kDirs = {"up", "down", "left", "right"};
                for (const std::string &a : kDirs)
                {
                    if (IsMoveValid(a, grid)) { chosen = a; break; }
                }
            }
            break;

        case State::Chase:
            chosen = (mTargetX >= 0 && mTargetY >= 0)
                         ? ChaseMove(grid, mTargetX, mTargetY)
                         : RoamMove(grid);
            break;

        case State::Roam:
        default:
            chosen = RoamMove(grid);
            break;
        }

        last_action = chosen;
        return LookupActionID(chosen);
    }

    /**
     * Scans a (2*radius+1)^2 area centred on the hunter for cells named "player" or "agent".
     * Returns the nearest such cell by Chebyshev distance via out_x and out_y.
     * TendencyAgents must be represented as "agent" grid cells, or use Notify to push positions directly.
     */
    bool HunterAgent::ScanForTarget(const WorldGrid &grid,
                                    int &out_x, int &out_y,
                                    int radius) const
    {
        const WorldPosition cur = GetLocation().AsWorldPosition();
        const int cx = static_cast<int>(cur.X());
        const int cy = static_cast<int>(cur.Y());

        int best_dist = std::numeric_limits<int>::max(), best_x = -1, best_y = -1;

        for (int dy = -radius; dy <= radius; ++dy)
        {
            for (int dx = -radius; dx <= radius; ++dx)
            {
                if (dx == 0 && dy == 0) continue;

                const int nx = cx + dx;
                const int ny = cy + dy;
                if (nx < 0 || ny < 0) continue;

                const WorldPosition cand(nx, ny);

                if (!grid.IsValid(cand)) continue;

                const std::string tile = grid.GetCellTypeName(grid[cand]);
                if (tile == "player" || tile == "agent")
                {
                    const int dist = std::max(std::abs(dx), std::abs(dy));
                    if (dist < best_dist) { best_dist = dist; best_x = nx; best_y = ny; }
                }
            }
        }

        if (best_dist == std::numeric_limits<int>::max()) return false;
        out_x = best_x;
        out_y = best_y;
        return true;
    }

    /**
     * Greedy Chebyshev step toward (tx, ty).
     * Breaks ties by lowest failure count; falls back to any valid move if cornered.
     */
    std::string HunterAgent::ChaseMove(const WorldGrid &grid,
                                       int tx, int ty) const
    {
        static const std::array<std::string, 4> kDirs = {"up", "down", "left", "right"};

        std::string best_action;
        double best_dist = std::numeric_limits<double>::infinity();
        int best_fails = std::numeric_limits<int>::max();

        for (const std::string &action : kDirs)
        {
            if (!SupportsAction(action) || !IsMoveValid(action, grid)) continue;

            const WorldPosition npos = NextPos(action);
            const double nd = std::max(std::abs(npos.X() - static_cast<double>(tx)),
                                       std::abs(npos.Y() - static_cast<double>(ty)));
            const int fc = FailCount(action);

            if (nd < best_dist || (nd == best_dist && fc < best_fails))
            {
                best_dist = nd;
                best_fails = fc;
                best_action = action;
            }
        }

        /** Fallback: cornered or all moves increase distance. */
        if (best_action.empty())
        {
            for (const std::string &action : kDirs)
            {
                if (SupportsAction(action) && IsMoveValid(action, grid))
                { best_action = action; break; }
            }
        }

        return best_action;
    }

    /**
     * Holds a preferred direction for mRoamChangeTick ticks, then cycles to a new one.
     * Falls back through remaining directions if the preferred one is blocked.
     */
    std::string HunterAgent::RoamMove(const WorldGrid &grid)
    {
        static const std::array<std::string, 4> kDirs = {"up", "right", "down", "left"};

        ++mRoamTicks;
        if (mRoamTicks >= mRoamChangeTick)
        {
            /** Cycle direction deterministically using accumulated failure counts for variety. */
            mRoamDirection = (mRoamDirection + 1 +
                              (fail_up + fail_down + fail_left + fail_right) % 3) %
                             static_cast<int>(kDirs.size());
            mRoamTicks = 0;
        }

        const std::string preferred = kDirs[static_cast<size_t>(mRoamDirection)];
        if (SupportsAction(preferred) && IsMoveValid(preferred, grid))
            return preferred;

        /** Preferred direction blocked; try remaining directions in rotation. */
        for (size_t i = 1; i < kDirs.size(); ++i)
        {
            const std::string alt = kDirs[(static_cast<size_t>(mRoamDirection) + i) % kDirs.size()];
            if (SupportsAction(alt) && IsMoveValid(alt, grid))
            {
                mRoamDirection = static_cast<int>((static_cast<size_t>(mRoamDirection) + i) % kDirs.size());
                mRoamTicks = 0;
                return alt;
            }
        }

        return "up"; /** Completely boxed in; let the world reject it. */
    }

    /** Returns true if the action leads to a valid, traversable cell. */
    bool HunterAgent::IsMoveValid(const std::string &action,
                                  const WorldGrid &grid) const
    {
        const WorldPosition next = NextPos(action);
        return grid.IsValid(next) && grid.IsTraversable(grid[next]);
    }

    /** Returns the grid position resulting from taking the named action. */
    WorldPosition HunterAgent::NextPos(const std::string &action) const
    {
        const WorldPosition cur = GetLocation().AsWorldPosition();
        if (action == "up")    return cur.Up();
        if (action == "down")  return cur.Down();
        if (action == "left")  return cur.Left();
        if (action == "right") return cur.Right();
        return cur;
    }

    /** Returns the consecutive failure count for the given action direction. */
    int HunterAgent::FailCount(const std::string &action) const
    {
        if (action == "up")    return fail_up;
        if (action == "down")  return fail_down;
        if (action == "left")  return fail_left;
        if (action == "right") return fail_right;
        return 0;
    }

    bool HunterAgent::SupportsAction(const std::string &name) const
    {
        return HasAction(name);
    }

    size_t HunterAgent::LookupActionID(const std::string &name) const
    {
        return GetActionID(name);
    }

} // namespace cse498
