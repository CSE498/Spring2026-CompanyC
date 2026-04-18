/**
 * @file DynamicAgent.hpp
 * @author Ahmed Ezaz Labib, Shashank Papani
 *
 * Agent types:
 *   - Leader: builds only
 *   - Collector (default): gathers the least-needed resource
 *   - Farmer: collects wheat only
 *   - Miner: collects stone only
 *   - Woodsman: collects wood only
 *
 * @example Best used to fulfill win condition of DynamicWorld:
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
            Leader
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
            specialty = Specialty::None;
            switch (type)
            {
            case AgentType::Leader:
                is_leader = true;
                break;
            case AgentType::Farmer:
                specialty = Specialty::Farmer;
                break;
            case AgentType::Miner:
                specialty = Specialty::Miner;
                break;
            case AgentType::Woodsman:
                specialty = Specialty::Woodsman;
                break;
            case AgentType::Collector:
            default:
                break;
            }
            return *this;
        }

        /** Sets or clears the leader role; leaders do not collect resources. */
        DynamicAgent &SetLeader(bool v)
        {
            if (v)
            {
                is_leader = true;
                specialty = Specialty::None;
            }
            else
            {
                is_leader = false;
            }
            return *this;
        }

        /** Configures the agent as a generic collector with no specialty. */
        DynamicAgent &SetCollector()
        {
            is_leader = false;
            specialty = Specialty::None;
            return *this;
        }

        /** Configures the agent to collect wheat only. */
        DynamicAgent &SetFarmer()
        {
            is_leader = false;
            specialty = Specialty::Farmer;
            return *this;
        }

        /** Configures the agent to collect stone only. */
        DynamicAgent &SetMiner()
        {
            is_leader = false;
            specialty = Specialty::Miner;
            return *this;
        }

        /** Configures the agent to collect wood only. */
        DynamicAgent &SetWoodsman()
        {
            is_leader = false;
            specialty = Specialty::Woodsman;
            return *this;
        }

        bool Initialize() override;
        size_t SelectAction(WorldGrid &grid) override;
        void Notify(const std::string &message,
                    const std::string &msg_type = "none") override;

    private:
        bool is_leader = false;

        enum class Specialty
        {
            None,
            Farmer,
            Miner,
            Woodsman
        };
        Specialty specialty = Specialty::None;

        enum class BuildPhase
        {
            Quarry,
            Farm,
            Lumberyard,
            Townhall,
            Done
        };
        BuildPhase build_phase = BuildPhase::Quarry;
        int phase_built_count = 0;
        bool waiting_for_build = false;

        /** Per-agent resource counts. */
        size_t wood = 0, stone = 0, wheat = 0, steel = 0;

        /** Shared resource pool across all agents; updated exclusively via Notify. */
        static size_t pool_wood, pool_stone, pool_wheat, pool_steel;

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
