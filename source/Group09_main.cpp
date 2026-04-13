/**
 * @file Group09_main.cpp
 * @author Group-9
 * @brief Example driver demonstrating the Database module and its integration
 *        with other groups' game objects.
 * Used claude code to assist with creating this file
 *
 * This driver shows:
 *  1. Basic Store / Load / Update / Delete of built-in types
 *  2. Registering and persisting custom game-object types (stubs for other
 *     groups' classes are provided so this file compiles standalone)
 *  3. Pattern-based key searching (FindKeys)
 *  4. Transaction / rollback semantics
 *  5. Disk persistence (SaveToFile / LoadFromFile)
 *  6. SQLite-backed persistence (optional, enabled via command-line flag)
 *
 * ─────────────────────────────────────────────────────────────────────────────
 * HOW OTHER GROUPS PLUG IN
 * ─────────────────────────────────────────────────────────────────────────────
 * 1. Include "source/core/Database.hpp"
 * 2. Call db.RegisterType<YourType>(…) once at startup (see examples below).
 * 3. Use db.Store / db.Load / db.Update / db.Delete / db.FindKeys normally.

 * ─────────────────────────────────────────────────────────────────────────────
 */
// ── Real Database header ─────────────────────────────────────────────────────
#include "core/Database.hpp"   
 
// ── Standard library ─────────────────────────────────────────────────────────
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>  
 
// ═════════════════════════════════════════════════════════════════════════════
//  STUB TYPES  –  stand-ins for other groups' game objects
//  Replace these with the real #include from those groups once available.
// ═════════════════════════════════════════════════════════════════════════════
 
namespace stubs {
 
// ── stub: Player ──────────────────────────────────────────────────────
struct Player {
    std::string name;
    int         health   = 100;
    int         level    = 1;
    double      x        = 0.0;
    double      y        = 0.0;
};
 
// ── stub: NPC ────────────────────────────────────────────────────────
struct NPC {
    std::string id;
    std::string dialogue;
    bool        hostile  = false;
};
 
// ──stub: WorldChunk ─────────────────────────────────────────────────
struct WorldChunk {
    int         chunk_x  = 0;
    int         chunk_y  = 0;
    std::string biome;
    std::vector<std::string> entities; // entity IDs present in this chunk
};
 
// ── stub: Config ─────────────────────────────────────────────────────
struct Config {
    std::string key;
    std::string value;
};
 
} // namespace stubs
 
// ═════════════════════════════════════════════════════════════════════════════
//  SERIALIZATION HELPERS  –  teach the Database how to encode each stub type
//  (In production, each group provides these inside their own RegisterType call)
// ═════════════════════════════════════════════════════════════════════════════
 
