# Dynamic World Module

**Documented By:** John Korreck  
**Developed By:** John Korreck, Sehaj Gupta, Dmitry Starodubtsev, Logan Langmeyer, Myles Spencer-Walker  

---

## 0. Introduction

This module implements a **2D, grid-based simulation** where agents interact with an environment to gather resources, construct buildings, and scale production.

The system evolves over time from:
- **Manual resource collection**
→ to  
- **Automated production via buildings**

---

## 1. Main Structure

### Objective

The primary goal is to **build a Town Hall** by efficiently managing resources.

**Win Condition:**
- 500 Wood  
- 500 Stone  
- 500 Steel  
- 500 Wheat  

---

### Simulation Setup

At the start:
- The user is given **16 agents**
  - 15 **collector agents**
  - 1 **leader agent**

The user:
- Places agents on the grid strategically  
- Then relinquishes control  

After initialization:
- The simulation runs **fully autonomously**

---

### Agent Behavior

Each simulation tick:
- Agents receive:
  - Current **world state**
  - List of **available actions**
- Agents select an action based on internal logic (or AI)

#### Agent Roles

- **Collectors**
  - Gather raw materials

- **Leader (AI-capable)**
  - Can build structures
  - Can influence strategy (future: communication, coordination)

---

### System Evolution

The simulation progresses as:

1. Agents gather resources  
2. Buildings are constructed  
3. Resource production scales  

Eventually:
- The system shifts from **manual collection → automated generation**

---

## 2. Available Actions

### Movement
- Up  
- Down  
- Left  
- Right  
- Up-left  
- Up-right  
- Down-left  
- Down-right  

---

### Resource Collection
- Wood  
- Stone  
- Wheat  

---

### Building Construction

Buildings:
- Must be placed on **empty tiles**
- Require **resource costs**

#### Buildings

| Building      | Cost                     | Output |
|--------------|--------------------------|--------|
| Quarry       | 20 Stone + 20 Wood       | +1 Steel / 40 ticks, +1 Stone / 10 ticks |
| Lumberyard   | 20 Wood + 20 Steel       | +1 Wood / 20 ticks |
| Farm         | 20 Wheat + 20 Wood       | +1 Wheat / 10 ticks |
| Spawner      | —                        | Spawns agent every 60 ticks |
| Town Hall    | —                        | **Win condition** |

---

### Future Actions
- **Communicate**
  - Agents coordinate strategies
  - Adjust priorities dynamically

---

## 3. UI Requirements

The interface should display:

- Grid contents:
  - Terrain
  - Resources
  - Buildings
  - Agents *(rendered on top)*
- Resource totals (collection pool)
- Time elapsed

> UI may use stock images for visualization.

---

## 4. Database Requirements

The system must support:

- Saving world state  
- Loading world state  

### Persistence Design

- Worlds are tied to a **generated UserID**
- Users can resume simulations from previous sessions

---

## 5. Public API

```cpp
std::vector<size_t> GetGrid();
```

### Description
Returns a representation of the **WorldGrid**, used by:
- UI for visualization  
- Agents for decision-making  

---

## 6. Summary

This module provides:
- A scalable simulation framework  
- Autonomous agent behavior  
- A progression system from manual → automated production  
- A foundation for future AI coordination features  