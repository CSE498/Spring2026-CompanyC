[Directory](/source/Worlds/InteractionHeavyWorld.hpp)

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
  - Can be cleared by paying gold

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
| `pay` | Pays gold to a nearby goblin to clear its blocking position |
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

## 11. Dependencies

### Internal Modules

| Component | Role |
|----------|------|
| `WorldBase.hpp` | Base framework providing grid, agent management, resources, and timer system |
| `AgentBase.hpp` | Base interface for all agents acting within the world |

### Agent Integrations

| Component | Role |
|----------|------|
| `HunterAgent` | Combat agent that actively pursues the player and participates in HP-based combat |
| `PacingAgent` | Combat agent that patrols and deals contact damage when adjacent |
| `GoblinAgent` | Blocking NPC that can be removed via the `Pay()` interaction |

### UI / Output

| Component | Role |
|----------|------|
| `EndGameScreen.hpp` | Generates HTML output for the endgame results screen |

### Data Files

| File | Role |
|------|------|
| `DungeonMapLarge.txt` / `DungeonMapSmall.txt` | Input maps used to construct the dungeon layout |

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