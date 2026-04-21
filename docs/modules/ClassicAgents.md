# Classic Agents -- Dynamic World

**Documented By:** Dillan Kowalski
**Developed By:** Dillan Kowalski, Joshua Thomas, Matthew Vazquez, and Bryent Shepherd

---

## 0. Introduction

This module implements an **autonomous, Behavior Tree driven system** capable of navigating, surviving, and completing complex objectives within simulation grids.

The system utilizes an Object-Oriented architecture:
- **Base `ClassicAgent`** providing core movement, sensing, and pathfinding (A*) mechanics.
- **Specialized child agents** (e.g., `ClassicDynamicAgent`) that override the base logic with advanced, world-specific behavior trees.

---

## 1. Main Structure

### Objective

The primary goal of the agent is to process environmental data and execute the optimal action every tick to fulfill world specific win conditions (such as building a Town Hall or surviving combat).

**Core Mechanics:**
- **Blackboard Memory:** Agents store active knowledge (resource counts, nearby enemies, active targets) in a shared memory structure.
- **Dynamic Pathfinding:** Agents generate an internal `WorldView` every tick to map discovered obstacles and use A* to navigate around walls.

---

The agent module operates on a **World-Driven Modular Architecture**:
- Each specific simulation world explicitly calls the specialized agent types it requires (e.g., the dynamic world instantiates `ClassicDynamicAgent`, while the other world calls `HeavyInteractionAgent`).
- At initialization, the agent builds its unique **Behavior Tree**.
- The Behavior Tree hierarchy strictly determines priorities based on the first branch, then second branch then so on.

---

### Agent Behavior

Each simulation tick, the agent executes its game loop:
1. **Reset Memory:** Clears stale actions to prevent freezing (`remain_still`).
2. **Sense Environment:** Scans a local radius to update known tiles, resources, and entities on the Blackboard.
3. **Think (Tree Update):** Traverses the Behavior Tree to evaluate conditions and select an action.
4. **Execute:** Translates the tree's decision into an engine-readable command.

#### Behavior Tree Branches (Dynamic Agent Example)
- **Win Condition:** Checks for 500+ of all resources to build Town Hall.
- **Expansion:** Builds spawners if population scaling is needed.
- **Economy:** Evaluates resource deficits to build Quarries, Lumberyards, or Farms.
- **Logistics:** Actively seeks grass tiles for building placement.
- **Gathering:** Uses a 15-tile radar to hunt and pathfind to the nearest raw materials.

---

## 2. Available Actions

### Movement
- `move_up`
- `move_down`
- `move_left`
- `move_right`

### Interaction & Economy
- `collect` (Harvests wood, stone, wheat)
- `build_quarry`
- `build_lumberyard`
- `build_farm`
- `build_spawner`
- `build_townhall`

### Failsafe
- `remain_still` (Triggers if pathfinding fails or the agent is completely blocked, preventing engine crashes).

---

## 3. Sensory Requirements (Inputs)

To function correctly, the agent relies on the World Module providing:

- **Grid Contents:** - Traversability (Walls vs. Walkable)
  - Tile Types (Grass, Stone, Trees)
- **Entities:** - Visibility of other agents and resources.
- **Global Inventory:** - Access to current counts of Wood, Stone, Steel, and Wheat to trigger building conditions.

---

## 4. Database Requirements

The agent system operates primarily in runtime memory, but for state persistence it requires:

- Saving the agent's current coordinates.
- Saving the agent's ID and specific Class Type (so the correct Behavior Tree is rebuilt on load).

---

## 5. Public API

```cpp
size_t SelectAction(WorldGrid & grid) override;
```

## 6. Summary

This module provides:

- A crash-resistant, environment-specific framework
- Autonomous, Behavior Tree-driven decision making
- Advanced navigation via dynamic obstacle mapping

---

# Classic Agents -- Heavy Interaction World

**Documented By:** Joshua Thomas

**Developed By:** Dillan Kowalski, Joshua Thomas, Matthew Vazquez, and Bryent Shepherd

---

## 7. Introduction

This module implements the autonomous agent systems used within the **Heavy Interaction World** simulation. Unlike the behavior-tree-driven Classic Agents, these agents use simpler, state-driven decision loops optimized for direct player interaction. This is done either through combat or conditional blocking.

