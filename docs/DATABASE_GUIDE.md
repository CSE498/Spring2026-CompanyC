# Database Usage Guide

**Group 9 — CSE 498 Capstone**

A simple guide for all teams to store and retrieve data using the shared Database class.

---

## Getting Started

Include the header:

```cpp
#include "core/Database.hpp"
using namespace cse498;
```

Create a database:

```cpp
// Option 1: In-memory only (data lost when program exits)
Database db;

// Option 2: Persistent (data saved to a file on disk)
Database db("game_data.db");

// Option 3: Use a config object
DatabaseConfig config;
config.db_path = "game_data.db";
config.auto_compress = true;
config.compression_threshold = 200;
config.verbose = false;
Database db_with_config(config);
```

That's it. Everything below works the same way regardless of which option you pick.

`Database` is not thread-safe right now. If multiple threads use the same database object, add your own locking around it.

---

## What Works Out of the Box

You can store these types without registering anything:

- `int`, `double`, `bool`, `char`, `std::string`
- `std::vector<T>`
- `std::map<K, V>`
- `std::unordered_map<K, V>`

These shared project types are also registered for you automatically:

- `DataGrid`
- `WorldPosition`
- `Location`

If your own class is not on that list, register it with `RegisterType<T>()`.

---

## Storing Data

Use `Store` to save any supported value under a string key. If the key already exists, it gets overwritten.

```cpp
db.Store("player:health", 100);
db.Store("player:name", std::string("Alice"));
db.Store("player:alive", true);
db.Store("player:speed", 3.14);
```

You can also store containers:

```cpp
std::vector<int> scores = {10, 20, 30};
db.Store("player:scores", scores);

std::map<std::string, int> inventory = {{"sword", 1}, {"potion", 5}};
db.Store("player:inventory", inventory);

std::unordered_map<std::string, double> position = {{"x", 10.5}, {"y", 20.25}};
db.Store("player:position_map", position);
```

---

## Built-In Shared Types

Some shared framework types are already registered inside `Database`, so you can store them directly.

```cpp
WorldPosition pos(10.5, 20.25);
db.Store("player:position", pos);

Location loc(WorldPosition(5, 7));
db.Store("item:location", loc);

DataGrid grid(2, 2);
grid.Insert(0, 0, "tree");
grid.Insert(0, 1, true);
grid.Insert(1, 0, 3.14);
grid.Insert(1, 1, 42);
db.Store("world:grid", grid);
```

You do not need to call `RegisterType` for those three types.

---

## Loading Data

Use `Load<Type>` to retrieve a value. You must specify the same type you stored it as.

```cpp
auto health = db.Load<int>("player:health");
if (health.has_value()) {
    std::cout << "Health: " << *health << std::endl;
} else {
    std::cout << "Key not found!" << std::endl;
}

auto name = db.Load<std::string>("player:name");
auto scores = db.Load<std::vector<int>>("player:scores");
```

---

## Updating Data

Use `Update` when you know the key already exists. It's optimized to store only what changed.

```cpp
db.Store("player:health", 100);

// Later, health changes:
db.Update("player:health", 85);
```

If the key doesn't exist, `Update` returns an error — use `Store` for new entries.

---

## Checking and Deleting

```cpp
// Check if a key exists
if (db.Exists("player:health")) {
    // ...
}

// Delete a single key
db.Delete("player:health");

// Delete everything
db.Clear();

// How many entries are stored?
size_t count = db.Size();
```

---

## Finding Keys

Use `FindKeys` with wildcard patterns to search for keys.

- `*` matches any number of characters
- `?` matches exactly one character

```cpp
// Find all player keys
auto player_keys = db.FindKeys("player:*");

// Find all chunk keys for any world
auto chunks = db.FindKeys("world:*:chunk:*");

// List every key in the database
auto all_keys = db.ListKeys();
```

---

## Checking What Type Is Stored

Before loading, you can check what type a key holds:

```cpp
auto type = db.GetType("player:health");
if (type.has_value()) {
    std::cout << *type << std::endl;  // prints "int"
}
```

