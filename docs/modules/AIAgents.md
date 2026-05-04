# AI Agents Module

**Documented By:** Shashank Papani

**Developed By:** Ahmed Ezaz Labib, Shashank Papani, David Kaczanowski, Sachin Karatha, Shamar Dotson

---

## 0. Introduction

This module implements **six concrete agent types** used across Company C simulations:

| Agent | Primary use |
|--------|-------------|
| **`DynamicAgent`** | DynamicWorld — role-based economy and phased building for the win condition |
| **`HunterAgent`** | Interaction-heavy worlds — roam / alert / chase pursuit |
| **`SmartAgent`** | Any grid with four cardinal moves — async LLM-driven movement (callback required) |
| **`SokobanAgent`** | SokobanWorld — BFS puzzle solver |
| **`TendencyAgent`** | DynamicWorld-style action sets — weighted personality scoring across all actions |
| **`MazeSolverAgent`** | MazeWorld — BFS pathfinding toward exit or unexplored floor |

All of them inherit **`AgentBase`** (`source/core/AgentBase.hpp`):

- The world registers **action names → ids**; **`0`** means “no action.”
- Each tick the world sets **`action_result`**, calls **`SelectAction(WorldGrid &)`**, and may send **`Notify(message, msg_type)`**.

The **Web Interface** module (Settings → Simulation) can surface **behavior settings** (for example, tendency weights, `DynamicAgent` roles, or external AI hooks). This document describes the **C++ policies** for the agents above, including **`web_main.cpp`** examples where useful.

---

## 1. Main Structure

### Objective

Provide **decision policies** that stay within each world’s **registered action set** while supporting different play styles: scripted economy (`DynamicAgent`), personality scoring (`TendencyAgent`), search (`SokobanAgent`, `MazeSolverAgent`), pursuit AI (`HunterAgent`), and external reasoning (`SmartAgent`).

---

### Core lifecycle

1. **`Initialize()`** — Agent checks that required action names exist (via **`HasAction`**).
2. **`SelectAction(grid)`** — Returns the next action id.
3. **`Notify(...)`** — Optional; used for resources, goals, target hints, failures, etc., depending on the agent.

---

### Base class (`AgentBase`)

- **`action_map`**, **`GetActionID`**, **`HasAction`**, **`SetActionResult`**
- **`SelectAction`** — implemented by each concrete agent
- **`Notify`** — overridden where agents need world feedback

---

## 2. Agent types

The following sections describe the six agents in this module. Source: `source/Agents/*.hpp` and `*.cpp`.

---

### 2.1 `DynamicAgent`

**World:** **DynamicWorld** (resource-building simulation; see **Dynamic World** module).

Purpose-built for the win condition: **five quarries**, **five farms**, **five lumberyards**, then **one town hall**, in that order. A **`SyncPool()`** call at the start of **`SelectAction`** reads **`world_global_counts`** so all agents share accurate pool totals for affordability checks. (Static pool fields are also maintained for compatibility with resource **`Notify`** messages.)

**Roles:**

| Role | Behavior (summary) |
|------|---------------------|
| **Leader** | Does not collect; moves onto **grass** and issues **`build_*`** when the pool can afford the current phase; advances after **5** structures in the quarry/farm/lumberyard phases or **1** town hall |
| **Collector** | Gathers whichever resource the pool has **least** of (toward thresholds) |
| **Farmer / Miner / Woodsman** | Collects **wheat / stone / wood** (`tree`) only |
| **Ghost** | **Moves only** — never collects or builds |

**Build phase sequence:**

| Phase | Structure | Cost (each) |
|------|-----------|-------------|
| 1 | Quarry × 5 | 20 wood + 20 stone |
| 2 | Farm × 5 | 20 wood + 20 wheat |
| 3 | Lumberyard × 5 | 20 wood + 20 steel |
| 4 | Town hall × 1 | 500 of each resource |

**Usage in `web_main.cpp`:**

```cpp
else if (run_mode == "dynamic")
{
    auto &world = g_app->Initialize<cse498::DynamicWorld>();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> x_pos(0, world.GetWidth() - 1);
    std::uniform_int_distribution<int> y_pos(0, world.GetHeight() - 1);

    world.AddAgent<cse498::DynamicAgent>("Builder")
        .SetLeader(true)
        .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});

    world.AddAgent<cse498::DynamicAgent>("Farmer")
        .SetFarmer()
        .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});

    world.AddAgent<cse498::DynamicAgent>("Miner")
        .SetMiner()
        .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});

    world.AddAgent<cse498::DynamicAgent>("Woodsman")
        .SetWoodsman()
        .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});

    world.AddAgent<cse498::DynamicAgent>("Collector")
        .SetCollector()
        .SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});

    g_auto_run = true;
}
```

---

