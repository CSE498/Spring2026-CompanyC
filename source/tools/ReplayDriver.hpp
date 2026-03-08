#pragma once

#include "../tools/ActionLog.hpp"
#include "../core/AgentBase.hpp"
#include "../core/WorldBase.hpp"

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <chrono>

namespace cse498
{ 

    struct ReplayEvent {
    size_t agent_id;
    std::string actionType;
    std::chrono::high_resolution_clock::time_point time;
    };

    class ReplayDriver {
    private:
        WorldBase& mWorld; // Reference to world to sent actions to agents
        std::vector<ReplayEvent> mEvents{}; // list of events
        std::size_t mNext{}; // index of next event played
        bool mRunning{}; // status for if replay is running
        bool mPaused{}; // status for if replay is paused
    
    public:

        ReplayDriver(WorldBase& world) : mWorld(world) {}

        /**
        * @brief Loads events from an ActionLog and prepares replay.
        *
        * Clears any previous replay data, copies events from the log,
        * sorts them chronologically, and prepares replay state.
        *
        * @param log ActionLog containing recorded actions
        */
        void startReplay(const ActionLog& log) {

            clearReplay();

            if (log.getActions().empty()) return;


            for (const auto& [agent_id, entries] : log.getActions()) {
                // Create an event for each agent and their action
                for (const auto& actionEntry: entries) {

                    ReplayEvent event;
                    event.agent_id = agent_id;
                    event.actionType = actionEntry.actionType;
                    event.time = actionEntry.timeOfAction;
                    mEvents.push_back(event); 
                }
            }

            if (mEvents.empty()) return;     

            // Sort chronologically so events are played in correct order
            std::sort(mEvents.begin(), mEvents.end(), [](const ReplayEvent& a, const ReplayEvent& b) {
                return a.time < b.time;
            });

            mRunning = true;
            mPaused = false;
            mNext = 0;
        }

        /**
        * @brief Executes a single ReplayEvent by sending the action to the agent.
        *
        * Retrieves the agent from the world using agent_id,
        * converts the action string to an action ID,
        * and executes it via WorldBase::DoAction().
        *
        * @param event The replay event to execute
        */
        void sendAction(const ReplayEvent& event) {

            AgentBase& agent = mWorld.GetAgent(event.agent_id);

            const size_t action_id = agent.GetActionID(event.actionType); // Convert action type to action id
            const int result = mWorld.DoAction(agent, action_id); // Send action to agent 
            agent.SetActionResult(result); // Set result for Agent, 1 = success, 0 = fail
        }

        /**
        * @brief Advances replay by executing the next event.
        *
        * Executes exactly one event per call to allow step-by-step replay.
        * Stops replay automatically when all events are processed.
        */
        void update() {
            if(!mRunning || mPaused) return;

            if (mNext < mEvents.size()) {
                sendAction(mEvents[mNext]);
                mNext++;
            }

            if(isFinished()) {
                mRunning = false;
            }
        }

       // Status for if replay is done
        bool isFinished() const {
            if(mNext < mEvents.size()) {
                return false;
            }   
            return true;
        }

       // Status for if replay is running
        bool isRunning() const {
            return mRunning;
        }

       // Status for if replay is paused
        bool isPaused() const {
            return mPaused;
        }

        // Pauses replay if replay is running
        void pauseReplay() {
            // Check to make sure replay is running and not already paused
            if (!mRunning || mPaused) return;

            mPaused = true;
        }

        // Resumes replay if paused
        void resumeReplay() {

            // Check to make sure replay is running and paused
            if (!mRunning || !mPaused) return;

            mPaused = false;
        }

        /**
        * @brief Resets replay progress without clearing loaded events.
        *
        * Allows replay to be restarted from the beginning.
        */
        void resetReplay() {
                mNext = 0;
                mPaused = false;
                mRunning = false;
        }

        /**
        * @brief Clears all replay events and resets replay state.
        */
        void clearReplay() {
            mEvents.clear();
            mNext = 0;
            mRunning = false;
            mPaused = false;
        }
    };
}