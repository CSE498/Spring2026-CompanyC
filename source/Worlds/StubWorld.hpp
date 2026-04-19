/**
 * @brief A simple resource-collection world for the web demo.
 * @note  Replaces the standalone StubWorldAdapter with a proper WorldBase
 *        derivation so the same simulation engine used by all other groups
 *        drives the demo.
 **/

#pragma once

#include <chrono>
#include <map>
#include <string>

#include "../core/WorldBase.hpp"
#include "../core/WorldScore.hpp"

namespace cse498 {

class Database;      // forward declaration for save/load
class WebInterface;  // forward declaration; full definition needed only in .cpp

/// @brief 10×10 resource-collection world for the Group 24 web demo.
///
/// The world is bordered by walls and contains trees, stone, and wheat that
/// the player can collect.  Collected wood can be used to build structures.
/// All game logic is driven through DoAction() so it integrates cleanly with
/// WorldBase::RunAgents() and any AgentBase-derived agent (including WebInterface).
///
/// Status messages, resource counts, and mode transitions are delivered to
/// agents via AgentBase::Notify() rather than through shared mutable state,
/// keeping the world and the UI interface decoupled.
class StubWorld : public WorldBase {
 protected:
  /// Numeric IDs for every action this world supports.
  /// Values 1-10 are intentionally matched to WebMain::ActionCode so that
  /// the JS/C++ integer bridge requires no translation table.
  enum ActionType {
    REMAIN_STILL = 0,
    START        = 1,
    RESET        = 2,
    SAVE         = 3,
    LOAD         = 4,
    MOVE_UP      = 5,
    MOVE_DOWN    = 6,
    MOVE_LEFT    = 7,
    MOVE_RIGHT   = 8,
    COLLECT      = 9,
    BUILD        = 10,
  };

  /// Register all available actions and bootstrap the agent with the
  /// world name, initial resource counts, and a welcome status message.
  void ConfigAgent(AgentBase& agent) override;

 public:
  StubWorld();
  ~StubWorld() override = default;

  /// Execute one action for the given agent.
  /// @return 1 on success, 0 on failure (status message sent via Notify).
  int DoAction(AgentBase& agent, size_t action_id) override;

  /// Set the Database pointer used for save/load (non-owning).
  void SetDatabase(Database* db) { db_ = db; }

  /// Serialize base + custom state into the Database.
  void SaveState(const std::string& world_name);

  /// Restore base + custom state from the Database.
  void LoadState(const std::string& world_name);

  [[nodiscard]] WorldScoreDisplay GetWorldScoreDisplay() const override;

 private:
  static constexpr int kWidth  = 10;
  static constexpr int kHeight = 10;

  size_t grass_id_ = 0;
  size_t tree_id_  = 0;
  size_t stone_id_ = 0;
  size_t wheat_id_ = 0;
  size_t wall_id_  = 0;
  size_t built_id_ = 0;

  bool started_ = false;
  std::map<std::string, int> resources_;

  Database* db_ = nullptr;

  /// Resource-based scoring (see GetWorldScoreDisplay).
  static constexpr int kCollectibleGoal_ = 5;  ///< trees+stone+wheat cells on the map
  int resources_collected_count_ = 0;
  int move_actions_ = 0;
  int build_actions_ = 0;
  bool objective_complete_ = false;
  bool session_clock_running_ = false;
  std::chrono::steady_clock::time_point session_start_{};

  [[nodiscard]] int ComputeStubScore_() const;
  [[nodiscard]] long long ElapsedSeconds_() const;
  void ResetScoringState_();

  /// Load the initial (or reset) grid layout from a character string map.
  void InitializeGrid_();

  /// Return true if the agent may move onto the given position.
  [[nodiscard]] bool IsWalkable_(WorldPosition pos) const;

  /// Attempt to collect a resource at (x, y).  Updates resources_ and sends
  /// a "status" and "resource" notification to agent on success.
  /// @return true if a resource was collected.
  bool TryCollectAt_(AgentBase& agent, int x, int y);

  /// Send the current value of every resource to the agent via Notify.
  void SendResources_(AgentBase& agent) const;
};

}  // namespace cse498
