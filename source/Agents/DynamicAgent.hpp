/**
 * @file DynamicAgent.hpp
 * @author Ahmed Ezaz Labib
 *
 * Agent types:
 *   - Leader: builds only
 *   - Collector (default): gathers the least-needed resource
 *   - Farmer: collects wheat only
 *   - Miner: collects stone only
 *   - Woodsman: collects wood only
 *   - Ghost: moves only, no collection or building
 *
 * @example Best used with DynamicWorld:
 * @code
 *   world.AddAgent<cse498::DynamicAgent>("Builder")
 *       .SetLeader(true)
 *       .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
 *
 *   world.AddAgent<cse498::DynamicAgent>("Farmer")
 *       .SetFarmer()
 *       .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
 *
 *   world.AddAgent<cse498::DynamicAgent>("Miner")
 *       .SetMiner()
 *       .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
 *
 *   world.AddAgent<cse498::DynamicAgent>("Woodsman")
 *       .SetWoodsman()
 *       .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
 *
 *   world.AddAgent<cse498::DynamicAgent>("Collector")
 *       .SetCollector()
 *       .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
 * @endcode
 **/

#pragma once

#include <string>
#include "../core/AgentBase.hpp"
#include "../core/WorldBase.hpp"

namespace cse498
{

    class DynamicAgent : public AgentBase
    {
    public:
        enum class AgentType
        {
            Collector,
            Farmer,
            Miner,
            Woodsman,
            Leader,
            Ghost
        };

        DynamicAgent(size_t id,
                     const std::string &name,
                     WorldBase &world,
                     AgentType type = AgentType::Collector)
            : AgentBase(id, name, world)
        {
            SetType(type);
        }

        ~DynamicAgent() = default;

        /** Sets the agent's role and specialty based on the given AgentType. */
        DynamicAgent &SetType(AgentType type)
        {
            is_leader = false;
            is_ghost  = false;
            specialty = Specialty::None;
            switch (type)
            {
            case AgentType::Leader:    is_leader = true;               break;
            case AgentType::Farmer:    specialty = Specialty::Farmer;  break;
            case AgentType::Miner:     specialty = Specialty::Miner;   break;
            case AgentType::Woodsman:  specialty = Specialty::Woodsman;break;
            case AgentType::Ghost:     is_ghost  = true;               break;
            case AgentType::Collector:
            default:                                                    break;
            }
            return *this;
        }

        /** Sets or clears the leader role; leaders do not collect resources. */
        DynamicAgent &SetLeader(bool v)
        {
            if (v) { is_leader = true; is_ghost = false; specialty = Specialty::None; }
            else   { is_leader = false; }
            return *this;
        }

        /** Configures the agent as a generic collector with no specialty. */
        DynamicAgent &SetCollector()
        {
            is_leader = false; is_ghost = false; specialty = Specialty::None;
            return *this;
        }

        /** Configures the agent to collect wheat only. */
        DynamicAgent &SetFarmer()
        {
            is_leader = false; is_ghost = false; specialty = Specialty::Farmer;
            return *this;
        }

        /** Configures the agent to collect stone only. */
        DynamicAgent &SetMiner()
        {
            is_leader = false; is_ghost = false; specialty = Specialty::Miner;
            return *this;
        }

        /** Configures the agent to collect wood only. */
        DynamicAgent &SetWoodsman()
        {
            is_leader = false; is_ghost = false; specialty = Specialty::Woodsman;
            return *this;
        }

        /** Configures the agent to only move; no collection or building. */
        DynamicAgent &SetGhost()
        {
            is_leader = false; is_ghost = true; specialty = Specialty::None;
            return *this;
        }

        bool   Initialize() override;
        size_t SelectAction(WorldGrid &grid) override;
        void   Notify(const std::string &message,
                      const std::string &msg_type = "none") override;

    private:
        bool is_leader = false;
        bool is_ghost  = false;

        enum class Specialty { None, Farmer, Miner, Woodsman };
        Specialty specialty = Specialty::None;

        enum class BuildPhase { Quarry, Farm, Lumberyard, Townhall, Done };
        BuildPhase build_phase = BuildPhase::Quarry;
        int  phase_built_count = 0;
        bool waiting_for_build = false;

        /** Tick counter used for periodic resource logging by the leader. */
        size_t mTickCount = 0;

        /** Shared resource pool across all agents; synced from world_global_counts each tick. */
        static size_t pool_wood;
        static size_t pool_stone;
        static size_t pool_wheat;
        static size_t pool_steel;

        /** Reads world_global_counts into the static pool variables. */
        void SyncPool();

        /** Returns true if the shared pool meets the cost of the current build phase. */
        bool CanAfford() const;

        /** Returns the action name for the current build phase. */
        std::string BuildAction() const;

        /** Returns the resource cell type this agent should currently collect. */
        std::string WhatToCollect() const;

        /** Returns the action name that moves the agent toward the nearest cell of the given type. */
        std::string MoveTo(const std::string &cell, const WorldGrid &grid) const;

        /** Returns the grid position resulting from taking the named action. */
        WorldPosition NextPos(const std::string &action) const;
    };

} // namespace cse498
