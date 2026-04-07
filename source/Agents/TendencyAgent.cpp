#include "TendencyAgent.hpp"

/**
 * TendencyAgent pipeline overview:
 *
 *   Initialize        - Verifies the world supports the minimum required actions for the agent to function.
 *   Notify            - Receives world feedback (failures, resources) and updates internal memory.
 *   SelectAction      - Main loop: chooses a goal, scores all actions, and returns the best one.
 *
 *   ChooseGoal        - Converts tendencies into a single current objective (intent selection).
 *   ScoreAction       - Evaluates each action and selects the best based on weighted component scores.
 *
 *   ScoreExplore      - Provides baseline movement incentive so the agent keeps exploring.
 *   ScoreCollect      - Rewards movement that advances the current resource goal.
 *   ScoreBuild        - Strongly rewards valid build actions matching the current goal.
 *   ScorePersistence  - Encourages repeating recently successful actions.
 *   ScoreAvoidance    - Penalizes directions that have failed before.
 *   ScoreSocial       - Rewards proximity to other agents (group behavior).
 *   ScoreCombat       - Rewards engaging or moving toward enemies.
 *   ScoreSurvival     - Rewards safer positions and avoidance of danger.
 *
 *   CanAfford         - Checks if a build goal is currently feasible given stored resources.
 *   GoalBuildAction   - Maps a build goal to the corresponding world action string.
 *   GoalResourceCell  - Maps a collect goal to the corresponding grid cell type.
 *
 *   IsGoalCell        - Checks if a grid cell matches the current resource goal.
 *   OnGoalResource    - Checks if the agent is currently on its target resource.
 *   StepsOntoGoal     - Checks if an action moves the agent onto its target resource.
 *   OnGrass           - Ensures building is only attempted on valid terrain.
 *
 *   IsMoveAction      - Identifies movement actions for routing scoring logic.
 *   IsMoveValid       - Rejects invalid movement actions.
 *   NextPos           - Computes the resulting position of a movement action.
 *   FailCount         - Returns failure history for a direction (used in avoidance).
 */

