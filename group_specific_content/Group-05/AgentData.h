/**
 * @file AgentData.h
 * @author ezazl
 *
 * @brief
 * Shared, lightweight data definitions for Group 5 agent psychology.
 *
 * This header intentionally contains ONLY plain data structures (no behavior).
 * The goal is to make these types safe to include from anywhere in the codebase
 * (World modules, UI, analytics, serialization, tests) without pulling in the
 * implementation of the agent logic.
 *
 * Architectural intent:
 *  - Tendencies = slow-changing "traits" or sensitivities (personality knobs).
 *    These represent how strongly an agent cares about certain kinds of outcomes.
 *    They should usually change slowly (or remain fixed in the simplest version).
 *
 *  - State = fast-changing "pressures" or internal levels (moment-to-moment).
 *    These evolve each tick as time passes and as the agent experiences outcomes.
 *    The agent's dynamics code (e.g., AgentInternal) is responsible for updating
 *    these values.
 *
 * Why this file exists:
 *  - Keeps the schema of agent internals centralized.
 *  - Avoids circular dependencies (logic modules depend on data, not vice versa).
 *  - Makes it easy to log/visualize agent internals across the whole company project
 *    without including heavy agent code.
 *
 * Extensibility:
 *  - We can safely add new tendency or state fields over time as the simulation grows.
 *  - When adding fields, keep them:
 *      (1) simple numeric primitives,
 *      (2) well-documented,
 *      (3) consistent in range/meaning across all agents.
 *
 * Conventions:
 *  - Values are intended to be normalized floats in the range [0.0, 1.0].
 *    (Not enforced here; enforcement belongs in the logic/update code.)
 */

#ifndef AGENTDATA_H
#define AGENTDATA_H

namespace Group5 {

/**
 * @brief Slow-changing traits / sensitivities that shape agent behavior.
 *
 * These are not "emotions happening right now." Instead, they are biases that
 * affect how quickly certain internal pressures grow, how strongly outcomes are
 * valued, and how the agent tends to respond over many ticks.
 *
 * Intended range: [0.0, 1.0]
 */
struct Tendencies {
  /// Sensitivity to novelty and exploration; higher => boredom builds faster,
  /// and novel outcomes are valued more.
  float curiosity = 0.5f;

  /// Sensitivity to bonding/affiliation/connection; higher => loneliness builds
  /// faster when socially "unmet", and social outcomes are valued more.
  float love = 0.5f;

  /// Sensitivity to threat/uncertainty; higher => anxiety/risk pressures rise
  /// faster and avoidance becomes more attractive.
  float fear = 0.5f;

  /// Sensitivity to stability/ease/rest; higher => fatigue/discomfort matters more,
  /// and low-effort/stable outcomes are valued more.
  float comfort = 0.5f;

  /// General preference for "feeling okay" / maintaining well-being.
  /// In early versions this can be treated as a global weighting that influences
  /// how strongly negative internal states are resisted.
  ///
  /// Note: This is a placeholder trait and may be renamed/refined later
  /// (e.g., "homeostasis", "wellbeing", or split into multiple traits).
  float satisfaction = 0.5f;
};

/**
 * @brief Fast-changing internal levels / pressures (agent’s current condition).
 *
 * These represent the agent's momentary internal status. They change each tick as:
 *  - time passes (baseline regulation),
 *  - the agent experiences events/outcomes (later integration with the world),
 *  - internal variables influence each other (couplings).
 *
 * Intended range: [0.0, 1.0]
 */
struct State {
  /// Pressure to seek novelty/stimulation (exploration bias later).
  float boredom = 0.0f;

  /// Pressure to seek social connection (approach/affiliate bias later).
  float loneliness = 0.0f;

  /// Pressure reflecting perceived threat/uncertainty (avoidance bias later).
  float anxiety = 0.0f;

  /// Pressure reflecting energy depletion (rest/low-effort bias later).
  float fatigue = 0.0f;

  /// A general satisfaction/meaning signal that can buffer other negative pressures.
  float fulfillment = 0.0f;
};

} // namespace Group5

#endif // AGENTDATA_H
