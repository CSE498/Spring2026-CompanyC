/**
* @file TendencyAgent.h
 * @auther Ahmed Ezaz Labib, Shashank Papani
 * Tendency-based agent that collects resources and constructs buildings.
 *
 * Design:
 *   - This agent operates entirely within the action space defined in
 *     DynamicWorld.cpp. It does not invent behaviors; it selects from the
 *     available actions provided by the world (movement, collect, build).
 *
 *   - Tendencies act as weights that bias decision-making:
 *       • Resource affinities → which resources the agent prefers to gather
 *       • Building desires    → which structures the agent prioritizes
 *       • Top-level tendencies → how strongly it explores, collects, builds, etc.
 *     Higher values = stronger preference when scoring actions.
 *
 *   - Each turn, the agent evaluates all possible actions available to it
 *     (as defined by DynamicWorld) and assigns a score based on:
 *       • Current surroundings (nearby resources, buildings, agents)
 *       • Its tendencies (affinities, desires, behavior weights)
 *       • Simple memory (recent successes/failures)
 *     The highest-scoring action is selected.
 *
 *   - Goal selection is implicit, not fixed:
 *       The agent does not commit to long-term plans. Instead, it re-evaluates
 *       priorities every turn by comparing:
 *         • build opportunities (if affordable)
 *         • resource gathering opportunities
 *       The strongest weighted option becomes the current behavior.
 *
 *   - Memory is intentionally minimal:
 *       • Resource counts (for affordability checks)
 *       • Last-action success/failure (for persistence)
 *       • Per-direction failure counts (for avoidance)
 *     No long-term planning or pathfinding state is stored.
 *
 * Supported actions (must match DynamicWorld.cpp):
 *   Movement : up, down, left, right, up_left, up_right, down_left, down_right
 *   Resource : collect
 *   Build    : build_lumberyard, build_quarry, build_farm,
 *              build_spawner, build_townhall
 */

#pragma once

#include <limits>
#include <string>
#include "../core/AgentBase.hpp"


namespace cse498 {

class TendencyAgent : public AgentBase {
public:
  /**
   * Tendencies
   *
   * This struct defines the agent’s personality through weighted tendencies.
   * Behavior is not determined by a single value — it emerges from the
   * combination of:
   *   1. Top-level tendencies (what the agent generally wants to do)
   *   2. Resource affinities (what resources it prefers)
   *   3. Building desires (what structures it prioritizes)
   *
   * All values are non-negative. Larger values = stronger behavioral pull.
   *
   * Example archetypes (combinations):
   *
   * Woodsman:
   *   High collect + high wood_affinity + high lumberyard_desire
   *   → Prioritizes trees, gathers wood frequently, and builds lumberyards early.
   *
   * Miner:
   *   High collect + high stone_affinity + high quarry_desire
   *   → Focuses on stone gathering and quarry development.
   *
   * Farmer:
   *   High collect + high wheat_affinity + high farm_desire
   *   → Focuses on wheat and farm production.
   *
   * Builder:
   *   High build + moderate collect + high building desires
   *   → Converts resources into structures quickly once affordable.
   *
   * Fighter:
   *   High combat + moderate explore + lower survival
   *   → Aggressively engages enemies and accepts risk.
   *
   * Scout:
   *   High explore + low build + low persistence
   *   → Constantly moves and explores rather than committing to routines.
   *
   * Social Agent:
   *   High social + moderate survival
   *   → Stays near other agents and moves in groups.
   *
   * Survivor:
   *   High survival + high avoidance + low combat
   *   → Avoids danger and unnecessary conflict.
   *
   * In short:
   *   - Tendencies control behavior
   *   - Affinities control target preference
   *   - Desires control building priorities
   *
   * Example configuration:
   *
   * auto& woodsman = world.AddAgent<cse498::TendencyAgent>("woodsman");
   * woodsman
   *   .SetWoodAffinity(3.0)
   *   .SetStoneAffinity(0.5)
   *   .SetWheatAffinity(0.5)
   *   .SetLumberyardDesire(2.0)
   *   .SetQuarryDesire(0.3)
   *   .SetFarmDesire(0.3)
   *   .SetBuild(1.2)
   *   .SetCollect(1.5)
   *   .SetExplore(0.8);
   */