### 2.2 `HunterAgent`

**World:** **Interaction-heavy** worlds (e.g. **`InteractionHeavyWorld`**).

An enemy **pursuit** agent. It does **not** apply damage — that is a **world** responsibility (for example contact damage or **`Notify`**, sometimes described as **`ApplyEnemyContactDamage()`** in design docs). The hunter only needs to become **adjacent** to the target.

**States:**

- **`Roam`** — wanders pseudo-randomly; holds a direction for several ticks to reduce jitter
- **`Alert`** — target just entered range; **one-tick** delay before **`Chase`**
- **`Chase`** — **Chebyshev-greedy** move toward the last known target; returns to roam if the target leaves **`mChaseRadius`**, with **`mChaseMemoryTicks`** of memory

**Targets:** Scans for cells named **`"player"`** or **`"agent"`** within **`mDetectRadius`**. **`TendencyAgent`**-driven entities should be registered as **`"agent"`** if they should be hunted, or push positions via **`Notify("x,y", "target_position")`**. Hunters are not typically named `"player"` / `"agent"` on the grid.

**Tuning:**

```cpp
world.AddAgent<cse498::HunterAgent>("Hunter")
    .SetDetectRadius(10)   // default 8
    .SetChaseRadius(15)    // default 12
    .SetChaseMemory(20)    // default 10 ticks
    .SetLocation(WorldPosition{5, 5});
```

**Actions:** **`up`**, **`down`**, **`left`**, **`right`**.

---

### 2.3 `SmartAgent`

**World:** Any grid world that exposes the four **cardinal** moves.

Designed to connect an **external LLM** (or other NPC backend) to a grid agent.

**Flow:**

- Builds a **prompt**: primary goal (**`Notify`**), **legal moves** with destination coordinates, ASCII **grid** with the agent highlighted.
- **`SetNpcRequestCallback`** supplies **`std::future<std::string>`** per request.
- Parses **`PLAN:`** / **`MOVE:`** lines (or falls back to scanning for **`up`/`down`/`left`/`right`**).
- Keeps up to **two** validated moves buffered and can **prefetch** the next move while the first executes.

**Integration:** Install **`SmartAgent::SetNpcRequestCallback`** before simulation ticks; use **`GetSystemPrompt()`** for model system instructions.

**Practicality:** Without a real async HTTP client or local model behind the callback, the agent cannot obtain replies. The current web demo does not ship that infrastructure — **`SmartAgent`** is **LLM-ready** but **not turnkey** until a callback is wired in.

```cpp
SmartAgent::SetNpcRequestCallback(myLlmCallback);
```

---

### 2.4 `SokobanAgent`

**World:** **SokobanWorld** only.

Solves Sokoban puzzles using **BFS** over the full combined state space (agent position + all boulder positions). Relies on cell type names such as **`floor`**, **`wall`**, **`boulder`**, **`button`**, **`pressed`**, **`exit`** — **not** valid in arbitrary worlds.

**Behavior:**

- When the **grid hash** changes (new level), runs BFS and stores the solution in a **deque**.
- Each **`SelectAction()`** pops one action.
- If the world **rejects** a move, the queue is cleared and the agent **re-solves** from the current state.

**Adding in `web_main.cpp`:**

```cpp
else if (run_mode == "sokoban")
{
    auto &world = g_app->Initialize<cse498::SokobanWorld>();
    world.AddAgent<cse498::SokobanAgent>("Solver")
        .SetLocation(WorldPosition{1, 1});
    g_auto_run = true;
}
```

---

### 2.5 `TendencyAgent`

**World:** Any world that exposes the same **action names** as **DynamicWorld** (eight directions, **`collect`**, **`build_*`**).

A **personality-driven** agent: each tick it scores **every** legal action and picks the highest scorer.

**Pipeline each tick:**

1. **`ChooseGoal()`** — collect vs build intention. Build goals must be affordable and registered. **`BuildTownhall`** is treated as a **hard priority** when affordable.
2. **`ScoreAction()`** — weighted sum of:
   - **`ScoreExplore`** — baseline movement so the agent does not idle
   - **`ScoreCollect`** — rewards progress toward the goal resource
   - **`ScoreBuild`** — rewards the matching affordable build action
   - **`ScorePersistence`** — rewards repeating recently successful actions
   - **`ScoreAvoidance`** — penalizes directions that have repeatedly failed
   - **`ScoreSocial`**, **`ScoreCombat`**, **`ScoreSurvival`** — **stubs** (return **0** until implemented)

**Design:** The **`Tendencies`** struct holds explore, collect, build, persistence, avoidance, social, combat, survival, resource **affinities**, and building **desires**. **`Goal`** is **recomputed** every tick (no fixed long-term plan). Fluent setters live in `TendencyAgent.hpp`.

