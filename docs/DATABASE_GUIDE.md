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
```

That's it. Everything below works the same way regardless of which option you pick.

---

## Storing Data

Use `Store` to save any value under a string key. If the key already exists, it gets overwritten.

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
```

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
| Find keys by pattern | `db.FindKeys("player:*")` |
| Check stored type | `db.GetType("key")` |
| Register custom type | `db.RegisterType<T>(...)` |
| Begin transaction | `db.BeginTransaction()` |
| Commit transaction | `db.Commit()` |
| Rollback transaction | `db.Rollback()` |
| Save to file | `db.SaveToFile("save.bin")` |
| Load from file | `db.LoadFromFile("save.bin")` |
