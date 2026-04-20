# Classic Agents Module

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

## 6. Summary

This module provides:
- A crash-resistant, environment-specific framework
- Autonomous, Behavior Tree-driven decision making
- Advanced navigation via dynamic obstacle mapping
