#pragma once
#include "../tools/ActionLog.hpp"
#include "../core/AgentBase.hpp"
#include "../core/WorldBase.hpp"
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <iostream>

struct ReplayEvent {
    std::shared_ptr<cse498::AgentBase> agent;
    std::string actionType;
};

namespace cse498
{ 
    class ReplayDriver {
    private:
        WorldBase& mWorld; // Reference to world to sent actions to agents
        std::vector<ReplayEvent> mEvents{}; // list of events
        std::size_t mNext{}; // index of next event played
        bool mRunning{}; // status for if replay is running
        bool mPaused{}; // status for if replay is paused
    
    public:

        ReplayDriver(WorldBase& world) : mWorld(world) {}

         // Loading action log and start replay 
        void startReplay(const ActionLog& log) {

            if (log.getActions().empty()) return; // Check if there are actions to replay

            clearReplay(); // Clear events from any previous replay

            for (const auto& pair: log.getActions()) {

                // Create an event for each agent and their action
                for (const auto& actionEntry: pair.second) {

                    ReplayEvent event;
                    event.agent = pair.first;
                    event.actionType = actionEntry.actionType;

                    mEvents.push_back(event); // Adding event to list of events
                }
            }
            // Check if there are events to replay
            if (mEvents.empty()) return;           

            // Set replay to running
            mRunning = true;
        }

       //Sends action to the agent
        void sendAction(const ReplayEvent& event) {
            if (!event.agent) return; // check if agent is valid

            size_t action_id = event.agent->GetActionID(event.actionType); // Convert action type to action id

            int result = mWorld.DoAction(*event.agent, action_id); // Send action to agent 
            event.agent->SetActionResult(result); // Set result for Agent, 1 = success, 0 = fail
        }

        // Updates replay by sending next action to agent
        void update() {
            if(!mRunning || mPaused) return; // check if replay is running or paused

            while (mNext < mEvents.size()) {
                sendAction(mEvents[mNext]);
                mNext++;
            }

            // Check if replay is finished
            if(isFinished()) {
                mRunning = false;
                return;
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

            mPaused = true; // Set paused status to true
        }

        // Resumes replay if paused
        void resumeReplay() {

            // Check to make sure replay is running and paused
            if (!mRunning || !mPaused) return;

            mPaused = false; // Set paused status to false to resume replay
        }

        // Resets replay
        void resetReplay() {
                mNext = 0;
                mPaused = false;
                mRunning = false;
        }

        //Clears events from replay
        void clearReplay() {
            if (mEvents.size() > 0) {
                mEvents.clear();
            }
        }
    };
}