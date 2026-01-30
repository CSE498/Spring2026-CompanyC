# Module Description

We are building the foundation for a miniature civilization. Instead of a dead map, we are creating a world that changes based on what the AI agents do. The main goal for the agents is to go from being lone survivors to building a thriving, high-tech village by gathering resources and working together.

## What We Are Building

We are building the physical world and its rules.

- **The Land**: A grid of tiles where everything happens.
- **The Buildings**: Structures like Mines, Houses, and Markets that agents "buy" and place into the world.
- **The Economics**: A system where the price of wood or gold changes automatically based on how much is available in the village.
- **The Heartbeat**: A "timer" that makes sure the most important things (like a busy market) update faster than slow things (like a tree growing).

## How Agents Interact with the World

The world is the "authority." Agents can't just cheat; they have to follow the world's laws.

- **Building**: If an agent wants to build a Wall, they ask the world. The world checks if they have enough wood. If they do, the world takes the wood and places the Wall.
- **Harvesting**: Agents "attack" or interact with a resource tile (like an Iron Deposit) to pull materials out of the land and into their pockets.
- **Ownership**: The world tracks who owns what. If Agent A builds a house, the world tells Agent B, "You can't go in there; it belongs to Agent A."

## How Agents Interact with Each Other

We provide the "social space" for them to talk and trade.

- **The Market (Trading)**: We build specific "Market Hub" locations. To trade, agents must walk to the same spot. One agent drops off an item, the other picks it up, and the world makes sure the payment is handled fairly so nobody gets robbed.
- **Leaving Notes**: Agents can "write" on the world. An agent can leave a virtual sign on a tile that says "Big monster this way" or "I claim this forest." Other agents walk by later and "read" the sign.
- **The Shout**: An agent can ask the world, "Who is standing near me?" The world gives them a list of names so they can start a conversation or a fight.
 
# StateGrid Class

## Overall Vision / Class Description

StateGrid will be a grid representation of the environment that the agent simulation will run in. It defines the boundaries and parameters of the simulation and manages the locations of agents. Each state "type" will have a name, a symbol, and any other parameters needed.

## Similar Classes

- `std::vector`: Used for continuous grid storage
- `std::unordered_map`: Used for state parameter management

## Key Functions

- `void Resize(int width, int height)`
- `int RegisterState(StateType state)`
- `int GetState(int x, int y)`
- `void SetState(int x, int y, int stateID)`
- `bool InBounds(int x, int y)`

## Error Conditions

- Out of bounds error → programmer error, assert
- Invalid StateID → programmer error, assert

## Expected Challenges

Efficient state management will be important as the grid could be a bottleneck in the program if not fast enough. The entire program will run based off the StateGrid so we need to make sure optimizations are made for performance.

## Other Classes to Coordinate With

- **StateGridPosition** (Our Group): tracking of agents within the grid
- **Classic Agents / AI Agents** (Groups 5 & 6): querying grid state and properties
- **Web Interface** (Group 24): rendering grid representations


# StateGridPosition

## Class Description

StateGridPosition represents a single "agent location" inside a StateGrid, with information to support movement, navigation, and comparisons with other agent positions. At minimum it stores a (row, col) (or (x, y)) coordinate, and an orientation the agent is facing.

### High-level Goals

- Provide a wrapper around grid coordinates so the rest of the code doesn't pass raw ints everywhere.
- Make common grid operations simple and consistent: agent position lookup, movement, bounds checking, distance, comparison, etc.
- Support pathfinding and agent behavior by enabling cheap hashing / ordering and easy conversion to/from index forms.

### Typical Usage

- An agent stores a `StateGridPosition pos;`
- The world updates the agent via `pos.StepForward()` / `pos.Move(Direction::North)`.
- Pathfinding systems use it as a node type (hashable/comparable).

## Similar Standard Library Classes

- `std::pair<int,int>` / `std::tuple<int,int,Dir>`: simple coordinate containers
- `std::array<int,2>`: fixed-size coordinate representation
- `std::hash` + unordered containers (`std::unordered_set`, `std::unordered_map`): for pathfinding visited sets
- `std::strong_ordering` / comparisons (`operator<=>`): for sorting positions

## Key Functions to Implement

### Core Construction + Access

- `StateGridPosition(int row, int col);`
- `StateGridPosition(int row, int col, Direction facing);`
- `int Row() const;`
- `int Col() const;`
- `Direction Facing() const;`
- `void SetFacing(Direction d);`

### Movement / Neighbors

- `StateGridPosition Moved(Direction d, int steps=1) const;` (returns new position)
- `bool CanMove(Direction d, const StateGrid& grid) const;`
- `bool MoveIfValid(Direction d, const StateGrid& grid);` (moves, returns success)

### Comparisons + Rotation

