#pragma once

#include <optional>
#include <string>

#include "../core/AgentBase.hpp"

namespace cse498 {

/**
 * @brief An enemy agent that patrols until the player enters its vision radius,
 * then attempts to chase and attack.
 */
class EnemyAgent : public AgentBase {
private:
  bool horizontal =
      true;          ///< Patrol axis: true for horizontal, false for vertical.
  int direction = 1; ///< Patrol direction: 1 for right/down, -1 for left/up.
  size_t vision_radius =
      5; ///< Maximum Manhattan distance at which the player can be detected.
  std::string target_name =
      "Player"; ///< Substring used to identify the player agent by name.
  bool player_in_vision =
      false; ///< Whether the player is currently within vision radius.
  bool player_adjacent = false; ///< Whether the player is currently adjacent.
  std::optional<WorldPosition>
      sensed_player_pos; ///< Most recently sensed player position.
  std::optional<std::string>
      chase_action; ///< Movement action that advances toward the player.
  std::optional<std::string>
      patrol_action; ///< Movement action that advances along the patrol path.

  /**
   * @brief Update the enemy's sensed state for the current tick.
   * @param grid The world grid visible to the agent.
   */
  void Sense(WorldGrid &grid);

public:
  /**
   * @brief Construct a new EnemyAgent.
   * @param id Unique agent identifier.
   * @param name Display name for this agent.
   * @param world Reference to the world containing this agent.
   */
  EnemyAgent(size_t id, const std::string &name, const WorldBase &world);

  /**
   * @brief Destroy the EnemyAgent.
   */
  ~EnemyAgent() = default;

  /**
   * @brief Set this enemy to patrol horizontally.
   * @return Reference to this enemy agent.
   */
  EnemyAgent &SetHorizontal();

  /**
   * @brief Set this enemy to patrol vertically.
   * @return Reference to this enemy agent.
   */
  EnemyAgent &SetVertical();

  /**
   * @brief Reverse the current patrol direction.
   * @return Reference to this enemy agent.
   */
  EnemyAgent &ToggleDirection();

  /**
   * @brief Set the enemy's vision radius.
   * @param r Maximum Manhattan detection distance.
   * @return Reference to this enemy agent.
   */
  EnemyAgent &SetVisionRadius(size_t r);

  /**
   * @brief Set the substring used to identify the player by name.
   * @param name Target name substring.
   * @return Reference to this enemy agent.
   */
  EnemyAgent &SetTargetName(const std::string &name);

  /**
   * @brief Verify that the enemy has the actions it needs to function.
   * @return True if required actions are available.
   */
  bool Initialize() override;

  /**
   * @brief Select the next action for this enemy.
   * @param grid The current world grid.
   * @return The chosen action ID.
   */
  size_t SelectAction(WorldGrid &grid) override;
};

} // namespace cse498