Possible values: `"int"`, `"double"`, `"bool"`, `"char"`, `"string"`, `"vector"`, `"map"`, `"unordered_map"`, or the name of a registered custom type.

If `store_type_metadata` is turned off in the config, `GetType` may return an empty string.

---

## Storing Custom Game Objects

If your team has its own classes (agents, entities, items, etc.), you can register them with the database so `Store` and `Load` work directly.

```cpp
struct Agent {
    int id;
    std::string name;
    double health;
};

// Register the type once at startup
db.RegisterType<Agent>("Agent",
    // How to convert Agent -> string
    [](const Agent& a) {
        Serializer s;
        return s.Serialize(a.id) + s.Serialize(a.name) + s.Serialize(a.health);
    },
    // How to convert string -> Agent
    [](const std::string& data) -> std::optional<Agent> {
        Serializer s;
        size_t pos = 0;
        auto id = s.DeserializeAt<int>(data, pos);
        auto name = s.DeserializeAt<std::string>(data, pos);
        auto health = s.DeserializeAt<double>(data, pos);
        if (!id || !name || !health) return std::nullopt;
        return Agent{*id, *name, *health};
    }
);

// Now you can store and load Agents directly
Agent alice{1, "Alice", 100.0};
db.Store("agent:1", alice);

auto loaded = db.Load<Agent>("agent:1");  // returns the Agent
```

You can also check registration:

```cpp
if (db.IsTypeRegistered("Agent")) {
    // safe to store/load Agent objects
}
```

---

## Transactions (All-or-Nothing Saves)

When saving multiple related values, use transactions to make sure they all succeed or none do.

```cpp
db.BeginTransaction();

db.Store("player:health", 100);
db.Store("player:position", position);
db.Store("player:inventory", inventory);

db.Commit();    // All three saved together
// or
db.Rollback();  // Undo all three — nothing changed
```

This is especially useful with the persistent database to prevent partially saved game states.

Transactions are not nested. Start one transaction, then either `Commit()` or `Rollback()` before starting another.

---

## Saving and Loading from Files

Even in in-memory mode, you can dump all data to a file and reload it later.

```cpp
// Save everything to a file
db.SaveToFile("quicksave.bin");

// Later, load it back (into any database, in-memory or persistent)
db.LoadFromFile("quicksave.bin");
```

Loaded entries merge with existing data. If a key already exists, the loaded value overwrites it.

---

## Changing Database Settings

If you want more control, use `DatabaseConfig`.

```cpp
DatabaseConfig config;
config.db_path = "game_data.db";   // empty string = in-memory only
config.auto_compress = true;       // compress larger values automatically
config.compression_threshold = 150;
config.verbose = true;             // print debug messages to stderr
config.store_type_metadata = true; // lets GetType() report stored types

Database db(config);
```

Advanced SQLite fields also exist in `DatabaseConfig` (`wal_mode` and `auto_flush`), but most teams can ignore them.

You can also change the config later:

```cpp
DatabaseConfig config = db.GetConfig();
config.verbose = true;
db.SetConfig(config);
```

---

## Checking Storage Size

If you want to know how many bytes one entry is using inside the database:

```cpp
auto size = db.GetStorageSize("player:inventory");
if (size.has_value()) {
    std::cout << "Stored bytes: " << *size << std::endl;
}
```

---

## Key Naming Conventions

We recommend using colon-separated keys to keep things organized:

| Pattern | Example |
|---------|---------|
| Player data | `player:<id>:health`, `player:<id>:name` |
| World data | `world:<name>:chunk:<x>:<y>` |
| Config | `config:difficulty`, `config:volume` |
| NPC data | `npc:<id>:position`, `npc:<id>:dialogue` |

This makes `FindKeys("player:alice:*")` return all of Alice's data.

---

## World Serialization

`SaveWorld` and `LoadWorld` (from `WorldHelpers.hpp`) serialize a `WorldBase`'s runtime state into the Database using structured key conventions. This lets you save and restore entire game worlds.

```cpp
#include "core/WorldHelpers.hpp"
```