namespace {
 
using namespace cse498;
 
// ── Register all stub types with a Database instance ─────────────────────────
void RegisterGameTypes(Database& db) {
 
    // ── Player ────────────────────────────────────────────────────────────────
    db.RegisterType<stubs::Player>(
        "Player",
        // Serialize: concatenate field bytes via Serializer
        [](const stubs::Player& p) -> std::string {
            Serializer s;
            return s.Serialize(p.name)
                 + s.Serialize(p.health)
                 + s.Serialize(p.level)
                 + s.Serialize(p.x)
                 + s.Serialize(p.y);
        },
        // Deserialize: read fields back in the same order
        [](const std::string& data) -> std::optional<stubs::Player> {
            Serializer s;
            size_t pos = 0;
            auto name   = s.DeserializeAt<std::string>(data, pos);
            auto health = s.DeserializeAt<int>(data, pos);
            auto level  = s.DeserializeAt<int>(data, pos);
            auto x      = s.DeserializeAt<double>(data, pos);
            auto y      = s.DeserializeAt<double>(data, pos);
            if (!name || !health || !level || !x || !y) return std::nullopt;
            return stubs::Player{*name, *health, *level, *x, *y};
        }
    );
 
    // ── NPC ───────────────────────────────────────────────────────────────────
    db.RegisterType<stubs::NPC>(
        "NPC",
        [](const stubs::NPC& n) -> std::string {
            Serializer s;
            return s.Serialize(n.id)
                 + s.Serialize(n.dialogue)
                 + s.Serialize(n.hostile);
        },
        [](const std::string& data) -> std::optional<stubs::NPC> {
            Serializer s;
            size_t pos = 0;
            auto id       = s.DeserializeAt<std::string>(data, pos);
            auto dialogue = s.DeserializeAt<std::string>(data, pos);
            auto hostile  = s.DeserializeAt<bool>(data, pos);
            if (!id || !dialogue || !hostile) return std::nullopt;
            return stubs::NPC{*id, *dialogue, *hostile};
        }
    );
 
    // ── WorldChunk ────────────────────────────────────────────────────────────
    db.RegisterType<stubs::WorldChunk>(
        "WorldChunk",
        [](const stubs::WorldChunk& c) -> std::string {
            Serializer s;
            std::string result = s.Serialize(c.chunk_x)
                               + s.Serialize(c.chunk_y)
                               + s.Serialize(c.biome);
            // Encode entity list length then each entity string
            result += s.Serialize(static_cast<int>(c.entities.size()));
            for (const auto& e : c.entities) {
                result += s.Serialize(e);
            }
            return result;
        },
        [](const std::string& data) -> std::optional<stubs::WorldChunk> {
            Serializer s;
            size_t pos = 0;
            auto cx    = s.DeserializeAt<int>(data, pos);
            auto cy    = s.DeserializeAt<int>(data, pos);
            auto biome = s.DeserializeAt<std::string>(data, pos);
            auto count = s.DeserializeAt<int>(data, pos);
            if (!cx || !cy || !biome || !count) return std::nullopt;
            stubs::WorldChunk chunk{*cx, *cy, *biome, {}};
            for (int i = 0; i < *count; ++i) {
                auto e = s.DeserializeAt<std::string>(data, pos);
                if (!e) return std::nullopt;
                chunk.entities.push_back(*e);
            }
            return chunk;
        }
    );
 
    // ── Config ────────────────────────────────────────────────────────────────
    db.RegisterType<stubs::Config>(
        "Config",
        [](const stubs::Config& cfg) -> std::string {
            Serializer s;
            return s.Serialize(cfg.key) + s.Serialize(cfg.value);
        },
        [](const std::string& data) -> std::optional<stubs::Config> {
            Serializer s;
            size_t pos = 0;
            auto k = s.DeserializeAt<std::string>(data, pos);
            auto v = s.DeserializeAt<std::string>(data, pos);
            if (!k || !v) return std::nullopt;
            return stubs::Config{*k, *v};
        }
    );
}
 
// ═════════════════════════════════════════════════════════════════════════════
//  DEMO SECTIONS
// ═════════════════════════════════════════════════════════════════════════════
 
// ── Built-in types ────────────────────────────────────────────────────────
void DemoBuiltinTypes(Database& db) {
    std::cout << "\n── Built-in types ──────────────────────────────\n";
 
    // int
    db.Store("config:max_players", 64);
    auto mp = db.Load<int>("config:max_players");
    assert(mp.has_value() && *mp == 64);
    std::cout << "  config:max_players = " << *mp << "\n";
 
    // double
    db.Store("config:gravity", 9.81);
    auto g = db.Load<double>("config:gravity");
    assert(g.has_value());
    std::cout << "  config:gravity     = " << *g << "\n";
 
    // bool
    db.Store("config:pvp_enabled", true);
    auto pvp = db.Load<bool>("config:pvp_enabled");
    assert(pvp.has_value() && *pvp == true);
    std::cout << "  config:pvp_enabled = " << (*pvp ? "true" : "false") << "\n";
 
    // std::string
    db.Store("config:server_name", std::string("MyGameServer"));
    auto sn = db.Load<std::string>("config:server_name");
    assert(sn.has_value() && *sn == "MyGameServer");
    std::cout << "  config:server_name = " << *sn << "\n";
 
    // std::vector<int>
    std::vector<int> scores = {42, 99, 7, 1337};
    db.Store("leaderboard:top4", scores);
    auto ls = db.Load<std::vector<int>>("leaderboard:top4");
    assert(ls.has_value() && ls->size() == 4);
    std::cout << "  leaderboard:top4   = [";
    for (size_t i = 0; i < ls->size(); ++i)
        std::cout << (*ls)[i] << (i + 1 < ls->size() ? ", " : "");
    std::cout << "]\n";
}
 
// ── Custom game-object types ──────────────────────────────────────────────
void DemoCustomTypes(Database& db) {
    std::cout << "\n── Custom game/object types ────────────────────\n";
 
    // Player
    stubs::Player alice{"Alice", 85, 12, 100.5, 200.3};
    db.Store("player:alice", alice);
 
    auto loaded_alice = db.Load<stubs::Player>("player:alice");
    assert(loaded_alice.has_value());
    assert(loaded_alice->name == "Alice" && loaded_alice->health == 85);
    std::cout << "  player:alice -> name=" << loaded_alice->name
              << " hp=" << loaded_alice->health
              << " lv=" << loaded_alice->level << "\n";
 
    // NPC
    stubs::NPC shopkeeper{"npc_001", "Welcome, traveller!", false};
    db.Store("npc:npc_001", shopkeeper);
 
    auto loaded_npc = db.Load<stubs::NPC>("npc:npc_001");
    assert(loaded_npc.has_value() && !loaded_npc->hostile);
    std::cout << "  npc:npc_001  -> id=" << loaded_npc->id
              << " hostile=" << (loaded_npc->hostile ? "yes" : "no") << "\n";
 
    // WorldChunk
    stubs::WorldChunk chunk{3, 7, "forest", {"tree_42", "rock_11", "player:alice"}};
    db.Store("world:main:chunk:3:7", chunk);
 
    auto loaded_chunk = db.Load<stubs::WorldChunk>("world:main:chunk:3:7");
    assert(loaded_chunk.has_value() && loaded_chunk->biome == "forest");
    std::cout << "  world:main:chunk:3:7 -> biome=" << loaded_chunk->biome
              << " entities=" << loaded_chunk->entities.size() << "\n";
 
    // Config 
    stubs::Config cfg{"render_distance", "16"};
    db.Store("config:render_distance", cfg);
 
    auto loaded_cfg = db.Load<stubs::Config>("config:render_distance");
    assert(loaded_cfg.has_value() && loaded_cfg->value == "16");
    std::cout << "  config:render_distance -> " << loaded_cfg->value << "\n";
}
 
// ──  Update and diff compression ──────────────────────────────────────────
void DemoUpdate(Database& db) {
    std::cout << "\n──  Update ──────────────────────────────────────\n";
 
    // Update a player's health and position after combat
    auto before = db.Load<stubs::Player>("player:alice");
    stubs::Player updated = *before;
    updated.health = 55;
    updated.x     = 105.0;
    updated.y     = 198.7;
 
    auto res = db.Update("player:alice", updated);
    assert(res.has_value());
 
    auto after = db.Load<stubs::Player>("player:alice");
    assert(after.has_value() && after->health == 55);
    std::cout << "  player:alice after combat -> hp=" << after->health
              << " pos=(" << after->x << ", " << after->y << ")\n";
 
    // Attempting Update on a non-existent key returns KeyNotFound
    auto bad = db.Update("player:ghost", updated);
    assert(!bad.has_value() && bad.error() == cse498::DatabaseError::KeyNotFound);
    std::cout << "  Update non-existent key correctly returned KeyNotFound\n";
}
 
// ──  Key querying ──────────────────────────────────────────────────────────
void DemoFindKeys(Database& db) {
    std::cout << "\n── FindKeys ────────────────────────────────────\n";
 
    // Store a few more players so we have something to search
    stubs::Player bob  {"Bob",   100, 5,  0.0, 0.0};
    stubs::Player carol{"Carol", 100, 3, 50.0, 50.0};
    db.Store("player:bob",   bob);
    db.Store("player:carol", carol);
 
    auto players = db.FindKeys("player:*");
    std::cout << "  player:*  -> " << players.size() << " keys:";
    for (const auto& k : players) std::cout << " [" << k << "]";
    std::cout << "\n";
 
    auto configs = db.FindKeys("config:*");
    std::cout << "  config:*  -> " << configs.size() << " keys\n";
 
    auto chunks = db.FindKeys("world:main:chunk:*");
    std::cout << "  world:main:chunk:* -> " << chunks.size() << " keys\n";
 
    // Single-character wildcard
    auto shortConfigs = db.FindKeys("config:???");
    std::cout << "  config:??? (3-char suffix) -> " << shortConfigs.size() << " keys\n";
 
    // Exact match (no wildcards – still works)
    auto exact = db.FindKeys("npc:npc_001");
    assert(exact.size() == 1);
    std::cout << "  Exact match 'npc:npc_001' -> found\n";
}
 
// ──  Delete and Exists ─────────────────────────────────────────────────────
void DemoDeleteExists(Database& db) {
    std::cout << "\n──  Delete / Exists ─────────────────────────────\n";
 
    db.Store("temp:session_token", std::string("abc123xyz"));
    assert(db.Exists("temp:session_token"));
    std::cout << "  temp:session_token exists: true\n";
 
    bool deleted = db.Delete("temp:session_token");
    assert(deleted);
    assert(!db.Exists("temp:session_token"));
    std::cout << "  After delete, exists: false\n";
 
    // Deleting a missing key returns false (no crash)
    bool second = db.Delete("temp:session_token");
    assert(!second);
    std::cout << "  Double-delete returns false safely\n";
}
 
// ──  Transaction rollback ──────────────────────────────────────────────────
void DemoTransaction(Database& db) {
    std::cout << "\n── Transactions ────────────────────────────────\n";
 
    int original_size = static_cast<int>(db.Size());
 
    auto begin_result = db.BeginTransaction();
    assert(begin_result.has_value());
 
    // Write several entries inside the transaction
    db.Store("tx:entry_a", std::string("alpha"));
    db.Store("tx:entry_b", std::string("beta"));
    db.Store("tx:entry_c", std::string("gamma"));
    std::cout << "  After 3 stores inside transaction, size = " << db.Size() << "\n";
 
    // Rollback – all three writes should disappear
    auto rb = db.Rollback();
    assert(rb.has_value());
    assert(static_cast<int>(db.Size()) == original_size);
    assert(!db.Exists("tx:entry_a"));
    std::cout << "  After rollback, size restored to " << original_size << " ✓\n";
 
    // Commit path
    db.BeginTransaction();
    db.Store("tx:committed", std::string("persisted"));
    db.Commit();
    assert(db.Exists("tx:committed"));
    std::cout << "  Committed entry 'tx:committed' survives ✓\n";
    db.Delete("tx:committed"); // clean up
}
 
// ── File persistence ──────────────────────────────────────────────────────
void DemoPersistence(Database& db) {
    std::cout << "\n── File persistence ────────────────────────────\n";
 
    const std::string path = "/tmp/group09_demo_snapshot.bin";
 
    auto save_result = db.SaveToFile(path);
    if (!save_result) {
        std::cout << "  [SKIP] SaveToFile failed (check /tmp permissions)\n";
        return;
    }
    std::cout << "  Saved " << db.Size() << " entries to " << path << "\n";
 
    // Load into a fresh database and verify a sample key
    cse498::Database fresh_db;
    RegisterGameTypes(fresh_db);
 
    auto load_result = fresh_db.LoadFromFile(path);
    assert(load_result.has_value());
    assert(fresh_db.Size() == db.Size());
    std::cout << "  Fresh database loaded " << fresh_db.Size() << " entries\n";
 
    // Spot-check a player
    auto alice = fresh_db.Load<stubs::Player>("player:alice");
    assert(alice.has_value() && alice->name == "Alice");
    std::cout << "  player:alice round-trips correctly -> hp=" << alice->health << "\n";
}
 
// ── Type metadata ─────────────────────────────────────────────────────────
void DemoTypeMetadata(Database& db) {
    std::cout << "\n── Type metadata ───────────────────────────────\n";
 
    for (const auto& key : {"config:max_players", "config:gravity",
                             "config:pvp_enabled", "config:server_name",
                             "player:alice"}) {
        auto type = db.GetType(key);
        if (type.has_value()) {
            std::cout << "  " << key << " -> type tag: \"" << *type << "\"\n";
        }
    }
}
 
// ── Storage size reporting ────────────────────────────────────────────────
void DemoStorageSize(Database& db) {
    std::cout << "\n── Storage sizes ───────────────────────────────\n";
 
    for (const auto& key : {"player:alice", "world:main:chunk:3:7",
                             "leaderboard:top4", "config:server_name"}) {
        auto sz = db.GetStorageSize(key);
        if (sz.has_value()) {
            std::cout << "  " << key << " -> " << *sz << " bytes on-disk\n";
        }
    }
}
 
} // anonymous namespace
 
