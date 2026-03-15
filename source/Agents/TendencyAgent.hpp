/**
 * @file TendencyAgent.h
 * @author ezazl
 *
 * A simple tendency-based agent built in the spirit of PacingAgent.
 *
 * This class is intentionally kept small and modular.
 * The goal is to preserve the readability of PacingAgent while introducing
 * a richer internal decision skeleton:
 *
 *   tendencies -> stable motivational pulls
 *   state      -> current internal condition
 *   mode       -> current behavioral mindset
 *   memory     -> recent experience
 *
 * The agent still exposes one main decision function:
 *
 *   SelectAction(...)
 *
 * But instead of following one fixed rule, that action is meant to emerge
 * from the interaction of these internal pieces.
 */

#pragma once

#include <string>
#include <limits>

#include "../core/AgentBase.hpp"

namespace cse498 {

class TendencyAgent : public AgentBase {
protected:
  /**
   * The agent's current behavioral mindset.
   *
   * Mode is not itself the final action, but it acts as the agent's current
   * "state of mind" and strongly affects which actions will be valued.
   *
   * Example:
   *   Explore -> movement and novelty matter more
   *   Pursue  -> actions toward a target matter more
   *   Flee    -> safety matters more
   *   Recover -> self-preservation matters more
   *
   * In a fuller implementation, mode would typically be updated from the
   * interaction of tendencies, state, and recent memory.
   */
  enum class Mode {
    Explore,
    Pursue,
    Flee,
    Recover,
    Idle
  };

  /// The agent's currently active behavioral mode.
  Mode current_mode = Mode::Explore;

  /**
   * Represents the agent's current internal condition.
   *
   * These values are temporary and situational rather than permanent.
   * They may rise or fall as the agent acts and reacts to the world.
   *
   * Conceptually:
   *   tendencies = what kind of agent this is
   *   state      = what condition the agent is currently in
   *
   * Example:
   *   fear rising may push the agent toward Flee
   *   low health may push the agent toward Recover
   */
  struct State {
    double health = 100.0;        ///< Current physical condition
    double energy = 100.0;        ///< Current stamina / readiness to act

    double fear = 0.0;            ///< Pressure toward safer behavior
    double confidence = 50.0;     ///< Belief in likely success
    double curiosity = 50.0;      ///< Current pull toward exploration
    double aggression = 0.0;      ///< Current pull toward confrontation

    bool under_threat = false;    ///< Fast signal that danger is currently relevant
    bool low_health = false;      ///< Convenience flag derived from health
    bool recently_blocked = false;///< Recent failure signal (e.g. failed move)
  };

  /// The agent's current internal state.
  State state;

  /**
   * Represents the agent's stable motivational biases.
   *
   * These are the longer-term pulls that shape how the agent tends to behave.
   * They are closer to "personality" than to momentary condition.
   *
   * Example:
   *   explore high -> agent is naturally more exploratory
   *   safety high  -> agent is naturally more cautious
   *   goal high    -> agent holds onto objectives more strongly
   *
   * In a fuller implementation, these values would influence both which mode
   * becomes active and how candidate actions are scored.
   */
  struct Tendencies {
    double explore = 1.0;         ///< Pull toward novelty, movement, discovery
    double safety = 1.0;          ///< Pull toward caution and self-preservation
    double goal = 1.0;            ///< Pull toward pursuing objectives
    double aggression = 0.5;      ///< Pull toward confrontation
    double recovery = 0.5;        ///< Pull toward resting / healing
    double persistence = 0.5;     ///< Pull toward repeating successful actions
    double avoidance = 0.5;       ///< Pull away from recently failed actions
  };

  /// The agent's stable tendency weights.
  Tendencies tendencies;

  /**
   * Stores recent events so the agent does not behave as if every turn is
   * completely independent.
   *
   * This is short-term memory only.
   * It is meant to provide continuity across turns:
   *
   *   what just happened?
   *   what was last tried?
   *   was danger seen recently?
   *   was an opportunity seen recently?
   *
   * In a fuller implementation, memory would influence state and mode.
   */
  struct ShortTermMemory {
    int last_action_result = 1;       ///< Result of previous action
    std::string last_action_name = "";///< Name of previous action
    int failed_moves_in_row = 0;      ///< Useful for detecting being stuck

    bool saw_enemy_recently = false;  ///< Recent threat signal
    bool saw_item_recently = false;   ///< Recent opportunity signal

    int turns_since_enemy = 9999;     ///< Age since last enemy sighting
    int turns_since_item = 9999;      ///< Age since last item sighting
    int turns_since_damage = 9999;    ///< Age since last damage event
  };

