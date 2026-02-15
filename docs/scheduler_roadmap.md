# Scheduler Implementation Roadmap

Implementation roadmap for the real-time process scheduling system for tile-based civilization game.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    Event Queue                          │
│  (One-shot timed events: "building finishes at T+5s")   │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│              Tiered Stride Schedulers                   │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐    │
│  │CRITICAL │→ │ GAMEPLAY│→ │ ECONOMY │→ │COSMETIC │    │
│  │(combat) │  │(agents) │  │ (trade) │  │ (grass) │    │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘    │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│                 LOD/Coarsening Layer                    │
│    (Adjusts update deltas based on visibility/distance) │
└─────────────────────────────────────────────────────────┘
```

## Priority Calculation

Final priority is computed from:
1. **Importance tier** - Determines which scheduler tier handles the process
2. **Proximity** - Distance to nearest player/agent
3. **Visibility** - On-screen vs off-screen, affects update frequency

---

## Phase 1: Tiered Scheduler

**Goal**: Extend base `Scheduler` to support importance tiers with budget allocation.

### 1.1 Define Importance Tiers

- [ ] Create `ProcessTier` enum in `Scheduler.hpp`:
  - `CRITICAL` - Combat, agent decisions, player interactions
  - `GAMEPLAY` - NPC AI, pathfinding, active quests
  - `ECONOMY` - Trade routes, resource production, market prices
  - `COSMETIC` - Grass growth, ambient animations, weather particles

- [ ] Define budget allocation per tier (percentage of frame time):
  - CRITICAL: 40%
  - GAMEPLAY: 30%
  - ECONOMY: 20%
  - COSMETIC: 10%

### 1.2 Create TieredScheduler Class

- [ ] Create `TieredScheduler.hpp`
- [ ] Hold one `Scheduler` instance per tier
- [ ] Implement `AddProcess(id, tier, priority)`
- [ ] Implement `GetNext()` that respects tier ordering and budgets
- [ ] Implement `RemoveProcess(id)` that finds correct tier
- [ ] Implement `UpdatePriority(id, new_priority)`
- [ ] Add `SetTierBudget(tier, percentage)` for runtime tuning

### 1.3 Budget Enforcement

- [ ] Track time spent per tier each frame
- [ ] `GetNext()` skips to lower tier when budget exhausted
- [ ] Add `ResetFrameBudgets()` to call at frame start
- [ ] Consider soft vs hard budget enforcement (allow overflow for CRITICAL)

### 1.4 Testing

- [ ] Unit tests for tier ordering
- [ ] Unit tests for budget enforcement
- [ ] Test that lower tiers still run when higher tiers are empty

---

## Phase 2: Timed Event Queue

**Goal**: Handle one-shot delayed events (building completion, cooldowns, scheduled actions).

### 2.1 Create TimedEventQueue Class

- [ ] Create `TimedEventQueue.hpp`
- [ ] Store events as `{timestamp, callback/event_data}` pairs
- [ ] Use min-heap or sorted container (priority_queue with smallest time first)
- [ ] Implement `Schedule(event, delay)` - schedules event at `now + delay`
- [ ] Implement `ScheduleAt(event, timestamp)` - schedules at absolute time
- [ ] Implement `Cancel(event_id)` - removes pending event

### 2.2 Event Processing

- [ ] Implement `ProcessDueEvents(current_time)`:
  - Pop all events where `timestamp <= current_time`
  - Execute or dispatch each event
- [ ] Decide event representation:
  - Option A: `std::function<void()>` callbacks
  - Option B: Event struct with type enum + data union
  - Option C: Event IDs that map to handlers

### 2.3 Integration with Scheduler

- [ ] Events can register ongoing processes in TieredScheduler
- [ ] Events can wake sleeping processes
- [ ] Events can trigger priority boosts

### 2.4 Testing

- [ ] Unit tests for event ordering
- [ ] Unit tests for cancellation
- [ ] Test events scheduled for same timestamp

---

## Phase 3: LOD/Coarsening Layer

**Goal**: Reduce update frequency for distant/invisible entities.

### 3.1 Process Metadata Extension

- [ ] Add to process state:
  - `last_update_time` - When process last ran
  - `target_interval` - Desired time between updates
  - `accumulated_delta` - Time since last update (for scaled updates)

### 3.2 Interval Calculation

- [ ] Create `LODCalculator` class/function
- [ ] Inputs: process position, camera position, on-screen status
- [ ] Output: target update interval
- [ ] Define interval tiers:
  - On-screen, close: 16ms (60 FPS)
  - On-screen, medium: 50ms (20 FPS)
  - On-screen, far: 100ms (10 FPS)
  - Off-screen, close: 200ms (5 FPS)
  - Off-screen, far: 500ms-1000ms (1-2 FPS)

### 3.3 Scheduler Integration

- [ ] Modify `GetNext()` to check if interval has elapsed
- [ ] Skip processes whose interval hasn't elapsed
- [ ] Pass `delta_time` to process update functions
- [ ] Processes must handle variable delta (e.g., `growth += rate * delta`)

### 3.4 Visibility Detection

- [ ] Define interface for visibility queries
- [ ] Integrate with camera/viewport system
- [ ] Consider chunk-based visibility (cheaper than per-entity)

### 3.5 Testing

- [ ] Test interval enforcement
- [ ] Test that delta accumulates correctly
- [ ] Verify distant entities still update (just less frequently)

---

## Phase 4: Dynamic Priority System

**Goal**: Automatically adjust priority based on game state.

### 4.1 Priority Components

- [ ] Create `PriorityCalculator` class
- [ ] Define priority formula:
  ```
  final_priority = base_priority
                 + proximity_bonus
                 + activity_bonus
                 + event_boost
  ```

### 4.2 Base Priority

- [ ] Define base priorities per entity type:
  - Player agent: 100
  - Hostile NPC: 50
  - Friendly NPC: 20
  - Village: 15
  - Trade post: 10
  - Resource node: 5
  - Environment tile: 1

### 4.3 Proximity Bonus

- [ ] Implement distance-based bonus calculation
- [ ] Consider multiple agents (use nearest)
- [ ] Formula: `bonus = MAX_PROXIMITY_BONUS / (1 + distance * FALLOFF)`
- [ ] Define MAX_PROXIMITY_BONUS and FALLOFF constants

### 4.4 Activity Bonus

- [ ] Bonus for entities with pending actions
- [ ] Bonus for entities in active interactions
- [ ] Bonus for entities player has selected/focused

### 4.5 Event Boost (Temporary)

- [ ] Implement `ApplyBoost(process_id, amount, duration)`
- [ ] Store active boosts with expiration times
- [ ] Remove expired boosts during priority calculation
- [ ] Use cases: combat started, trade completed, damage taken

### 4.6 Priority Update Triggers

- [ ] Define when to recalculate priorities:
  - Option A: Every N frames (simple, predictable cost)
  - Option B: On agent movement (accurate, variable cost)
  - Option C: On chunk boundary crossing (balanced)
- [ ] Implement chosen strategy
- [ ] Batch priority updates for efficiency

### 4.7 Testing

- [ ] Test proximity bonus calculation
- [ ] Test boost application and expiration
- [ ] Verify priorities update when agents move

---

## Phase 5: Sleep/Wake System

**Goal**: Remove idle processes from active scheduling.

### 5.1 Sleep Mechanism

- [ ] Add `SleepProcess(id)` - removes from active scheduler, keeps state
- [ ] Add `SleepUntil(id, timestamp)` - auto-wake at time
- [ ] Add `SleepUntilEvent(id, event_type)` - wake on specific event
- [ ] Store sleeping processes in separate container

### 5.2 Wake Mechanism

- [ ] Add `WakeProcess(id)` - returns process to scheduler
- [ ] Set initial pass to current min pass (fair re-entry)
- [ ] Process timed wake-ups in event queue

### 5.3 Auto-Sleep Conditions

- [ ] Define when entities should auto-sleep:
  - Village with no pending production and no threats
  - Trade post with no incoming caravans
  - Resource node fully depleted
  - NPC with no active goals and not visible

### 5.4 Wake Triggers

- [ ] Define wake events:
  - Player enters proximity
  - Entity takes damage
  - Resource becomes available
  - Timer expires
  - External event (caravan arrives)

### 5.5 Testing

- [ ] Test sleep/wake roundtrip
- [ ] Test timed wake-up
- [ ] Verify sleeping processes don't consume scheduler time

---

## Phase 6: Chunk-Based Optimization

**Goal**: Organize processes spatially for cache efficiency and bulk operations.

### 6.1 Chunk Data Structure

- [ ] Define chunk size (e.g., 16x16 tiles)
- [ ] Create `Chunk` struct with:
  - Position (chunk coordinates)
  - List of process IDs in this chunk
  - Aggregate priority/activity level
  - Load state (loaded/unloaded)

### 6.2 Chunk Manager

- [ ] Create `ChunkManager` class
- [ ] Implement `GetChunk(world_position)`
- [ ] Implement `RegisterProcess(id, position, chunk)`
- [ ] Implement `MoveProcess(id, old_chunk, new_chunk)`

### 6.3 Chunk-Level Scheduling

- [ ] Option: Schedule chunks, then processes within chunk
- [ ] Reduces scheduler overhead for large entity counts
- [ ] Active chunks (near player) get more budget

### 6.4 Chunk Loading/Unloading

- [ ] Define load radius around players
- [ ] Unloaded chunks: all processes sleeping
- [ ] Loading a chunk: wake all processes, restore state
- [ ] Unloading: sleep all, optionally serialize state

### 6.5 Testing

- [ ] Test process registration/movement
- [ ] Test chunk load/unload
- [ ] Benchmark with large entity counts

---

## Phase 7: Integration & Polish

**Goal**: Connect all systems and optimize.

### 7.1 Main Loop Integration

- [ ] Define frame structure:
  ```
  1. Process input
  2. Process due events (Phase 2)
  3. Update priorities if needed (Phase 4)
  4. Reset tier budgets
  5. Run scheduled processes until frame budget exhausted
  6. Render
  ```

### 7.2 Save/Load Support

- [ ] Serialize scheduler state (pass values, sleeping processes)
- [ ] Serialize event queue (pending events)
- [ ] Restore deterministically on load

### 7.3 Debug Tooling

- [ ] Add scheduler statistics:
  - Processes per tier
  - Time spent per tier
  - Processes sleeping vs active
  - Average/max process update time
- [ ] Add visualization (optional):
  - Color entities by priority
  - Show update frequency
  - Highlight sleeping entities

### 7.4 Performance Tuning

- [ ] Profile with realistic entity counts
- [ ] Tune budget allocations
- [ ] Tune LOD distance thresholds
- [ ] Consider pass value normalization (prevent float drift)

### 7.5 Determinism (If Required)

- [ ] Evaluate floating point reproducibility needs
- [ ] Consider fixed-point arithmetic for pass/stride
- [ ] Ensure consistent ordering for equal-pass processes

---

## Design Decisions Checklist

Before implementation, decide:

- [ ] **Tier budgets**: Hard limit vs soft preference
- [ ] **Priority recalculation**: Continuous vs periodic vs event-driven
- [ ] **Coarsening granularity**: Per-process vs per-chunk vs per-tier
- [ ] **Off-screen handling**: Slower updates vs batch catch-up vs full pause
- [ ] **Determinism requirement**: Floating point acceptable vs fixed-point needed
- [ ] **Event system**: Callbacks vs typed events vs IDs

---

## File Structure

Suggested file organization:

```
source/core/
├── Scheduler.hpp           # Base stride scheduler (exists)
├── TieredScheduler.hpp     # Phase 1
├── TimedEventQueue.hpp     # Phase 2
├── LODCalculator.hpp       # Phase 3
├── PriorityCalculator.hpp  # Phase 4
├── SleepManager.hpp        # Phase 5 (or integrate into TieredScheduler)
└── ChunkManager.hpp        # Phase 6

tests/
├── unit/
│   ├── Scheduler_test.cpp
│   ├── TieredScheduler_test.cpp
│   ├── TimedEventQueue_test.cpp
│   └── ...
└── integration/
    └── SchedulerIntegration_test.cpp
```

---

## Dependencies Between Phases

```
Phase 1 (Tiered) ─────┬────► Phase 3 (LOD)
                      │
                      └────► Phase 4 (Priority) ────► Phase 5 (Sleep)

Phase 2 (Events) ─────┴────► Phase 5 (Sleep)

Phase 5 (Sleep) ──────────► Phase 6 (Chunks)

All Phases ───────────────► Phase 7 (Integration)
```

**Recommended order**: 1 → 2 → 4 → 3 → 5 → 6 → 7

Phases 1 and 2 are foundational. Phase 4 (priority) is more important for gameplay feel than Phase 3 (LOD), which is purely optimization. Sleep system depends on having events to trigger wakes. Chunks are optional optimization for scale.