// ═════════════════════════════════════════════════════════════════════════════
//  main
// ═════════════════════════════════════════════════════════════════════════════
 
int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  Group-09 Database Module – Integration Demo         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
 
    // ── Choose in-memory or SQLite backend ───────────────────────────────────
    bool use_sqlite = (argc >= 2 && std::string(argv[1]) == "--sqlite");
    std::unique_ptr<cse498::Database> db_ptr;
 
    if (use_sqlite) {
        const std::string db_file = "group09_demo.db";
        std::cout << "\nBackend: SQLite (" << db_file << ")\n";
        db_ptr = std::make_unique<cse498::Database>(db_file);
    } else {
        std::cout << "\nBackend: in-memory\n";
        db_ptr = std::make_unique<cse498::Database>();
    }
 
    cse498::Database& db = *db_ptr;
 
    // ── Register all game types once at startup ──────────────────────────────
    RegisterGameTypes(db);
 
    // ── Run demo sections ────────────────────────────────────────────────────
    DemoBuiltinTypes(db);
    DemoCustomTypes(db);
    DemoUpdate(db);
    DemoFindKeys(db);
    DemoDeleteExists(db);
    DemoTransaction(db);
    DemoPersistence(db);
    DemoTypeMetadata(db);
    DemoStorageSize(db);
 
    std::cout << "\n── Summary ─────────────────────────────────────────\n";
    std::cout << "  Total entries in database: " << db.Size() << "\n";
    std::cout << "\n All assertions passed – demo complete.\n\n";
 
    return 0;
}