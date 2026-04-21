# AI Agents Module

**Documented By:** Shashank Papani

**Developed By:** Ahmed Ezaz Labib, Shashank Papani, David Kaczanowski, Sachin Karatha, Shamar Dotson

---

## 0. Introduction

This module implements **five concrete agent types** used across Company C simulations:

| Agent | Primary use |
|--------|-------------|
| **`DynamicAgent`** | Dynamic / resource-building worlds |
| **`HunterAgent`** | Interaction-heavy worlds (enemy pursuit) |
| **`SmartAgent`** | LLM / NPC-driven grid movement |
| **`SokobanAgent`** | Sokoban puzzle worlds |
| **`TendencyAgent`** | Dynamic World with weighted personality-driven scoring |

All of them inherit **`AgentBase`** (`source/core/AgentBase.hpp`):

- The world registers **action names → ids**; **`0`** means “no action.”
- Each tick the world sets **`action_result`**, calls **`SelectAction(WorldGrid &)`**, and may send **`Notify(message, msg_type)`**.

The **Web Interface** module (Settings → Simulation) can surface **behavior settings** (for example, tendency weights or external AI hooks); this document describes the **C++ policies** for the agents above.

---

## 1. Main Structure

### Objective

Provide **decision policies** that stay within each world’s **registered action set** while supporting different play styles: scripted economy (`DynamicAgent`), personality scoring (`TendencyAgent`), search (`SokobanAgent`), combat AI (`HunterAgent`), and external reasoning (`SmartAgent`).

---

### Core lifecycle

1. **`Initialize()`** — Agent checks that required action names exist (via **`HasAction`**).
2. **`SelectAction(grid)`** — Returns the next action id.
3. **`Notify(...)`** — Optional; used for resources, goals, target hints, etc., depending on the agent.

---

### Base class (`AgentBase`)

- **`action_map`**, **`GetActionID`**, **`HasAction`**, **`SetActionResult`**
- **`SelectAction`** — implemented by each agent below
- **`Notify`** — overridden where agents need world feedback

---

## 2. Agent types

The following sections describe the five agents in this module. Source: `source/Agents/*.hpp` and `*.cpp`.

---

### 2.1 `DynamicAgent`

**World:** Dynamic / resource-building simulation (see **Dynamic World** module).

**Roles** (via **`SetType`**, **`SetLeader`**, **`SetCollector`**, **`SetFarmer`**, **`SetMiner`**, **`SetWoodsman`**):

| Role | Behavior (summary) |
|------|---------------------|
| **Leader** | Does not collect; follows a **build phase** (quarry → farm → lumberyard → town hall) when the **shared resource pool** can afford each step |
| **Collector** | Gathers the **least-needed** resource type toward win thresholds |
| **Farmer / Miner / Woodsman** | Collects **wheat / stone / wood** only |

**Pooling:** Static **`pool_wood`**, **`pool_stone`**, **`pool_wheat`**, **`pool_steel`** are updated in **`Notify`** when `msg_type == "resource"` so the leader can judge team-wide affordability.

---

### 2.2 `HunterAgent`

**World:** **Interaction-heavy** worlds; contact / HP effects are applied by the world (for example **`ApplyEnemyContactDamage`**), not inside the hunter.

**States:** **`Roam`** → **`Alert`** (one-tick delay after detection) → **`Chase`** (Chebyshev-greedy pursuit). Returns to roam if the target leaves **`mChaseRadius`**, with **`mChaseMemoryTicks`** of memory.

**Tuning:** **`SetDetectRadius`**, **`SetChaseRadius`**, **`SetChaseMemory`**.

**Targets:** Scans for cells named **`player`** or **`agent`** within **`mDetectRadius`** (Chebyshev). Optional **`Notify("x,y", "target_position")`**.

**Actions:** **`up`**, **`down`**, **`left`**, **`right`**.

---

### 2.3 `SmartAgent`

**World:** Any grid world that exposes the four **cardinal** moves; designed for **async NPC / LLM** decisions.

**Flow:**

- Builds a **prompt**: primary goal (**`Notify`**), **legal moves** with destination coordinates, ASCII **grid** with the agent highlighted.
- **`SetNpcRequestCallback`** supplies **`std::future<std::string>`** per request.
- Parses **`PLAN:`** / **`MOVE:`** lines (or falls back to scanning for **`up`/`down`/`left`/`right`**).
- Keeps up to **two** validated moves buffered and can **prefetch** the next move while the first executes.

**Integration:** Install **`SmartAgent::SetNpcRequestCallback`** before simulation ticks; use **`GetSystemPrompt()`** for model system instructions.

---

### 2.4 `SokobanAgent`

**World:** **Sokoban-style** levels only (cell types such as floor, wall, boulder, button, pressed, exit — as named by that world).

**Algorithm:** When the **grid hash** changes (new level), runs **BFS** on the combined **player + boulders** state space, fills a **solution queue**, then returns one **precomputed** action per **`SelectAction`** call.

**Warning:** **Not** for general grid worlds; behavior assumes Sokoban push rules and cell naming.

---

### 2.5 `TendencyAgent`

**World:** Same **Dynamic World** action space: eight directions + **`collect`** + **`build_*`** (names must match the world).

**Design:** **`Tendencies`** — weights for explore, collect, build, persistence, avoidance, social, combat, survival, plus **resource affinities** and **building desires**. Each turn **`ScoreAction`** ranks **all** legal actions; the highest score wins. **`Goal`** is **recomputed** every tick (no fixed long-term plan).

**Configuration:** Fluent setters (**`SetExplore`**, **`SetCollect`**, **`SetWoodAffinity`**, **`SetTownhallDesire`**, etc.) in `TendencyAgent.hpp`.

**Memory:** Minimal — resource counts, last action success, per-direction failure counts — used in scoring only.

---

## 3. Action requirements

| Agent | Typical required actions |
|--------|---------------------------|
| `DynamicAgent` | `up`, `down`, `left`, `right`, `collect` (+ build action names for leader) |
| `HunterAgent` | `up`, `down`, `left`, `right` |
| `SmartAgent` | `up`, `down`, `left`, `right` |
| `SokobanAgent` | `up`, `down`, `left`, `right` |
| `TendencyAgent` | Dynamic World set: diagonals + cardinals, `collect`, `build_*` |

Exact strings must match what the **world** registers on the agent.

---

## 4. UI and integration notes

- **Settings** can serialize **tendency** parameters for **`TendencyAgent`** or roles for **`DynamicAgent`**.
- **`SmartAgent`** needs a **host-provided callback** (local model, HTTP client, or test stub) wired before the simulation loop.

---

## 5. Dependencies

- **`AgentBase`**, **`WorldGrid`**, **`WorldBase`**, **`WorldPosition`** — `source/core/`
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

- **`DynamicAgent`** — role-based economy and phased building with a shared resource pool  
- **`HunterAgent`** — roam / alert / chase pursuit for interaction-heavy worlds  
- **`SmartAgent`** — prompt-based, async **LLM-ready** movement on small grids  
- **`SokobanAgent`** — **BFS** optimal replay for Sokoban levels  
- **`TendencyAgent`** — **weighted scoring** over Dynamic World actions for configurable “personalities”  

---

## 8. Source layout

- Implementations: **`source/Agents/`** (`DynamicAgent`, `HunterAgent`, `SmartAgent`, `SokobanAgent`, `TendencyAgent`)
- Tests (where present): **`tests/Agents/`**