- `bool operator==(const StateGridPosition& other) const;`
- `auto operator<=>(const StateGridPosition& other) const;`
- `StateGridPosition RotatedLeft();` / `RotatedRight();` (if orientation is used)

## Error Conditions

### Programmer Errors (assert)

- Constructing with impossible coordinates if you require validity at construction (e.g., negative row/col).
- Requesting neighbor with an invalid enum value (if Direction is not strongly typed / validated).

### Recoverable Errors (exception)

- `StateGridPosition Parse(string)` and it fails → `std::invalid_argument`.
- If you integrate with a grid pointer/reference and it is null → `std::runtime_error` (or prevent by design).

### User/Input Errors (special return / optional)

- Movement out of bounds: `Neighbor(...)` returns `std::nullopt`.
- Illegal movement into blocked cells (if you check collision rules): `MoveIfValid(...)` returns false.

## Expected Challenges / Topics to Learn

- **Designing a clean interface boundary** between StateGridPosition and StateGrid:
  - Should StateGridPosition be pure data (row/col/facing only), or should it also provide grid-aware queries via passing a `StateGrid&`?
- **Direction/orientation design**:
  - Picking a simple `enum class Direction { North, East, South, West };`
  - Handling rotation and forward movement consistently.
- **Avoiding "silent bugs"**:
  - Clear semantics for row/col vs x/y, and consistent coordinate system.

## Other Groups / Classes to Coordinate With

### Within Group 7

- **StateGrid**: bounds, passability rules (walls vs open), state metadata (symbols/properties)
- **Scheduler**: if scheduling agents at grid positions
- **DataMap**: if position carries per-agent or per-cell extra metadata

### Across the Company

- **Group 6** (Classic Agents): PathGenerator and WorldPath can use StateGridPosition as the node/point type.
- **Group 5** (AI Agents): positions may be tagged/annotated or used as features for learning.
- **Group 24** (Web Interface): for visualizing agents and cursor selections on the grid. 
# Scheduler Class (Deterministic Stride Version)

## Class Description

The Scheduler acts as the "CPU" of the simulation. Its primary goal is to manage the execution frequency of various simulation elements (Markets, Farms, Mines, etc.) based on their importance. Instead of a simple round-robin or random update, it uses Stride Scheduling—a virtual time approach. This ensures high-priority processes run more frequently while preventing low-priority "background" processes (like grass growth) from being entirely starved or causing performance spikes.

## Similar Standard Library Classes

- `std::priority_queue`: This is the closest structural match, as the scheduler needs to constantly retrieve the element with the lowest "pass" value.
- `std::greater`: Used as a comparator for the priority queue to transform it into a min-heap.
- `std::map` or `std::unordered_map`: Necessary to keep track of process IDs and their corresponding metadata (like current stride or position in the heap) for updates or removals.

## Key Functions

- `void AddProcess(size_t id, double priority)`: Calculates the stride using S = L / P, where L is a large constant. It sets the initial Pass to the current minimum Pass in the system to ensure new processes don't "hog" the execution window.
- `size_t GetNext()`: Identifies the process with the minimum Pass value, increments its Pass by its Stride (V = V + S), re-inserts it into the queue, and returns the id to be executed.
- `void UpdatePriority(size_t id, double new_priority)`: Adjusts the stride of an existing process. This is vital if, for example, a Farm becomes more "active" due to a seasonal change.
- `void RemoveProcess(size_t id)`: Removes an entity from the scheduler (e.g., if a building is destroyed).

## Error Conditions

- **Invalid Priority** (Programmer Error): If a user passes a priority of ≤ 0, it would result in a division by zero or infinite stride. This should be caught by an assert.
- **Duplicate ID** (Programmer Error): Adding an ID that already exists in the scheduler. This should be caught by an assert.
- **Empty Scheduler** (User/Recoverable Error): Calling `GetNext()` when no processes are registered. This should return a "null" ID (like `std::numeric_limits<size_t>::max()`) or throw an exception for the main loop to handle.
- **Memory Allocation** (Recoverable Error): If the internal data structures fail to grow due to resource limits, this should trigger a standard `std::bad_alloc` exception.

## Expected Challenges

- **Pass Value Overflow**: If the simulation runs for a very long time, the Pass values (Virtual Time) will eventually overflow. I need to research Virtual Time Wraparound techniques or periodic "normalization" (subtracting the minimum Pass from all processes) to keep the numbers manageable.
- **Pointer Invalidation**: If using a `std::priority_queue`, updating an existing element's priority is difficult because the STL doesn't support "re-heaping" an arbitrary element. I may need to implement a custom heap or use a `std::set` of structs to allow for efficient updates.

## Coordination with Other Groups