  struct Tendencies {
    // Top-level behaviour weights
    double explore     = 1.0;  // desire to roam and discover new areas
    double collect     = 1.5;  // desire to gather nearby resources
    double build       = 1.0;  // desire to construct buildings when possible
    double persistence = 0.4;  // tendency to repeat recently successful actions
    double avoidance   = 0.8;  // tendency to avoid recently failed directions/actions
    double social      = 0.5;  // preference for being near other agents
    double combat      = 0.5;  // willingness to engage or chase enemies
    double survival    = 0.8;  // preference for safety and self-preservation

    // Resource affinities (scale the collect drive per resource type)
    double wood_affinity  = 1.0;  // preference for wood (trees)
    double stone_affinity = 1.0;  // preference for stone
    double wheat_affinity = 1.0;  // preference for wheat

    // Building desires (scale the build drive per building type)
    double lumberyard_desire = 0.5;  // desire to build lumberyards
    double quarry_desire     = 0.5;  // desire to build quarries
    double farm_desire       = 0.5;  // desire to build farms
    double spawner_desire    = 0.3;  // desire to build spawners
    double townhall_desire   = 1.0;  // desire to build a townhall (high priority)
  };


  Tendencies tendencies;

  // Chainable Setter methods for configuring agent tendencies; each call updates a parameter and returns *this to allow fluent configuration (e.g., agent.SetCollect(1.5).SetBuild(1.2))
  TendencyAgent& SetExplore(double v)          { tendencies.explore = v;           return *this; }
  TendencyAgent& SetCollect(double v)          { tendencies.collect = v;           return *this; }
  TendencyAgent& SetBuild(double v)            { tendencies.build = v;             return *this; }
  TendencyAgent& SetPersistence(double v)      { tendencies.persistence = v;       return *this; }
  TendencyAgent& SetAvoidance(double v)        { tendencies.avoidance = v;         return *this; }
  TendencyAgent& SetWoodAffinity(double v)     { tendencies.wood_affinity = v;     return *this; }
  TendencyAgent& SetStoneAffinity(double v)    { tendencies.stone_affinity = v;    return *this; }
  TendencyAgent& SetWheatAffinity(double v)    { tendencies.wheat_affinity = v;    return *this; }
  TendencyAgent& SetLumberyardDesire(double v) { tendencies.lumberyard_desire = v; return *this; }
  TendencyAgent& SetQuarryDesire(double v)     { tendencies.quarry_desire = v;     return *this; }
  TendencyAgent& SetFarmDesire(double v)       { tendencies.farm_desire = v;       return *this; }
  TendencyAgent& SetSpawnerDesire(double v)    { tendencies.spawner_desire = v;    return *this; }
  TendencyAgent& SetTownhallDesire(double v)   { tendencies.townhall_desire = v;   return *this; }
  TendencyAgent& SetSocial(double v)           { tendencies.social = v;            return *this; }
  TendencyAgent& SetCombat(double v)           { tendencies.combat = v;            return *this; }
  TendencyAgent& SetSurvival(double v)         { tendencies.survival = v;          return *this; }



  // Enumerates the agent’s current high-level objective, selected each turn based on tendencies and world state (no long-term commitment)
  enum class Goal {
    None,              // no strong objective; default fallback

    // Resource collection goals (driven by collect tendency + resource affinities)
    CollectWood,
    CollectStone,
    CollectWheat,

    // Building goals (only considered if the agent can afford the structure)
    BuildLumberyard,
    BuildQuarry,
    BuildFarm,
    BuildSpawner,
    BuildTownhall
  };

  /** Constructor */
  TendencyAgent(size_t id, const std::string& name, WorldBase& world)
    : AgentBase(id, name, world) {}

