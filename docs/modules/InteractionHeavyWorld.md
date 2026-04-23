# Interaction Heavy World Module

**Documented By:** Truong Phan
**Group 08:** Truong Phan, Jose Hernandez, Benjamin Forbes, George Almeida, John Masterman

---

## 0. Introduction

This module implements a **2D, grid-based dungeon simulation** where a player navigates through a
dungeon-like generated map, with the goal of reaching the end. Along the way, the player must
manage their inventory of resources, and fight enemies.

The module was developed with assistance from Claude AI and Copilot. The dungeon layout was designed by the authors using [donjon; RPG Tools](https://donjon.bin.sh/) and loaded from a text file.

Core gameplay loop:
- **Navigating** a dungeon loaded from a tile map
- **Gathering** stone and gold from boulders and chests
- **Engaging** or evading hunter, goblin, and pacing enemies
- **Reaching** the exit tile to win

---

## 1. Main Structure

### Objective

The goal is to **reach the exit tile** (`X`) while surviving enemy encounters and collecting resources.

**Win Condition:**
- Player moves onto the exit tile

**Lose Condition:**
- Player HP drops to 0 from enemy contact damage

---

### Simulation Setup

At the start of each session:
- The dungeon is loaded from a text file (`DungeonMapLarge.txt` or `DungeonMapSmall.txt`)
- The grid is populated with:
  - Walls, floors, a start tile (`S`), and an exit tile (`X`)
  - Chests (`C`) placed by the map author
  - Boulders placed **randomly** on valid floor tiles
- Enemies are spawned at positions encoded in the map (`H`, `G`, `P`)

The player begins at the `S` tile with:
- 100 HP
- 0 Stone
- 0 Gold

---

### Agent Behavior

Each simulation action, an agent receives a list of available actions and selects one.

#### Agent Roles

- **Player**
  - Moves, collects resources, breaks boulders, throws stones, pays goblins
  - Is the only non-combat agent

- **HunterAgent**
  - Pursues the player actively
  - Deals contact damage when adjacent
  - Has a finite HP pool; can be defeated by thrown stones

- **PacingAgent**
  - Patrols in a set pattern
  - Deals contact damage when adjacent
  - Has a finite HP pool; can be defeated by thrown stones

- **GoblinAgent**
  - Blocks a passage
  - Can be cleared by paying 5 gold

---

### Gameplay Progression

Gameplay progresses through these phases:

1. Player explores dungeon and breaks boulders for stone and gold
2. Player opens chests for bonus gold
3. Player uses stone offensively to defeat or avoid enemies
4. Player reaches the exit tile to trigger the win condition

---

## 2. Available Actions

### Movement

- Up
- Down
- Left
- Right

> Movement is blocked by walls, boulders, open/closed chests, and agents occupying the target tile.

---

### Resource Interaction

| Action | Description |
|--------|-------------|
| `collect` | Picks up dropped material or opens a chest on an adjacent/same tile |
| `break_boulder` | Breaks an adjacent boulder, converting it to dropped material |
| `pay` | Pays 5 gold to a nearby goblin to clear its blocking position |
| `print_inventory` | Prints current HP, stone, gold, and enemies killed to stdout |

---

### Combat — Throwing

Thrown stones travel up to **4 tiles** in a straight line and deal **10 damage** on hit.

| Action | Direction |
|--------|-----------|
| `throw_up` | Upward (-Y) |
| `throw_down` | Downward (+Y) |
| `throw_left` | Leftward (-X) |
| `throw_right` | Rightward (+X) |

> Stones are consumed on throw, regardless of hit or miss. Projectiles stop at solid tiles (walls, boulders, chests).

---

## 3. World Map Format

The dungeon is loaded from a plain-text tile map. Each character maps to a tile type or spawn event:

| Character | Meaning |
|-----------|---------|
| `#` | Wall (impassable) |
| ` ` | Floor (passable) |
| `S` | Player start position (treated as floor at runtime) |
| `X` | Exit tile |
| `C` | Chest (collectible resource, impassable) |
| `H` | Hunter spawn position (treated as floor at runtime) |
| `G` | Goblin spawn position (treated as floor at runtime) |
| `P` | Pacing enemy spawn position (treated as floor at runtime) |
| `O` | Boulder (placed procedurally, impassable until broken) |

---

## 4. Cell Types

Registered via `ConfigureCellTypes()` at construction:

| Cell Type | Symbol | Passable | Description |
|-----------|--------|----------|-------------|
| `wall` | `#` | No | Permanent barrier |
| `floor` | ` ` | Yes | Open passage |
| `start` | `S` | Yes | Initial player position |
| `exit` | `X` | Yes | Win-condition tile |
| `boulder` | `O` | No | Breakable resource node |
| `chest` | `C` | No | Locked chest, collectible |
| `material` | `M` | Yes | Dropped loot after breaking a boulder |
| `chest_open` | `c` | No | Opened (depleted) chest |

---

## 5. Boulder Placement

Boulders are placed randomly after the map is loaded. Placement is constrained:

- Must land on a `floor` tile
- Must not overlap reserved positions (start, exit, enemy spawns)
- Must not be within `kMinSpawnDistance` (5 tiles) of the start or exit

| Map Size | Min Boulders | Max Boulders |
|----------|-------------|-------------|
| Small | 3 | 5 |
| Large | 75 | 110 |

Each broken boulder drops a random loot bundle (keyed by cell position):

| Resource | Max Drop |
|----------|----------|
| Stone | 15 |
| Gold | 4 |

---

## 6. Combat System

### Enemy Contact Damage

- Any live combat agent adjacent (Manhattan distance = 1) to the player at the end of their or the player's turn deals **1 HP** of contact damage
- Only one damage event is applied per player turn (`mDamagedThisTurn` flag)
- HP is tracked in `mPlayerHP`; reaching 0 triggers `EndGame(false)`

### Enemy HP

- All combat agents default to **10 HP** on first access
- HP is stored per agent ID in `mCombatAgentHP`
- Defeated agents are moved off-grid and their kill is counted in `mEnemiesKilled`

### Stone Throw

- Costs 1 stone per throw
- Deals 10 damage on hit
- Range: 4 tiles in a straight line
- Terminates on solid tiles or on hitting an agent

---

## 7. Resource & Inventory System

Resources are tracked in three fields and synced into the WorldBase resource vector via `SyncResources()` after every mutation:

| Resource | Field | Default |
|----------|-------|---------|
| HP | `mPlayerHP` | 100 |
| Stone | `mStoneCount` | 0 |
| Gold | `mGoldCount` | 0 |

### Chest Reward

Opening a chest grants **4 gold** and converts the cell to `chest_open` (impassable, no further reward).

### Goblin Payment

Paying a goblin costs **5 gold**. On success, the goblin is moved off-grid and the passage is cleared.

---

## 8. Game Session Timer

A session timer (`Game::Session`) is started on the player's **first movement action** and stopped when `EndGame()` is called. This timer is managed through the WorldBase timer interface.

---

## 9. Public API

```cpp
// Resource getters
constexpr size_t GetStoneCount() const;
constexpr size_t GetGoldCount()  const;
constexpr int    GetPlayerHP()   const;

// Position getters
WorldPosition              GetStartPosition()        const;
std::vector<WorldPosition> GetHunterSpawnPositions() const;
std::vector<WorldPosition> GetGoblinSpawnPositions() const;
std::vector<WorldPosition> GetPacingSpawnPositions() const;
WorldPosition              GetRandomPosition()       const;

// Query helpers
bool IsEnemyAt(const WorldPosition& pos)                                        const;
bool NearPosition(const WorldPosition& pos, const WorldPosition& referencePos)  const;
bool IsReservedPosition(const WorldPosition& pos)                               const;

// Resource actions (also callable from tests)
void PlaceBoulders(int minBoulders, int maxBoulders);
void PrintInventory() const;
void BreakBoulder(size_t x, size_t y);
void Collect(size_t x, size_t y);
void Pay(size_t x, size_t y);
void ThrowStone(size_t x, size_t y, int dx, int dy);

// WorldBase override
int DoAction(AgentBase& agent, size_t actionId) override;
```

---

## 10. Constants Reference

| Constant | Value | Description |
|----------|-------|-------------|
| `kPlayerStartHP` | 100 | Initial player HP |
| `kHunterDefaultHP` | 10 | Default HP for all combat agents |
| `kThrowDamage` | 10 | Damage per stone throw |
| `kThrowRange` | 4 | Max tiles a thrown stone travels |
| `kEnemyContactDamage` | 1 | HP lost per adjacent enemy per turn |
| `kChestGoldReward` | 4 | Gold gained from opening a chest |
| `kGoblinGoldCost` | 5 | Gold required to pay off a goblin |
| `kMinSpawnDistance` | 5 | Minimum Manhattan distance from start/exit for placements |
| `kMaxBoulderStone` | 15 | Max stone dropped from one boulder |
| `kMaxBoulderGold` | 4 | Max gold dropped from one boulder |

---

## 11. Dependencies

| File | Role |
|------|------|
| `InteractionHeavyWorld.hpp/.cpp` | World implementation |
| `GoblinAgent.hpp` | Blocking NPC agent |
| `HunterAgent.hpp` | Pursuing combat agent |
| `PacingAgent.hpp` | Patrolling combat agent |
| `WorldBase.hpp` | Base class (grid, agents, resources, timer) |
| `DungeonMapLarge.txt` / `DungeonMapSmall.txt` | Tile map input files |

---

## 12. Summary

This module provides:
- A dungeon simulation loaded from a human-authored tile map
- Procedural boulder placement with controlled randomness
- A multi-type enemy system with HP tracking and defeat logic
- A resource and inventory pipeline (stone, gold, HP) synced to the WorldBase layer
- A projectile combat mechanic (stone throwing)
- A session timer tied to the player's first move
- Clear win/lose conditions with informative console output