Groups 7 and 8 (Dynamic and Interaction-Heavy Worlds) will use the Scheduler to manage ticking of their simulation elements; both groups should define a standard priority interface and communicate how priorities change (e.g., seasonally or event-driven). Group 6 (Classic Agents) may also schedule agent behaviors. Group 9 (Database) must serialize scheduler state (process registry and Pass values) to support save/load and replay functionality. Group 23 (Data Analytics) needs exposure to execution counts and timing statistics for each process. Group 24 (Web Interface) should be able to read the current process list and adjust priorities through safe `UpdatePriority()` calls. The main simulation loop will repeatedly call `GetNext()` to advance virtual time fairly across all systems.
 
# ExpressionParser Class

## Overall Vision / Class Description

ExpressionParser will convert text based mathematical expressions into callable functions that can be evaluated at runtime using dynamic data. This will allow world rules and computed properties to be data driven instead of hardcoded. Expressions are parsed once, validated, and evaluated efficiently many times during sim. This makes it easy to change world behavior without modifying C++ code.

## Similar Classes

- `std::function`: compiled callable expressions
- String tokenization / compiler-style parsing
- Expression trees or Reverse Polish Notation
- `std::unordered_map<string, double>`: for variable lookup

## Key Functions

- `Parse(string expr)`: Parses and validates an expression string and produces a compiled expression.
- `Evaluate(compiledExpr, variables)`: Evaluates the compiled expression using runtime variable values.
  - Supported features include numeric constants, variables, `+ - * /` operators, parentheses, and unary minus

## Error Conditions

- Syntax errors (invalid tokens, mismatched parentheses) → `std::invalid_argument`
- Missing variables or invalid evaluation → `std::runtime_error`
- Division by zero → defined behavior (exception or NaN)
- Internal parser errors → assert (programmer error)

## Expected Challenges

- Correctly handling operator precedence and parentheses
- Designing a parser that is both safe and efficient
- Providing clear error messages for invalid expressions
- Ensuring expressions are parsed once and evaluated quickly

## Other Classes to Coordinate With

- **StateGrid** and **DataMap** (Group 7): computing world and state properties
- **Scheduler** (Group 7): dynamic priorities or update rules
- **AI Agents** and **Classic Agents** (Groups 5 & 6): scoring and decision logic
- **Database** (Group 9): storing expressions as strings
- **Web Interface** (Group 24): user-defined configuration formulas


 
# DataMap

## Class Description

The DataMap class is a dynamic, type safe associative container designed to store and retrieve heterogeneous data types using string based keys. Its primary goal is to provide a flexible interface for managing disparate data while enforcing strict type-checking at runtime. It bridges the gap between dynamic storage and static typing by requiring explicit template arguments during retrieval and validating those types against the stored data via assertions.

## Similar Standard Library Classes

1. `std::unordered_map<std::string, T>`: for the basic key-value association and O(1) average time complexity lookups.
2. `std::any`: for storing values of any type in a single container.
3. `std::variant`: for type-safe unions.
4. `std::type_index` / `std::type_info`: for identifying and comparing the types of stored objects at runtime.

## Key Functions

- `template <typename T> void Set(const std::string& key, T&& value)`: Inserts or updates a value associated with a key.
- `template <typename T> T Get(const std::string& key)`: Retrieves a reference to the stored value. Contains an assert to verify that `typeid(T)` matches the stored type.
- `bool Contains(const std::string& key) const`: Checks if a key exists in the map.
- `void Remove(const std::string& key)`: Delete a key-value pair from the map.
- `void Clear()`: Remove all entries.

## Error Conditions

- **Type Mismatch**: Attempting to `Get<int>` a value stored as a double. Category: Programmer Error. Response: assert failure.
- **Missing Key**: Attempting to `Get` a key that has not been initialized. Category: Programmer Error. Response: assert failure.
- **Memory Allocation**: System fails to allocate memory for a new entry. Category: Recoverable Error. Response: `std::bad_alloc` exception.
- **Invalid Key Format**: A user provides an empty string or illegal character via a UI. Category: User Error. Response: Return false or throw a custom exception.

## Expected Challenges

- **Type Erasure**: Implementing a way to store "any" type in a standard container.
- **Runtime Type Identification**: Correctly using `typeid` or `std::type_index` to ensure the assert triggers accurately when types don't match.
- **Reference Handling**: Ensuring `Get` returns a usable reference or copy without causing memory safety issues.
- **Template Specialization**: Handling edge cases like `const char*` vs `std::string`.

## Other Group's C++ Classes to Coordinate With

- **RobinHood Map** (Group 5): RobinHood map will be faster for accessing data stored in it.
- **Datum** (Group 9): Its use is to store data of different types so could be very useful for DataMap since user can set and get data of various different types.
- **MemoryFactory** (Group 6): If DataMap is changed frequently, standard heap allocation can be slow.
- **Serializer** (Group 9): Can add a crucial save/load feature to DataMap. 

