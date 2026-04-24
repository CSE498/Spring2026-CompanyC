/**
 * @file ActionLog.hpp
 * @author Group 23
 *
 * @brief Tracks all the actions dones by agents within a world
 */
#pragma once

#include "../core/AgentBase.hpp"
#include "../core/Database.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498
{  

  // Data structure holding all the information about the action
  struct ActionEntry {
    std::chrono::microseconds timeOfAction;
    size_t actionType;
    std::chrono::microseconds duration;
  };

  class ActionLog {
  private:
    // Unordered map holding all the actions done by all the agents using their ID's as a key
    std::unordered_map<size_t, std::vector<ActionEntry>> AgentActions;
    
  public:
    /// Constructor
    ActionLog() = default;
    
    /**
     * @brief Records the given action for the given agent
     * 
     * @param id the id of the agent the action belongs to
     * @param action the action the agent is performing
     * @param time the time when the action has started
     */
    void recordAction(size_t id, size_t action, std::chrono::microseconds time) {
      ActionEntry entry{time, action, std::chrono::microseconds::zero()};
      AgentActions[id].push_back(entry);
    }

    /**
     * @brief Returns the action log
     * 
     * @return The unordered map holding all the actions
     */
    [[nodiscard]] const std::unordered_map<size_t, std::vector<ActionEntry>>& getActions() const {
      return AgentActions;
    }
    
    /**
     * @brief Returns all the actions done by one agent
     * 
     * @param agent the id of the agent whose actions is requested
     * @return The vector of all the agents actions
     */
    [[nodiscard]] const std::vector<ActionEntry>& getActionsByAgent(size_t id) const {
      auto it = AgentActions.find(id);
      
      if (it != AgentActions.end()) {
        return it->second;
      }

      static const std::vector<ActionEntry> empty;
      return empty;
    }

    /**
     * @brief Ends an agents action if it took time to complete
     * 
     * @param agent the agent whos action is ending
     * @param time the time at which the action was completed
     */
    void actionEnd(size_t id, std::chrono::microseconds time){
      auto it = AgentActions.find(id);

      if (it != AgentActions.end() && !it->second.empty()) {
          auto& last = it->second.back();
          last.duration = std::chrono::duration_cast<std::chrono::microseconds>(
              time - last.timeOfAction);
      }
    }
    
    /**
     * @brief Clears the action log
     */
    void clear() {
      AgentActions.clear();
    }

    /**
     * @brief Obtains the number of actions in a world
     * 
     * @return the number of actions
     */
    [[nodiscard]] int getNumberofActions(){
      int count = 0;

      for (const auto& [key, value] : AgentActions) {
          count += value.size();
      }

      return count;
    }

    /**
     * @brief Returns the amount of times a specific action was done
     * 
     * @param action the action that is wanted
     * @return the number of times that action was done
     */
    [[nodiscard]] int getNumOfSpecificAction(size_t action){
      int count = 0;

      for (const auto& [key, value] : AgentActions) {
          for (const auto& entry: value){
            if (entry.actionType == action){
              ++count;
            }
          }
      }

      return count;
    }

    /**
     * @brief registers the struct ActionEntry with the database
     * 
     * @param db the database that the struct will be saved on
     */
    void RegisterWithDatabase(cse498::Database& db) {
      db.RegisterType<ActionEntry>("ActionEntry",
          [](const ActionEntry& e) -> std::string {
              cse498::Serializer s;
              return s.Serialize(static_cast<long long>(e.timeOfAction.count()))
                  + s.Serialize(e.actionType)
                  + s.Serialize(static_cast<long long>(e.duration.count()));
          },
          [](const std::string& data) -> std::optional<ActionEntry> {
              cse498::Serializer s;
              size_t pos = 0;
              auto tp  = s.DeserializeAt<long long>(data, pos);
              auto act = s.DeserializeAt<size_t>(data, pos);
              auto dur = s.DeserializeAt<long long>(data, pos);
              if (!tp || !act || !dur) return std::nullopt;
              return ActionEntry{
                  std::chrono::microseconds(*tp),
                  *act,
                  std::chrono::microseconds(*dur)
              };
          }
      ); 
    }

  };
}