**Configuration example:**

```cpp
world.AddAgent<cse498::TendencyAgent>("Explorer")
    .SetExplore(1.5)
    .SetCollect(1.5)
    .SetCombat(0.6)
    .SetStoneAffinity(1.2)
    .SetLocation(WorldPosition{3, 1});
```

**Memory:** Resource counts, last action success, per-direction failure counts — used for scoring and affordability only.

---

### 2.6 `MazeSolverAgent`

**World:** **MazeWorld**.

Navigates using **BFS**. Priority: **exit** cell first, then nearest **unvisited** floor; see `MazeSolverAgent.hpp` for fallbacks. Keeps a **visited** map to limit redundant exploration.

**Behavior:**

- **`PlanPath()`** runs BFS when the path is exhausted or invalidated.
- **Two** consecutive failed moves trigger an immediate **replan**.

**Implementation note:** Walkability uses **`GetCellTypeName(...) != "wall"`** rather than relying only on **`IsTraversable`**. On some grids **`IsTraversable`** can be false for floor-like cells, which would leave BFS with no reachable nodes; the explicit wall check avoids that.

**Adding in `web_main.cpp`:**

```cpp
#include "Agents/MazeSolverAgent.hpp"
// ...
world.AddAgent<cse498::MazeSolverAgent>("Solver")
    .SetLocation(WorldPosition{1, 1});
```

**Actions:** **`up`**, **`down`**, **`left`**, **`right`**.

---

## 3. Action requirements

| Agent | Typical required actions |
|--------|---------------------------|
| `DynamicAgent` | Eight directions, `collect` (+ `build_*` for leader) |
| `HunterAgent` | `up`, `down`, `left`, `right` |
| `SmartAgent` | `up`, `down`, `left`, `right` |
| `SokobanAgent` | `up`, `down`, `left`, `right` |
| `TendencyAgent` | Diagonals + cardinals, `collect`, `build_*` |
| `MazeSolverAgent` | `up`, `down`, `left`, `right` |

Exact strings must match what the **world** registers on the agent.

---

## 4. UI and integration notes

- **Settings** can serialize **tendency** parameters for **`TendencyAgent`**, roles for **`DynamicAgent`**, or hunter tuning for **`HunterAgent`**.
- **`SmartAgent`** needs a **host-provided callback** (local model, HTTP client, or test stub) wired before the simulation loop.
- **`web_main.cpp`** `run_mode` / URL parameters select the world and agents; see §2 for snippets.

---

## 5. Dependencies

- **`AgentBase`**, **`WorldGrid`**, **`WorldBase`**, **`WorldPosition`**, **`Location`** — `source/core/`
- C++ standard library; **`SmartAgent`** uses **`<future>`**, **`<chrono>`**, etc.

Build settings are integrated via the **repository Makefile** with the rest of the project.

---

## 6. Public API (representative)

**`AgentBase`:**

```cpp
[[nodiscard]] virtual size_t SelectAction(WorldGrid & grid) = 0;
virtual void Notify(const std::string & message,
                    const std::string & msg_type = "none");
```

**`SmartAgent`:**

```cpp
static void SetNpcRequestCallback(
    std::future<std::string> (*)(const SmartAgent &, const std::string &) callback);
[[nodiscard]] static std::string_view GetSystemPrompt() noexcept;
```

**`DynamicAgent` (examples):**

```cpp
DynamicAgent & SetLeader(bool v);
DynamicAgent & SetFarmer();
DynamicAgent & SetMiner();
DynamicAgent & SetWoodsman();
DynamicAgent & SetCollector();
DynamicAgent & SetGhost();
```

**`HunterAgent` (examples):**

```cpp
HunterAgent & SetDetectRadius(int v);
HunterAgent & SetChaseRadius(int v);
HunterAgent & SetChaseMemory(int v);
```

---

## 7. Summary

The **AI Agents** module delivers:

- **`DynamicAgent`** — phased **5+5+5+1** building, **`world_global_counts`**-synced pool, specialists, and **ghost** roam-only mode  
- **`HunterAgent`** — **roam / alert / chase**; damage remains a **world** concern  
- **`SmartAgent`** — prompt-based, async **LLM-ready** movement (callback required)  
- **`SokobanAgent`** — **BFS** over full Sokoban state; re-solve on failure  
- **`TendencyAgent`** — **weighted scoring** with **town-hall-first** goal when affordable  
- **`MazeSolverAgent`** — **BFS** maze navigation with exit / exploration priorities  

---

## 8. Source layout

- Implementations: **`source/Agents/`** — `DynamicAgent`, `HunterAgent`, `SmartAgent`, `SokobanAgent`, `TendencyAgent`, `MazeSolverAgent`  
- Tests (where present): **`tests/Agents/`**