The two agents are:

- **`EnemyAgent`** — a patrol-and-chase combatant that pursues and attacks the player on detection.
- **`GoblinAgent`** — a stationary blocking agent that acts as a conditional obstacle, polled by the world for interaction events.

---

## 8. Main Structure

### Objective

Each agent processes environmental data every tick and executes a single action to fulfill its role within the world. It is either enforcing combat pressure (`EnemyAgent`) or gating player progression (`GoblinAgent`).

### Architecture

Both agents follow a **sense → decide → act** loop each tick:

1. **Sense:** Scans known agents to locate the player by name substring match.
2. **Decide:** Evaluates proximity and state flags to select a behavior.
3. **Act:** Returns an action ID to the engine.

There is no shared blackboard or behavior tree. Each agent's logic is self-contained within its `Sense()` and `SelectAction()` methods.

---

## 9. EnemyAgent

### Overview

`EnemyAgent` patrols on a fixed axis and chases the player when detected within a configurable vision radius, using A* pathfinding to navigate obstacles. If the player is adjacent, it attacks.

### Behavior Loop

1. Scans for the player by `target_name` substring.
2. If **adjacent** → attacks.
3. If **in vision** → pathfinds and chases.
4. If **out of vision** → resumes patrol, bouncing direction on unwalkable cells.

### Available Actions

- `up`, `down`, `left`, `right` — required; `Initialize()` fails without all four.
- `attack` — optional; silently skipped if absent.

### Public API

```cpp
EnemyAgent &SetHorizontal();                       // Patrol along X axis
EnemyAgent &SetVertical();                         // Patrol along Y axis
EnemyAgent &ToggleDirection();                     // Reverse patrol direction
EnemyAgent &SetVisionRadius(size_t r);             // Set detection range (Manhattan)
EnemyAgent &SetTargetName(const std::string &name);
int  GetHP() const;                                // Get current HP
int  GetMaxHP() const;                             // Get max HP
void SetHP(int value);                             // Set current HP (clamped >= 0)
void SetMaxHP(int value);                          // Set max HP (clamped >= 0)
void TakeDamage(int amount);                       // Apply damage
bool IsAlive() const;                              // True if HP > 0
bool Initialize() override;
size_t SelectAction(WorldGrid &grid) override;
```

---

## 10. GoblinAgent

### Overview

`GoblinAgent` is a stationary blocking agent. It never moves or attacks. The world polls its state flags each tick to determine whether a player interaction event should trigger.

### Behavior Loop

1. Scans for the player by `target_name` substring.
2. Updates `player_adjacent` if the player is within Manhattan distance 1.
3. Always returns action `0` — the goblin never acts on its own.

### Public API

```cpp
GoblinAgent &SetBlocking(bool value);              // Enable/disable blocking
GoblinAgent &ClearBlocking();                      // Disable blocking
GoblinAgent &SetTargetName(const std::string &name);
bool IsBlocking() const;                           // True if currently blocking
bool IsPlayerAdjacent() const;                     // True if player is adjacent
bool CanBePaid() const;                            // True if blocking && player adjacent
bool Initialize() override;
size_t SelectAction(WorldGrid &grid) override;     // Always returns 0
```

### Integration Note

`CanBePaid()` is the primary hook for world logic. Poll it each tick and handle unblocking externally. The goblin has no concept of payment or progression itself.

---

## 11. Shared Sensory Requirements

Both agents rely on the World Module providing:

- **Agent Visibility:** `world.GetKnownAgents()` to detect the player.
- **Grid Contents:** Cell traversability (`wall`, `blocked`, `goblin_block` vs. walkable) for patrol and pathfinding.

---

## 12. Database Requirements

For state persistence:

- Save each agent's current coordinates, ID, and class type.
- On load, restore configuration (vision radius, patrol axis, target name, blocking state) so behavior is identical to before the save.

---

## 13. Summary

This module provides:

- A lightweight, crash-resistant agent framework for the Heavy Interaction World.
- Combat pressure via `EnemyAgent`'s patrol-chase-attack state machine.
- Progression gating via `GoblinAgent`'s proximity-based blocking and world-polled interaction flags.