  /** Destructor */
  ~TendencyAgent() = default;

  //Verifies required actions exist in DynamicWorld before the agent can run
  bool Initialize() override;

  //Selects the best action each turn by scoring all available actions from the world
  size_t SelectAction(const WorldGrid& grid) override;

  //Receives feedback/events from the world (e.g., failures, resources) to update internal memory
  void Notify(const std::string& message,
              const std::string& msg_type = "none") override;

protected:
  //Minimal memory used for decision-making (no long-term planning or history)
  struct Memory {
    //Current resource counts (used for build affordability checks)
    size_t wood  = 0;
    size_t stone = 0;
    size_t wheat = 0;
    size_t steel = 0; // typically produced by quarries and received via Notify

    //Last action outcome (used for persistence and short-term feedback)
    std::string last_action = "";
    bool        last_succeeded = true;

    //Directional failure counts (used to avoid repeatedly bad moves)
    int fail_up    = 0;
    int fail_down  = 0;
    int fail_left  = 0;
    int fail_right = 0;
  } mem;

  //Current short-term objective, re-evaluated every turn from tendencies + world state
  Goal current_goal = Goal::None;

  //Core pipeline for decision-making: selects a goal and scores actions each turn
  void   ChooseGoal();

  //Computes a total score for an action based on tendencies, goal, and world state
  double ScoreAction(const std::string& action, const WorldGrid& grid) const;

  //Movement baseline: rewards general exploration when no strong target is present
  double ScoreExplore    (const std::string& action, const WorldGrid& grid) const;

  //Rewards actions that move toward or collect the current goal resource
  double ScoreCollect    (const std::string& action, const WorldGrid& grid) const;

  //Scores build actions, only allowing the current goal building when affordable and valid
  double ScoreBuild      (const std::string& action, const WorldGrid& grid) const;

  //Slightly rewards repeating the last successful action (short-term consistency)
  double ScorePersistence(const std::string& action) const;

  //Penalizes actions/directions that have recently failed (avoids bad moves)
  double ScoreAvoidance  (const std::string& action) const;

  //Rewards engaging or moving toward enemies based on combat tendency
  double ScoreCombat     (const std::string& action, const WorldGrid& grid) const;

  //Rewards staying safe and avoiding dangerous positions based on survival tendency
  double ScoreSurvival   (const std::string& action, const WorldGrid& grid) const;

  //Rewards moving toward or staying near other agents based on social tendency
  double ScoreSocial     (const std::string& action, const WorldGrid& grid) const;

  //Checks if the agent has enough resources in memory to build the given structure
  bool CanAfford(Goal g) const;

  //Maps the current build goal to its corresponding action string (e.g., "build_quarry")
  std::string GoalBuildAction() const;

  //Maps the current collect goal to its corresponding resource cell type (e.g., "tree")
  std::string GoalResourceCell() const;


  //Checks if a given cell matches the current goal resource type
  bool IsGoalCell (size_t cell_id, const WorldGrid& grid) const;

  //Returns true if the agent is currently standing on the goal resource
  bool OnGoalResource (const WorldGrid& grid) const;

  //Returns true if taking the action would move onto the goal resource
  bool StepsOntoGoal (const std::string& action, const WorldGrid& grid) const;

  //Returns true if the agent is standing on a grass tile (required for building)
  bool OnGrass (const WorldGrid& grid) const;

  //Returns true if the action is a movement action
  bool IsMoveAction(const std::string& action) const;

  //Checks whether a movement action leads to a valid grid position
  bool IsMoveValid (const std::string& action, const WorldGrid& grid) const;

  //Computes the next world position resulting from a movement action
  WorldPosition NextPos (const std::string& action) const;

  //Returns how many times a movement direction has failed (used for avoidance scoring)
  int FailCount(const std::string& action) const;

  virtual bool SupportsAction(const std::string& name) const;
  
  virtual size_t LookupActionID(const std::string& name) const;


};

} // namespace cse498
