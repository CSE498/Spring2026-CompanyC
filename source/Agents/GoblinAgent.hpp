/**
 * @file GoblinAgent.hpp
 * @author Joshua Thomas
 * @brief Fixed size-pool allocator for type T with a freelist for fast reuse
 *
 * @details
 *
 **/
#pragma once

#include <string>

#include "../core/AgentBase.hpp"

namespace cse498 {

/**
 * @brief A stationary doorway agent that blocks passage until the world
 * resolves payment and clears it.
 */
class GoblinAgent : public AgentBase {
private:
  bool blocking = true; ///< Whether this goblin is currently blocking passage.
  bool player_adjacent = false; ///< Whether the player is currently adjacent.
  std::string target_name =
      "Player"; ///< Substring used to identify the player.

  /**
   * @brief Update sensed state for the current tick.
   * @param grid The current world grid.
   */
  void Sense(WorldGrid &grid);

public:
  /**
   * @brief Construct a new GoblinAgent.
   * @param id Unique agent identifier.
   * @param name Display name for this agent.
   * @param world Reference to the world containing this agent.
   */
  GoblinAgent(size_t id, const std::string &name, const WorldBase &world);

  /**
   * @brief Destroy the GoblinAgent.
   */
  ~GoblinAgent() = default;

  /**
   * @brief Set whether this goblin is blocking.
   * @param value New blocking state.
   * @return Reference to this goblin.
   */
  GoblinAgent &SetBlocking(bool value);

  /**
   * @brief Clear the blocking state so passage is no longer blocked.
   * @return Reference to this goblin.
   */
  GoblinAgent &ClearBlocking();

  /**
   * @brief Check whether this goblin is still blocking.
   * @return True if this goblin blocks passage.
   */
  [[nodiscard]] bool IsBlocking() const;

  /**
   * @brief Check whether the player is currently adjacent.
   * @return True if the player is adjacent.
   */
  [[nodiscard]] bool IsPlayerAdjacent() const;

  /**
   * @brief Check whether this goblin is in a state where world payment logic
   * could pay it.
   * @return True if blocking and adjacent to the player.
   */
  [[nodiscard]] bool CanBePaid() const;

  /**
   * @brief Set the substring used to identify the player by name.
   * @param name Target name substring.
   * @return Reference to this goblin.
   */
  GoblinAgent &SetTargetName(const std::string &name);

  /**
   * @brief Verify that the goblin is ready to function.
   * @return Always true for this minimal stationary agent.
   */
  bool Initialize() override;

  /**
   * @brief Select the next action for this goblin.
   * @param grid The current world grid.
   * @return Always returns 0 so the goblin remains stationary.
   */
  size_t SelectAction(WorldGrid &grid) override;
};

} // namespace cse498
