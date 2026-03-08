# Scheduler Explanation Script

---

## Overview

This script walks through the current state of the scheduler system — what it does, how it fits into the broader simulation framework, and how the pieces talk to each other.

---

## The Big Picture: How a Simulation Turn Works

At the top level, we have **WorldBase**. It owns everything: the grid, the agents, and the items. Its `Run()` method drives the entire simulation loop.

Each iteration of that loop does three things:

1. **RunAgents()** — asks every agent to pick an action
2. **DoAction()** — executes that action in the world
3. **UpdateWorld()** — handles any background bookkeeping

So the flow starts with the world, which delegates to agents, which return a decision, and the world carries it out. The scheduler sits *on top of this loop* and controls *which agent runs when, and how often*.

---

## The Scheduler Class

**File:** `source/tools/scheduling/Scheduler.hpp`

The core scheduler is **stride-based**. Every registered process gets a **priority** value and a derived **stride**:

```
stride = 1,000,000 / priority
```

Internally, each process tracks a **pass value** — a virtual timestamp. The scheduler always picks the process with the *lowest* pass value next, then advances that process's pass by its stride. Higher priority means a smaller stride, so that process's pass value climbs slowly, and it gets selected again sooner.

Think of it like runners on a track: faster runners (higher priority) take smaller steps backward after each lap, so they stay near the front.

**Key public methods:**
- `AddProcess(id, priority)` — register a process
- `GetNext()` — select the next process to run (and advance its virtual time)
- `UpdatePriority(id, new_priority)` — change priority on the fly
- `RemoveProcess(id)` — unregister a process
- `PeekNext()` — look at what's next without advancing

---

## The Tiered Scheduler

**File:** `source/tools/scheduling/TieredScheduler.hpp`

The **TieredScheduler** wraps four independent Scheduler instances, one per tier:

| Tier | Default Budget |
|------|---------------|
| CRITICAL | 40% of frame time |
| GAMEPLAY | 30% of frame time |
| ECONOMY | 20% of frame time |
| COSMETIC | 10% of frame time |

At the start of each frame, `ResetFrameBudgets(total_frame_time_ms)` fills each tier's time budget. Then `GetNext()` works down the priority order — CRITICAL first, then GAMEPLAY, then ECONOMY, then COSMETIC — and only pulls from a tier if it still has budget remaining.

CRITICAL has a soft limit (it can slightly exceed its budget before stopping). All other tiers have hard limits — they stop the moment their budget runs out.

After a process runs, `RecordExecutionTime(actual_ms)` subtracts the real execution time from the currently active tier's budget.

This gives you the scheduling semantics of: *"always handle critical work first, then gameplay, then lower-priority concerns, and stop when the frame budget is spent."*

---

## The Timed Event Queue

**File:** `source/tools/scheduling/TimedEventQueue.hpp`

Separate from the stride scheduler, the **TimedEventQueue** handles one-shot and time-triggered events. An event looks like:

```cpp
struct Event {
  size_t id;
  double time_scheduled;
  double priority;           // reserved for future use
  std::function<void()> callback_action;
};
```

Events are stored in a sorted set by `time_scheduled` (with ID as a deterministic tiebreaker). Calling `ProcessDue(current_time)` fires every event whose scheduled time is at or before `current_time` and returns a count of how many ran. Events can also be cancelled by ID in O(log n) time.

---

## How It All Fits Together

Here is the end-to-end flow as it stands today:

```
WorldBase::Run()
    │
    ├─► RunAgents()
    │       │
    │       └─► For each agent:
    │               agent.SelectAction(grid)   ← agent reads the grid, returns an action ID
    │               world.DoAction(agent, id)  ← world executes the action, updates state
    │               agent.SetActionResult(r)   ← world tells agent if it succeeded
    │
    ├─► UpdateWorld()                          ← background simulation logic
    │
    └─► [loop continues until run_over == true]


Scheduler layer (controls the above loop):

    TieredScheduler::GetNext()
        │
        ├─ checks CRITICAL budget → pulls from Scheduler<CRITICAL>
        ├─ checks GAMEPLAY budget → pulls from Scheduler<GAMEPLAY>
        ├─ checks ECONOMY budget  → pulls from Scheduler<ECONOMY>
        └─ checks COSMETIC budget → pulls from Scheduler<COSMETIC>

                Each inner Scheduler uses stride scheduling
                to pick which process within that tier runs next.


TimedEventQueue (parallel track):

    World (or external code) calls:
        queue.Schedule({ id, time, callback })

    Each simulation tick:
        queue.ProcessDue(current_time)
            └─► fires all callbacks whose time has come
```

So in plain terms:
- The **Agent** observes the world and picks an action.
- The **World** carries out that action and owns all state.
- The **Scheduler** controls the *order and frequency* with which processes (agents, world updates, etc.) get CPU time within a frame.
- The **TimedEventQueue** handles anything that needs to happen *at a specific simulation time* — one-shot callbacks, delayed effects, etc.

---

## Agent → World → Scheduler Example

1. An agent is registered with the TieredScheduler under the GAMEPLAY tier with a given priority.
2. Each frame, `TieredScheduler::GetNext()` returns that agent's ID when it's that agent's turn.
3. The world calls `agent.SelectAction(grid)` to get a decision.
4. The world calls `DoAction(agent, action_id)` to execute it.
5. If a delayed side-effect needs to happen later, a callback is pushed into the `TimedEventQueue` with the appropriate time.
6. On the next tick where `current_time >= scheduled_time`, `ProcessDue()` fires it.

---

## What's Coming Next (If Needed)

Two potential extensions are on the table, to be added only if benchmarking shows they're necessary:

### Temporal Coarsening

Instead of running every agent every tick, lower-priority agents could be batched and run every N ticks — e.g., COSMETIC-tier agents might only run every 5 frames. This reduces overhead without changing the visible outcome for unimportant processes.

### Dynamic Prioritization

Priorities could be adjusted at runtime based on game state — for example, boosting an agent's priority when it's close to a player, or reducing it when it's idle off-screen. The `UpdatePriority(id, new_priority)` hook already exists in the Scheduler for exactly this purpose.

Both features are designed to slot in without restructuring the existing interface.

---

## Summary

| Component | Role |
|-----------|------|
| `Scheduler` | Stride-based per-tier process ordering |
| `TieredScheduler` | Wraps 4 Schedulers with frame-time budgets |
| `TimedEventQueue` | Time-triggered one-shot event callbacks |
| `WorldBase` | Owns agents, grid, state; drives the game loop |
| `AgentBase` | Observes world, returns action decisions |
| `WorldGrid` | 2D spatial data accessed by agents and the world |

The scheduler is deliberately kept simple and composable right now. The stride algorithm guarantees proportional CPU allocation across processes, the tiered wrapper enforces hard priorities between categories of work, and the event queue handles anything time-dependent — all without coupling any of these to the game-specific logic in World or Agent.