### World key conventions

| Key pattern | Contents |
|-------------|----------|
| `world:<name>:meta` | Agent count, item count, run_over flag |
| `world:<name>:grid` | Grid dimensions + all cell values (row-major) |
| `world:<name>:agent:<id>` | Agent name, location, symbol |
| `world:<name>:item:<id>` | Item name, location |

### Basic usage

```cpp
#include "core/WorldHelpers.hpp"
#include "../Worlds/MazeWorld.hpp"
#include "../Agents/PacingAgent.hpp"

// Set up a world
MazeWorld world;
auto& agent = world.AddAgent<PacingAgent>("Hero");
agent.SetLocation(WorldPosition(3.0, 5.0));

// Save to Database
Database db;
auto result = SaveWorld(db, "maze", world);

// Later, restore into a fresh world with the same structure
MazeWorld world2;
world2.AddAgent<PacingAgent>("placeholder");
LoadWorld(db, "maze", world2);
// world2 now has agent named "Hero" at position (3, 5)
```

`LoadWorld` requires the target world to have the same number of agents and items as the saved world. It restores names, locations, symbols (agents), and grid cell values. It does not construct new entities — it updates the ones already in the world.

### Extending with derived-class fields

`SaveWorld` saves base-class fields only (name, location, symbol). If your group has derived agents or items with extra fields, store them under the same key prefix:

```cpp
// After calling SaveWorld:
db.Store("world:maze:agent:0:health", my_agent.GetHealth());
db.Store("world:maze:agent:0:inventory", my_agent.GetInventory());
db.Store("world:maze:item:0:durability", my_item.GetDurability());
```

These extension keys are captured automatically by `SaveGame()`, which exports the entire Database. On load, call `LoadWorld` for base fields, then load your extension keys:

```cpp
LoadWorld(db, "maze", world);
auto health = db.Load<int>("world:maze:agent:0:health");
if (health) my_agent.SetHealth(*health);
```

### Integration with save server

`SaveWorld` stores data into a `Database`. To persist across machines, use the save server pipeline:

1. `SaveWorld(db, "my_world", world)` — serialize world into Database
2. `sync.SaveGame("my_save")` — upload entire Database to save server
3. On another machine: `sync.LoadGame("my_save")` — download into Database
4. `LoadWorld(db, "my_world", world)` — restore world state

Look at `source/SaveServer_main.cpp` documentation at the top to see how to start up the server.

---

## Error Handling

Most methods return `std::expected`. Check `.has_value()` before using the result:

```cpp
auto result = db.Load<int>("some_key");
if (result.has_value()) {
    int value = *result;
} else {
    // result.error() tells you what went wrong:
    //   KeyNotFound          — key doesn't exist
    //   DeserializationFailed — wrong type (stored int, loaded as string)
    //   IOError              — disk read/write failed (persistent mode)
}
```

---

## Quick Reference

| What you want to do | Code |
|---------------------|------|
| Create in-memory DB | `Database db;` |
| Create persistent DB | `Database db("game.db");` |
| Save a value | `db.Store("key", value);` |
| Get a value | `auto v = db.Load<int>("key");` |
| Update a value | `db.Update("key", new_value);` |
| Check if key exists | `db.Exists("key")` |
| Delete a key | `db.Delete("key")` |
| List all keys | `db.ListKeys()` |
| Find keys by pattern | `db.FindKeys("player:*")` |
| Get storage size | `db.GetStorageSize("key")` |
| Check stored type | `db.GetType("key")` |
| Register custom type | `db.RegisterType<T>(...)` |
| Check if type is registered | `db.IsTypeRegistered("MyType")` |
| Begin transaction | `db.BeginTransaction()` |
| Commit transaction | `db.Commit()` |
| Rollback transaction | `db.Rollback()` |
| Save to file | `db.SaveToFile("save.bin")` |
| Load from file | `db.LoadFromFile("save.bin")` |
| Create with config | `Database db(config);` |
| Read config | `db.GetConfig()` |
| Update config | `db.SetConfig(config)` |