  /// The agent's recent memory.
  ShortTermMemory short_memory;

  /**
   * Very lightweight movement memory.
   *
   * This is not full map memory. It simply tracks whether movement in each
   * direction has tended to succeed or fail recently.
   *
   * This gives the agent a simple form of continuity and adaptation without
   * introducing a large planning system.
   */
  struct DirectionMemory {
    int up_failures = 0;
    int down_failures = 0;
    int left_failures = 0;
    int right_failures = 0;

    int up_successes = 0;
    int down_successes = 0;
    int left_successes = 0;
    int right_successes = 0;
  };

  /// Simple directional success/failure memory.
  DirectionMemory direction_memory;

  /**
   * Stores a persistent objective across turns.
   *
   * Without goal memory, the agent may change its mind every turn.
   * With goal memory, it can stay oriented toward something for a while.
   *
   * This is intentionally very small for now:
   *   has_goal      -> whether a goal exists
   *   goal_type     -> what kind of goal it is
   *   goal_location -> where that goal is believed to be
   *   goal_age      -> how stale that goal is
   */
  struct GoalMemory {
    bool has_goal = false;
    std::string goal_type = "";   ///< e.g. "item", "enemy", "exit", "patrol"
    Location goal_location;       ///< Where the goal is believed to be, included in Entity.hpp
    int goal_age = 0;             ///< How long the current goal has existed
  };

  /// Persistent goal tracking.
  GoalMemory goal_memory;

public:
  /**
   * Construct a tendency-based agent.
   *
   * Like PacingAgent, construction is intentionally simple:
   * identity, name, and world membership come from AgentBase / Entity.
   */
  TendencyAgent(size_t id, const std::string & name, const WorldBase & world)
    : AgentBase(id, name, world) { }

  ~TendencyAgent() = default;

  // ----------------------------------------------------------------------
  // Simple tendency configuration
  // ----------------------------------------------------------------------
  //
  // These setters allow the agent's stable motivational biases to be tuned
  // without changing the structure of the class.
  //
  // Example:
  //   high explore weight -> more exploratory agent
  //   high safety weight  -> more cautious agent
  //

  TendencyAgent & SetExploreWeight(double in) { tendencies.explore = in; return *this; }
  TendencyAgent & SetSafetyWeight(double in) { tendencies.safety = in; return *this; }
  TendencyAgent & SetGoalWeight(double in) { tendencies.goal = in; return *this; }
  TendencyAgent & SetAggressionWeight(double in) { tendencies.aggression = in; return *this; }
  TendencyAgent & SetRecoveryWeight(double in) { tendencies.recovery = in; return *this; }
  TendencyAgent & SetPersistenceWeight(double in) { tendencies.persistence = in; return *this; }
  TendencyAgent & SetAvoidanceWeight(double in) { tendencies.avoidance = in; return *this; }

  /**
   * This agent currently expects the four basic movement actions.
   *
   * For now, it is still a movement-oriented agent, so it needs:
   *   up, down, left, right
   *
   * Later, this could be expanded to include richer action sets.
   */
  bool Initialize() override {
    return HasAction("up") && HasAction("down")
        && HasAction("left") && HasAction("right");
  }

  /**
   * Choose the best available movement action.
   *
   * Like PacingAgent, this is still the main public decision function.
   * The difference is that instead of directly choosing one fixed direction,
   * this function is intended to:
   *
   *   1. update memory
   *   2. update internal state
   *   3. update current mode
   *   4. score available actions
   *   5. return the best one
   *
   * For now, the deeper helper functions are left as hooks for later
   * implementation so the class structure stays clear.
   */
  size_t SelectAction(const WorldGrid & grid) override
  {
    UpdateMemory();
    UpdateDirectionMemory();
    UpdateState(grid);
    UpdateMode();

    size_t best_action = 0;
    double best_score = -std::numeric_limits<double>::infinity();

    for (const std::string action_name : {"up", "down", "left", "right"}) {
      if (!HasAction(action_name)) continue;

      const double score = ScoreAction(action_name, grid);

      if (score > best_score) {
        best_score = score;
        best_action = GetActionID(action_name);
      }
    }

    RememberChosenAction(best_action);
    return best_action;
  }

