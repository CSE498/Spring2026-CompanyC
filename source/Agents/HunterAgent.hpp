/**
 * @file HunterAgent.hpp
 * @author Ahmed Ezaz Labib, Shashank Papani
 *
 * Hunter enemy agent for InteractionHeavyWorld.
 *
 * Behavior overview:
 *   ROAM : Default state. Wanders pseudo-randomly, preferring the current
 *            direction to avoid jittering in place.
 *
 *   ALERT : Triggered when a target is detected within mDetectRadius tiles
 *            (Chebyshev). One-tick reaction delay before committing to CHASE.
 *
 *   CHASE : Full pursuit using Chebyshev-greedy pathfinding. Returns to ROAM
 *            if the target moves beyond mChaseRadius.
 *
 * Attack:
 *   HP damage is handled by InteractionHeavyWorld via ApplyEnemyContactDamage().
 *   The hunter only needs to reach an adjacent cell.
 *
 * Target detection:
 *   Scans each tick for the player/interface agent in the world. Older worlds
 *   can still expose target cells named "player" or "agent" in the grid.
 *
 * Supported actions: up, down, left, right
 **/

#pragma once

#include <climits>
#include <string>
#include <vector>
#include "../core/AgentBase.hpp"

namespace cse498
{

    class HunterAgent : public AgentBase
    {
    public:

        /** Chebyshev radius within which the hunter detects any target. */
        int mDetectRadius = 8;

        /** Chebyshev radius within which the hunter maintains active chase; should be >= mDetectRadius. */
        int mChaseRadius = 12;

        /** Ticks the hunter continues chasing after losing sight of the target. */
        int mChaseMemoryTicks = 10;

        /** Sets the detection radius. */
        HunterAgent &SetDetectRadius(int v) { mDetectRadius = v; return *this; }

        /** Sets the chase radius. */
        HunterAgent &SetChaseRadius(int v) { mChaseRadius = v; return *this; }

        /** Sets the number of chase-memory ticks. */
        HunterAgent &SetChaseMemory(int v) { mChaseMemoryTicks = v; return *this; }

        enum class State
        {
            Roam,  /** No target detected; wander. */
            Alert, /** Target just detected; one-tick orientation delay. */
            Chase  /** Actively pursuing last known target position. */
        };

        HunterAgent(size_t id, const std::string &name, WorldBase &world)
            : AgentBase(id, name, world) {}

        ~HunterAgent() = default;

        bool Initialize() override;
        size_t SelectAction(WorldGrid &grid) override;
        void Notify(const std::string &message,
                    const std::string &msg_type = "none") override;

    protected:

        State mState = State::Roam;
        int mChaseMemory = 0;    /** Ticks remaining on chase memory. */
        int mRoamDirection = 0;  /** Current roam direction index (0–3). */
        int mRoamTicks = 0;      /** Ticks spent in the current roam direction. */
        int mRoamChangeTick = 6; /** Ticks before picking a new roam direction. */

        /** Last known target position; updated every tick a target is visible. */
        int mTargetX = -1;
        int mTargetY = -1;

        int fail_up = 0;
        int fail_down = 0;
        int fail_left = 0;
        int fail_right = 0;

        std::string last_action;

        /**
         * Scans the grid for the nearest target ("player" or "agent") within the given Chebyshev radius.
         * Also accepts positions injected via Notify("x,y", "target_position").
         * Returns true and sets out_x / out_y if a target is found.
         */
        bool ScanForTarget(const WorldGrid &grid,
                           int &out_x, int &out_y,
                           int radius) const;

        /** Returns the action name that minimizes Chebyshev distance to (tx, ty). */
        std::string ChaseMove(const WorldGrid &grid, int tx, int ty) const;

        /** Returns a pseudo-random roam action, preferring the current direction. */
        std::string RoamMove(const WorldGrid &grid);

        /** Returns true if the given action leads to a valid, traversable cell. */
        bool IsMoveValid(const std::string &action, const WorldGrid &grid) const;

        /** Returns the grid position resulting from taking the named action. */
        WorldPosition NextPos(const std::string &action) const;

        /** Returns the consecutive failure count for the given action direction. */
        int FailCount(const std::string &action) const;

        virtual bool SupportsAction(const std::string &name) const;
        virtual size_t LookupActionID(const std::string &name) const;
    };

} // namespace cse498