namespace cse498 {

// Verifies that the required core actions (movement and collect) are available before the agent can operate
bool TendencyAgent::Initialize() {
  return SupportsAction("up") && SupportsAction("down")
    && SupportsAction("left") && SupportsAction("right")
    && SupportsAction("collect");
}
/**
 * Receives feedback and events from the world and updates the agent’s memory.
 *
 * Supported messages from DynamicWorld:
 *   - "action_failed" + <direction>
 *       → marks last action as failed and increments failure count for that direction
 *
 *   - "resource" + "wood" | "stone" | "wheat" | "steel"
 *       → increments the agent’s stored resource counts
 *
 * Any unrecognized message types are ignored, allowing compatibility with
 * worlds that send additional signals (e.g., "damage", "enemy") without breaking.
 */
void TendencyAgent::Notify(const std::string& message,
                           const std::string& msg_type) {
  if (msg_type == "action_failed") {
    mem.last_succeeded = false;
    if (message == "up")    mem.fail_up++;
    else if (message == "down")  mem.fail_down++;
    else if (message == "left")  mem.fail_left++;
    else if (message == "right") mem.fail_right++;
  }
  else if (msg_type == "resource") {
    if      (message == "wood")  mem.wood++;
    else if (message == "stone") mem.stone++;
    else if (message == "wheat") mem.wheat++;
    else if (message == "steel") mem.steel++;
  }
}

/**
 * Selects the best action each turn using a score-based decision pipeline.
 *
 * Process:
 *   1. ChooseGoal() determines the agent’s current objective based on tendencies.
 *   2. All available actions (from DynamicWorld) are evaluated using ScoreAction().
 *   3. The highest-scoring action is selected.
 *   4. The chosen action is stored in memory for persistence/avoidance feedback.
 *
 * The agent adapts to the world by only considering actions it supports
 * (via HasAction), making it flexible across different environments.
 */
size_t TendencyAgent::SelectAction(const WorldGrid& grid) {
  // 1. Decide what we want to accomplish this turn.
  ChooseGoal();

  // 2. Score every available action and pick the best.
  std::string best_action    = "up";
  double      best_score     = -std::numeric_limits<double>::infinity();

  // Candidate action list — build actions only considered if available.
  static const std::vector<std::string> kAllActions = {
    "up","down","left","right",
    "up_left","up_right","down_left","down_right",
    "collect",
    "build_lumberyard","build_quarry","build_farm",
    "build_spawner","build_townhall"
  };

  for (const std::string& action : kAllActions) {
    if (!SupportsAction(action)) continue;
    const double s = ScoreAction(action, grid);
    if (s > best_score) {
      best_score  = s;
      best_action = action;
    }
  }

  // 3. Remember the choice for next turn's persistence/avoidance scoring.
  //    Reset succeeded flag optimistically; Notify will correct it if needed.
  mem.last_action    = best_action;
  mem.last_succeeded = true;

  // 4. Optimistically track resources when we choose to collect.
  //    The world will send a "resource" Notify on the *next* tick if the
  //    action actually succeeds, so this optimistic bump isn't needed unless
  //    the framework doesn't use Notify — keep whichever suits your world.
  // (Removed to avoid double-counting when Notify is active.)

  return LookupActionID(best_action);
}

/**
 * Chooses the agent's current high-level goal for this turn.
 *
 * Full decision pipeline:
 *   1. This function does not directly choose an action like "up" or "collect".
 *      Instead, it first chooses an intention such as:
 *        - CollectWood
 *        - CollectStone
 *        - CollectWheat
 *        - BuildLumberyard
 *        - BuildQuarry
 *        - BuildFarm
 *        - BuildSpawner
 *        - BuildTownhall
 *
 *   2. The chosen goal is based on weighted preference scores derived from the
 *      agent's Tendencies struct. In other words, this function converts the
 *      agent's personality values into one short-term objective.
 *
 *   3. Build goals and collect goals are placed into the same competition:
 *
 *        build score   = tendencies.build   * <specific building desire>
 *        collect score = tendencies.collect * <specific resource affinity>
 *
 *      The single highest-scoring eligible goal becomes current_goal.
 *
 * Eligibility rules:
 *   - A build goal is only considered if:
 *       a) the agent can currently afford that structure, and
 *       b) the matching build action exists in the world
 *          (for example, HasAction("build_quarry")).
 *
 *   - A collect goal is always considered, because gathering resources is the
 *     fallback way for the agent to make progress when building is not yet possible.
 *
 * Townhall override:
 *   - BuildTownhall is treated as a hard priority. If the agent can afford it
 *     and the world supports the build_townhall action, this function selects
 *     that goal immediately and returns without comparing anything else.
 *   - This makes the townhall the top strategic priority in the current design.
 *
 * How Tendencies affect this function:
 *   - tendencies.build:
 *       Raises or lowers the importance of all build goals at once.
 *       A higher build value makes the agent more likely to prefer constructing
 *       something over gathering resources, whenever construction is available.
 *
 *   - tendencies.collect:
 *       Raises or lowers the importance of all collect goals at once.
 *       A higher collect value makes the agent more likely to prefer resource
 *       gathering over building.
 *
 *   - tendencies.lumberyard_desire / quarry_desire / farm_desire /
 *     spawner_desire / townhall_desire:
 *       These determine which specific building is preferred once building is
 *       already valued by tendencies.build.
 *
 *   - tendencies.wood_affinity / stone_affinity / wheat_affinity:
 *       These determine which specific resource is preferred once collecting is
 *       already valued by tendencies.collect.
 *
 * What this function does NOT do:
 *   - It does not decide movement direction.
 *   - It does not check whether the target resource is nearby.
 *   - It does not evaluate the best immediate action.
 *   - It does not use persistence, avoidance, social, combat, or survival directly
 *     in the current implementation.
 *
 * Those are handled later, mainly during action scoring.
 *
 * Connection to the rest of the agent pipeline:
 *   - ChooseGoal() answers: "What do I want right now?"
 *   - ScoreAction() answers: "Given that goal, what exact action should I take?"
 *
 * So the full flow is:
 *   Tendencies -> ChooseGoal() -> current_goal -> ScoreAction() -> selected action
 *
 * How this can be extended using other tendencies:
 *   - persistence and avoidance currently influence action scoring, not goal choice.
 *     They shape how the agent pursues a goal, not which goal it wants.
 *
 *   - social could be integrated here by adding new social-style goals, such as:
 *       FollowAlly, GroupUp, DefendAlly
 *     and scoring them using tendencies.social.
 *
 *   - combat could be integrated here by adding combat goals, such as:
 *       AttackEnemy, ChaseEnemy, HoldPosition
 *     and scoring them using tendencies.combat.
 *
 *   - survival could be integrated here by adding safety goals, such as:
 *       FleeDanger, Retreat, RepositionToSafety
 *     and scoring them using tendencies.survival.
 *
 * In that extended design, this function would become a broader intention selector
 * across economic, social, combat, and defensive goals, while ScoreAction() would
 * still remain responsible for turning the winning goal into a concrete move.
 *
 * How available actions affect goal selection:
 *   - This function only selects build goals that the world actually supports.
 *     For example, even if quarry_desire is high, BuildQuarry is ignored unless
 *     HasAction("build_quarry") is true.
 *
 *   - This makes the agent robust across different DynamicWorld configurations:
 *     unsupported behaviors are automatically excluded from consideration.
 *
 *
 *   This function is the intention layer of the agent. It compares all eligible
 *   build and collect objectives using weighted tendency scores, selects the
 *   strongest one, and stores it in current_goal so later parts of the pipeline
 *   can decide how to carry it out.
 */
void TendencyAgent::ChooseGoal() {
  // Hard override: build the townhall whenever possible.
  if (CanAfford(Goal::BuildTownhall) && SupportsAction("build_townhall")) {
    current_goal = Goal::BuildTownhall;
    return;
  }

  double best_score = -1.0;
  Goal   best_goal  = Goal::None;

  // Helper: update best if this candidate beats the current winner.
  auto consider = [&](Goal g, double score) {
    if (score > best_score) { best_score = score; best_goal = g; }
  };

  // --- Build candidates (only when affordable) ---
  if (CanAfford(Goal::BuildLumberyard) && SupportsAction("build_lumberyard"))
    consider(Goal::BuildLumberyard, tendencies.build * tendencies.lumberyard_desire);

  if (CanAfford(Goal::BuildQuarry) && SupportsAction("build_quarry"))
    consider(Goal::BuildQuarry,     tendencies.build * tendencies.quarry_desire);

  if (CanAfford(Goal::BuildFarm) && SupportsAction("build_farm"))
    consider(Goal::BuildFarm,       tendencies.build * tendencies.farm_desire);

  if (CanAfford(Goal::BuildSpawner) && SupportsAction("build_spawner"))
    consider(Goal::BuildSpawner,    tendencies.build * tendencies.spawner_desire);

  // --- Collect candidates (always eligible) ---
  consider(Goal::CollectWood,  tendencies.collect * tendencies.wood_affinity);
  consider(Goal::CollectStone, tendencies.collect * tendencies.stone_affinity);
  consider(Goal::CollectWheat, tendencies.collect * tendencies.wheat_affinity);

  current_goal = best_goal;
}

/**
 * Scores one candidate action for the current turn.
 *
 * This function is the action-level part of the agent pipeline. After
 * ChooseGoal() selects the agent's current intention, ScoreAction() evaluates
 * each available action and assigns it a numeric score. The action with the
 * highest score is chosen by SelectAction().
 *
 * Action categories:
 *   - Movement actions are scored by combining multiple tendency-driven
 *     component scorers.
 *   - The collect action is only rewarded when the agent is standing on the
 *     resource tile that matches the current goal.
 *   - Build actions are delegated to ScoreBuild(), which only rewards the
 *     build action matching the current build goal when it is valid and affordable.
 *
 * Hard rejections:
 *   - Invalid movement actions receive a very large negative score so they are
 *     never selected.
 *   - Collect receives a large penalty if the agent is not currently on the
 *     correct goal resource.
 *
 * Movement scoring:
 *   - Movement actions are scored as a weighted sum of behavior components.
 *   - Each tendency controls how much its corresponding scorer matters:
 *       explore     -> general roaming / movement pressure
 *       collect     -> movement toward the current resource goal
 *       persistence -> repeating recently successful actions
 *       avoidance   -> avoiding directions that have recently failed
 *       social      -> moving toward or staying near other agents
 *       combat      -> moving toward or engaging enemies
 *       survival    -> preferring safer positions and avoiding danger
 *
 * In short:
 *   ChooseGoal() decides what the agent wants,
 *   ScoreAction() decides which exact action best serves that goal.
 */
double TendencyAgent::ScoreAction(const std::string& action,
                                  const WorldGrid&   grid) const {
  const bool is_move    = IsMoveAction(action);
  const bool is_collect = (action == "collect");
  const bool is_build   = !is_move && !is_collect;

  // --- Hard rejections ---
  if (is_move && !IsMoveValid(action, grid)) return -1000.0;

  // Collect is only valid when standing on the target resource.
  if (is_collect) {
    return OnGoalResource(grid) ? 500.0 : -500.0;
  }

  // Build actions are handled entirely by ScoreBuild.
  if (is_build) {
    return ScoreBuild(action, grid);
  }

  // --- Movement: weighted sum of component scores ---
  double score = 0.0;
  score += tendencies.explore     * ScoreExplore(action, grid);
  score += tendencies.collect     * ScoreCollect(action, grid);
  score += tendencies.persistence * ScorePersistence(action);
  score += tendencies.avoidance   * ScoreAvoidance(action);
  score += tendencies.social      * ScoreSocial(action, grid);
  score += tendencies.combat      * ScoreCombat(action, grid);
  score += tendencies.survival    * ScoreSurvival(action, grid);
  return score;
}

// Returns a base exploration score for movement actions, encouraging the agent to keep moving
// even when no immediate goal is nearby; this provides a baseline that other scorers refine.
double TendencyAgent::ScoreExplore(const std::string& action,
                                   const WorldGrid& /*grid*/) const {
  if (!IsMoveAction(action)) return 0.0;
  return 1.0;
}

// Rewards movement that directly advances the current resource goal by stepping onto the
// correct tile; this is the main driver of goal-directed movement toward resources.
double TendencyAgent::ScoreCollect(const std::string& action,
                                   const WorldGrid&   grid) const {
  if (!IsMoveAction(action)) return 0.0;

  const WorldPosition cur = GetLocation().AsWorldPosition();
  int best_dist = INT_MAX;
  int best_x = -1, best_y = -1;
  const int scan_radius = 20;

  for (int dy = -scan_radius; dy <= scan_radius; ++dy) {
    for (int dx = -scan_radius; dx <= scan_radius; ++dx) {
      int nx = static_cast<int>(cur.X()) + dx;
      int ny = static_cast<int>(cur.Y()) + dy;
      WorldPosition candidate(nx, ny);
      if (!grid.IsValid(candidate)) continue;
      if (!IsGoalCell(grid[candidate], grid)) continue;
      int dist = std::abs(dx) + std::abs(dy);
      if (dist < best_dist) {
        best_dist = dist;
        best_x = nx;
        best_y = ny;
      }
    }
  }

  if (best_dist == INT_MAX) return 0.0;

  WorldPosition next = NextPos(action);
  int cur_dist  = std::abs(static_cast<int>(cur.X())  - best_x)
                + std::abs(static_cast<int>(cur.Y())  - best_y);
  int next_dist = std::abs(static_cast<int>(next.X()) - best_x)
                + std::abs(static_cast<int>(next.Y()) - best_y);

  return static_cast<double>(cur_dist - next_dist) * 5.0;
} 

// Strongly rewards the exact build action matching the current goal when all conditions
// are valid (correct goal, correct tile, affordable); otherwise heavily penalizes it.
double TendencyAgent::ScoreBuild(const std::string& action,
                                 const WorldGrid&   grid) const {
  const bool is_build_goal =
    current_goal == Goal::BuildLumberyard ||
    current_goal == Goal::BuildQuarry     ||
    current_goal == Goal::BuildFarm       ||
    current_goal == Goal::BuildSpawner    ||
    current_goal == Goal::BuildTownhall;

  if (!is_build_goal)                     return -500.0;
  if (action != GoalBuildAction())        return -500.0;
  if (!OnGrass(grid))                     return -500.0;
  if (!CanAfford(current_goal))           return -500.0;

  return 1000.0;
}

// Slightly rewards repeating the last successful action, providing short-term stability
// and preventing jittery behavior when a direction or action is working.
double TendencyAgent::ScorePersistence(const std::string& action) const {
  return (action == mem.last_action && mem.last_succeeded) ? 2.0 : 0.0;
}

// Penalizes directions that have failed in the past, pushing the agent away from repeatedly
// making bad moves and encouraging exploration of alternative directions.
double TendencyAgent::ScoreAvoidance(const std::string& action) const {
  return -0.75 * static_cast<double>(FailCount(action));
}

// Rewards actions that move toward or stay near other agents, enabling group behavior
// or clustering when social tendency is high.
double TendencyAgent::ScoreSocial(const std::string& action,
                                  const WorldGrid&   grid) const {
  if (!IsMoveAction(action)) return 0.0;
  // Example placeholder: replace with actual neighbor/agent proximity logic
  return 0.0;
}

// Rewards actions that move toward or engage enemies, encouraging aggressive or
// confrontational behavior when combat tendency is high.
double TendencyAgent::ScoreCombat(const std::string& action,
                                  const WorldGrid&   grid) const {
  if (!IsMoveAction(action)) return 0.0;
  // Example placeholder: replace with enemy detection / targeting logic
  return 0.0;
}

// Rewards safer actions and penalizes risky positions, helping the agent avoid danger
// and prefer survival when the survival tendency is high.
double TendencyAgent::ScoreSurvival(const std::string& action,
                                    const WorldGrid&   grid) const {
  if (!IsMoveAction(action)) return 0.0;
  // Example placeholder: replace with danger/health/risk evaluation
  return 0.0;
}

/**
 * Checks whether the agent has enough stored resources in memory to execute
 * a given build goal, using the same cost rules as DynamicWorld.
 *
 * Role in pipeline:
 *   - Used during ChooseGoal() to filter out build goals that are not currently possible.
 *   - Used during ScoreBuild() to ensure the agent only attempts valid constructions.
 *   - Acts as the "feasibility check" layer between desire (tendencies) and execution.
 */
bool TendencyAgent::CanAfford(Goal g) const {
  switch (g) {
    case Goal::BuildLumberyard: return mem.wood  >= 20 && mem.steel >= 20;
    case Goal::BuildQuarry:     return mem.stone >= 20 && mem.wood  >= 20;
    case Goal::BuildFarm:       return mem.wheat >= 20 && mem.wood  >= 20;
    case Goal::BuildSpawner:    return mem.stone >= 30 && mem.wheat >= 30;
    case Goal::BuildTownhall:   return mem.wood  >= 500 && mem.stone >= 500
                                    && mem.steel >= 500 && mem.wheat >= 500;
    default: return false;
  }
}

/**
 * Converts the current build goal into the exact action string expected by
 * DynamicWorld (e.g., BuildQuarry → "build_quarry").
 *
 * Role in pipeline:
 *   - Bridges the abstract Goal enum to concrete world actions.
 *   - Used in ScoreBuild() to ensure only the correct build action is rewarded.
 *   - Keeps goal selection (what to do) separate from execution (how to do it).
 */
std::string TendencyAgent::GoalBuildAction() const {
  switch (current_goal) {
    case Goal::BuildLumberyard: return "build_lumberyard";
    case Goal::BuildQuarry:     return "build_quarry";
    case Goal::BuildFarm:       return "build_farm";
    case Goal::BuildSpawner:    return "build_spawner";
    case Goal::BuildTownhall:   return "build_townhall";
    default:                    return "";
  }
}

/**
 * Converts the current collect goal into the corresponding WorldGrid cell type
 * (e.g., CollectWood → "tree").
 *
 * Role in pipeline:
 *   - Allows the agent to recognize which tiles in the grid satisfy its goal.
 *   - Used by helper functions like OnGoalResource() and StepsOntoGoal().
 *   - Enables ScoreCollect() to reward movement toward the correct resource.
 */
std::string TendencyAgent::GoalResourceCell() const {
  switch (current_goal) {
    case Goal::CollectWood:  return "tree";
    case Goal::CollectStone: return "stone";
    case Goal::CollectWheat: return "wheat";
    default:                 return "";
  }
}
/**
 * Checks whether a given cell matches the resource type required by the current goal.
 *
 * Role in pipeline:
 *   - Connects the abstract goal (e.g., CollectWood) to actual grid cell types.
 *   - Used by movement and collect logic to identify valid target tiles.
 */
bool TendencyAgent::IsGoalCell(size_t cell_id, const WorldGrid& grid) const {
  const std::string& target = GoalResourceCell();
  return !target.empty() && grid.GetCellTypeName(cell_id) == target;
}

/**
 * Returns true if the agent is currently standing on the resource tile
 * that satisfies its current collect goal.
 *
 * Role in pipeline:
 *   - Used in ScoreAction() to strongly reward the "collect" action when valid.
 *   - Determines when the agent has reached its resource objective.
 */
bool TendencyAgent::OnGoalResource(const WorldGrid& grid) const {
  const WorldPosition cur = GetLocation().AsWorldPosition();
  return grid.IsValid(cur) && IsGoalCell(grid[cur], grid);
}

/**
 * Returns true if taking the given movement action would place the agent
 * onto a tile that satisfies the current goal.
 *
 * Role in pipeline:
 *   - Used in ScoreCollect() to reward movement that advances the goal.
 *   - Guides directional choice toward target resources.
 */
bool TendencyAgent::StepsOntoGoal(const std::string& action,
                                  const WorldGrid&   grid) const {
  const WorldPosition next = NextPos(action);
  return grid.IsValid(next) && IsGoalCell(grid[next], grid);
}

/**
 * Checks whether the agent is currently standing on a grass tile.
 *
 * Role in pipeline:
 *   - Used in ScoreBuild() to ensure buildings are only placed on valid terrain.
 *   - Acts as an environmental constraint before allowing build actions.
 */
bool TendencyAgent::OnGrass(const WorldGrid& grid) const {
  const WorldPosition cur = GetLocation().AsWorldPosition();
  return grid.IsValid(cur) && grid.GetCellTypeName(grid[cur]) == "grass";
}
// Movement helpers used to interpret actions and evaluate valid next positions during scoring

/**
 * Returns true if the given action string represents a movement action.
 *
 * Role in pipeline:
 *   - Used to separate movement from collect/build actions in ScoreAction().
 *   - Ensures only movement actions are passed into movement-based scorers.
 */
bool TendencyAgent::IsMoveAction(const std::string& action) const {
  return action == "up"         || action == "down"
      || action == "left"       || action == "right"
      || action == "up_left"    || action == "up_right"
      || action == "down_left"  || action == "down_right";
}

/**
 * Checks whether taking a movement action results in a valid grid position.
 *
 * Role in pipeline:
 *   - Used for hard rejection in ScoreAction() to prevent invalid moves.
 *   - Ensures the agent never selects actions that go out of bounds.
 */
bool TendencyAgent::IsMoveValid(const std::string& action,
                                const WorldGrid&   grid) const {
  return grid.IsValid(NextPos(action));
}

/**
 * Computes the next world position that would result from taking the given action.
 *
 * Role in pipeline:
 *   - Core utility used by movement-related scorers (collect, avoidance, etc.).
 *   - Translates an abstract action string into a concrete grid position.
 */
WorldPosition TendencyAgent::NextPos(const std::string& action) const {
  const WorldPosition cur = GetLocation().AsWorldPosition();
  if (action == "up")         return cur.Up();
  if (action == "down")       return cur.Down();
  if (action == "left")       return cur.Left();
  if (action == "right")      return cur.Right();
  if (action == "up_left")    return cur.Up().Left();
  if (action == "up_right")   return cur.Up().Right();
  if (action == "down_left")  return cur.Down().Left();
  if (action == "down_right") return cur.Down().Right();
  return cur;
}

/**
 * Returns how many times a movement direction has previously failed.
 *
 * Role in pipeline:
 *   - Used by ScoreAvoidance() to penalize repeatedly failing directions.
 *   - Provides simple memory-driven feedback to guide future movement choices.
 */
int TendencyAgent::FailCount(const std::string& action) const {
  if (action == "up")    return mem.fail_up;
  if (action == "down")  return mem.fail_down;
  if (action == "left")  return mem.fail_left;
  if (action == "right") return mem.fail_right;
  return 0;
}

bool TendencyAgent::SupportsAction(const std::string& name) const {
  return HasAction(name);
}

size_t TendencyAgent::LookupActionID(const std::string& name) const {
  return GetActionID(name);
}

} // namespace cse498