  /**
   * Receive world notifications.
   *
   * For now, this is the simplest bridge from the world into the agent's
   * memory/state system.
   *
   * Example:
   *   damage     -> recent harm
   *   enemy      -> recent threat
   *   item_alert -> recent opportunity
   *
   * This can later be expanded into a Percept based system or better event handling.
   */
  void Notify(const std::string & /*message*/,
              const std::string & msg_type = "none") override
  {
    if (msg_type == "damage") {
      short_memory.turns_since_damage = 0;
      state.under_threat = true;
    }
    else if (msg_type == "enemy") {
      short_memory.saw_enemy_recently = true;
      short_memory.turns_since_enemy = 0;
    }
    else if (msg_type == "item_alert") {
      short_memory.saw_item_recently = true;
      short_memory.turns_since_item = 0;
    }
  }

protected:
  /**
   * Update short-term memory from the most recent turn.
   *
   * Intended responsibilities:
   *   - age memory counters
   *   - record latest action result
   *   - detect repeated failure
   *   - refresh recent-event flags
   *
   * This is one of the main bridges between "what just happened" and
   * future behavior.
   */
  virtual void UpdateMemory();

  /**
   * Update directional success/failure memory.
   *
   * Intended responsibilities:
   *   - remember whether movement in a direction has recently succeeded
   *     or failed
   *   - help prevent the agent from blindly repeating bad movement
   *
   * This is intentionally lightweight and local.
   */
  virtual void UpdateDirectionMemory();

  /**
   * Update the current internal state from memory and world context.
   *
   * Intended responsibilities:
   *   - compute or refresh low_health / under_threat style flags
   *   - let recent danger affect fear
   *   - let repeated failure affect confidence
   *   - let current circumstances shift internal pressure
   *
   * State is the agent's current condition, not its final decision.
   */
  virtual void UpdateState(const WorldGrid & grid);

  /**
   * Choose the current mode from tendencies, state, and memory.
   *
   * Intended responsibilities:
   *   - decide whether the agent should currently behave as exploring,
   *     pursuing, fleeing, recovering, etc.
   *   - provide a coherent current mindset before scoring actions
   *
   * This is where the internal pieces begin to turn into behavior.
   */
  virtual void UpdateMode();

  /**
   * Score one candidate action.
   *
   * Intended responsibilities:
   *   - combine the relevant tendency scores
   *   - allow current mode to shape the value of the action
   *   - return a single score that can be compared against other actions
   *
   * This is the core evaluation hook for the agent.
   */
  virtual double ScoreAction(const std::string & action_name,
                             const WorldGrid & grid) const;

  /**
   * Exploration-related part of action scoring.
   *
   * Intended idea:
   *   value movement, novelty, or discovery-related behavior
   */
  virtual double ScoreExplore(const std::string & action_name,
                              const WorldGrid & grid) const;

  /**
   * Safety-related part of action scoring.
   *
   * Intended idea:
   *   penalize risky or repeatedly bad actions and prefer safer movement
   */
  virtual double ScoreSafety(const std::string & action_name,
                             const WorldGrid & grid) const;

  /**
   * Goal-related part of action scoring.
   *
   * Intended idea:
   *   value actions that move toward or maintain an objective
   */
  virtual double ScoreGoal(const std::string & action_name,
                           const WorldGrid & grid) const;

  /**
   * Aggression-related part of action scoring.
   *
   * Intended idea:
   *   support more confrontational behavior when appropriate
   */
  virtual double ScoreAggression(const std::string & action_name,
                                 const WorldGrid & grid) const;

  /**
   * Recovery-related part of action scoring.
   *
   * Intended idea:
   *   support self-preserving behavior when energy/health are low
   */
  virtual double ScoreRecovery(const std::string & action_name,
                               const WorldGrid & grid) const;

  /**
   * Persistence-related part of action scoring.
   *
   * Intended idea:
   *   slightly favor repeating actions that have recently worked
   */
  virtual double ScorePersistence(const std::string & action_name,
                                  const WorldGrid & grid) const;

  /**
   * Avoidance-related part of action scoring.
   *
   * Intended idea:
   *   discourage repeating actions that have recently failed
   */
  virtual double ScoreAvoidance(const std::string & action_name,
                                const WorldGrid & grid) const;

  /**
   * Remember which action was chosen this turn.
   *
   * Intended responsibilities:
   *   - translate action ID back into an action name
   *   - store that name for next-turn persistence / avoidance logic
   */
  virtual void RememberChosenAction(size_t action_id);

  /**
   * Return the recent failure count for a given movement direction.
   *
   * Intended use:
   *   helper for safety / avoidance scoring
   */
  virtual int GetDirectionFailures(const std::string & action_name) const;
};

} 
