/**
 * @file Database.cpp
 * @brief Test suite for the Database class
 * @author Group 9
 */

#include "catch2/catch.hpp"
#include "../../source/core/Database.hpp"
#include "../../source/core/Location.hpp"
#include "../../source/tools/Datum.hpp"

using namespace cse498;

// Claude AI was used to help writing test cases & thinking of edge cases.

namespace {

std::string MakeRandomishString(size_t length) {
    const std::string alphabet =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string out;
    out.reserve(length);

    uint32_t state = 0xC0FFEEu;
    for (size_t i = 0; i < length; ++i) {
        state = state * 1664525u + 1013904223u;
        out.push_back(alphabet[state % alphabet.size()]);
    }

    return out;
}

} // namespace

// ============================================================================
// Basic Operations Tests
// ============================================================================

TEST_CASE("Database - Construction", "[database]") {
    SECTION("Default constructor") {
        Database db;
        REQUIRE(db.Size() == 0);
        REQUIRE_FALSE(db.Exists("anything"));
    }

    SECTION("Constructor with config") {
        DatabaseConfig config;
        config.verbose = true;
        config.compression_threshold = 200;
        config.auto_compress = false;

        Database db(config);
        REQUIRE(db.Size() == 0);
        REQUIRE(db.GetConfig().verbose == true);
        REQUIRE(db.GetConfig().compression_threshold == 200);
        REQUIRE(db.GetConfig().auto_compress == false);
    }
}

TEST_CASE("Database - Store and Load primitives", "[database]") {
    Database db;

    SECTION("Store and load int") {
        auto store_result = db.Store("score", 42);
        REQUIRE(store_result.has_value());
        REQUIRE(db.Exists("score"));

        auto load_result = db.Load<int>("score");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == 42);
    }

    SECTION("Store and load double") {
        auto store_result = db.Store("pi", 3.14159);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<double>("pi");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == 3.14159);
    }

    SECTION("Store and load bool") {
        auto store_result = db.Store("flag", true);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<bool>("flag");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == true);
    }

    SECTION("Store and load char") {
        auto store_result = db.Store("letter", 'A');
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<char>("letter");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == 'A');
    }

    SECTION("Store and load string") {
        std::string name = "Alice";
        auto store_result = db.Store("player_name", name);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<std::string>("player_name");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == "Alice");
    }
}

TEST_CASE("Database - Store and Load containers", "[database]") {
    Database db;

    SECTION("Store and load vector<int>") {
        std::vector<int> inventory = {1, 2, 3, 4, 5};
        auto store_result = db.Store("inventory", inventory);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<std::vector<int>>("inventory");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == inventory);
    }

    SECTION("Store and load vector<string>") {
        std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
        auto store_result = db.Store("names", names);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<std::vector<std::string>>("names");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == names);
    }

    SECTION("Store and load map") {
        std::map<std::string, int> scores = {
            {"Alice", 100},
            {"Bob", 200},
            {"Charlie", 300}
        };
        auto store_result = db.Store("scores", scores);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<std::map<std::string, int>>("scores");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == scores);
    }

    SECTION("Store and load unordered_map") {
        std::unordered_map<std::string, double> positions = {
            {"x", 10.5},
            {"y", 20.3},
            {"z", 30.7}
        };
        auto store_result = db.Store("positions", positions);
        REQUIRE(store_result.has_value());

        auto load_result = db.Load<std::unordered_map<std::string, double>>("positions");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == positions);
    }
}

TEST_CASE("Database - Exists and Delete", "[database]") {
    Database db;

    SECTION("Exists returns false for non-existent key") {
        REQUIRE_FALSE(db.Exists("missing"));
    }

    SECTION("Exists returns true after Store") {
        (void)db.Store("key", 42);
        REQUIRE(db.Exists("key"));
    }

    SECTION("Delete removes key") {
        (void)db.Store("key", 42);
        REQUIRE(db.Exists("key"));

        bool deleted = db.Delete("key");
        REQUIRE(deleted);
        REQUIRE_FALSE(db.Exists("key"));
    }

    SECTION("Delete returns false for non-existent key") {
        bool deleted = db.Delete("missing");
        REQUIRE_FALSE(deleted);
    }
}

TEST_CASE("Database - Update", "[database]") {
    Database db;

    SECTION("Update fails if key doesn't exist") {
        auto result = db.Update("missing", 42);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Update succeeds if key exists") {
        (void)db.Store("counter", 10);
        
        auto update_result = db.Update("counter", 20);
        REQUIRE(update_result.has_value());

        auto load_result = db.Load<int>("counter");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == 20);
    }

    SECTION("Update can change type (overwrites)") {
        (void)db.Store("value", 42);
        
        auto update_result = db.Update("value", std::string("hello"));
        REQUIRE(update_result.has_value());

        auto load_result = db.Load<std::string>("value");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == "hello");
    }

    SECTION("Update can store a diff-backed representation when it is smaller") {
        DatabaseConfig config;
        config.auto_compress = true;
        config.compression_threshold = 1;

        Database diff_db(config);
        Database full_db(config);

        std::string base = MakeRandomishString(240);
        std::string updated = base;
        updated[120] = updated[120] == '!' ? '?' : '!';

        REQUIRE(diff_db.Store("blob", base).has_value());
        REQUIRE(full_db.Store("blob", updated).has_value());

        auto full_store_size = full_db.GetStorageSize("blob");
        REQUIRE(full_store_size.has_value());

        auto update_result = diff_db.Update("blob", updated);
        REQUIRE(update_result.has_value());

        auto diff_store_size = diff_db.GetStorageSize("blob");
        REQUIRE(diff_store_size.has_value());
        REQUIRE(*diff_store_size < *full_store_size);

        auto load_result = diff_db.Load<std::string>("blob");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == updated);

        std::string second_update = updated;
        second_update[200] = second_update[200] == '#' ? '$' : '#';

        auto second_result = diff_db.Update("blob", second_update);
        REQUIRE(second_result.has_value());

        auto second_load = diff_db.Load<std::string>("blob");
        REQUIRE(second_load.has_value());
        REQUIRE(*second_load == second_update);
    }

    SECTION("Update falls back to a full snapshot when diff storage is larger") {
        DatabaseConfig config;
        config.auto_compress = false;

        Database updated_db(config);
        Database full_db(config);

        std::string base(180, 'A');
        std::string updated = MakeRandomishString(180);

        REQUIRE(updated_db.Store("blob", base).has_value());
        REQUIRE(full_db.Store("blob", updated).has_value());

        auto full_store_size = full_db.GetStorageSize("blob");
        REQUIRE(full_store_size.has_value());

        auto update_result = updated_db.Update("blob", updated);
        REQUIRE(update_result.has_value());

        auto updated_store_size = updated_db.GetStorageSize("blob");
        REQUIRE(updated_store_size.has_value());
        REQUIRE(*updated_store_size == *full_store_size);

        auto load_result = updated_db.Load<std::string>("blob");
        REQUIRE(load_result.has_value());
        REQUIRE(*load_result == updated);
    }
}

TEST_CASE("Database - Size and Clear", "[database]") {
    Database db;

    SECTION("Size starts at 0") {
        REQUIRE(db.Size() == 0);
    }

    SECTION("Size increases with Store") {
        (void)db.Store("key1", 1);
        REQUIRE(db.Size() == 1);

        (void)db.Store("key2", 2);
        REQUIRE(db.Size() == 2);

        (void)db.Store("key3", 3);
        REQUIRE(db.Size() == 3);
    }

    SECTION("Size doesn't change on overwrite") {
        (void)db.Store("key", 1);
        REQUIRE(db.Size() == 1);

        (void)db.Store("key", 2);  // Overwrite
        REQUIRE(db.Size() == 1);
    }

    SECTION("Size decreases with Delete") {
        (void)db.Store("key1", 1);
        (void)db.Store("key2", 2);
        REQUIRE(db.Size() == 2);

        db.Delete("key1");
        REQUIRE(db.Size() == 1);
    }

    SECTION("Clear removes all entries") {
        (void)db.Store("key1", 1);
        (void)db.Store("key2", 2);
        (void)db.Store("key3", 3);
        REQUIRE(db.Size() == 3);

        db.Clear();
        REQUIRE(db.Size() == 0);
        REQUIRE_FALSE(db.Exists("key1"));
        REQUIRE_FALSE(db.Exists("key2"));
        REQUIRE_FALSE(db.Exists("key3"));
    }
}

TEST_CASE("Database - ListKeys", "[database]") {
    Database db;

    SECTION("Empty database returns empty list") {
        auto keys = db.ListKeys();
        REQUIRE(keys.empty());
    }

    SECTION("ListKeys returns all stored keys") {
        (void)db.Store("key1", 1);
        (void)db.Store("key2", 2);
        (void)db.Store("key3", 3);

        auto keys = db.ListKeys();
        REQUIRE(keys.size() == 3);

        // Check all keys are present (order doesn't matter)
        REQUIRE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
    }
}

TEST_CASE("Database - GetStorageSize", "[database]") {
    Database db;

    SECTION("GetStorageSize returns error for non-existent key") {
        auto result = db.GetStorageSize("missing");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == DatabaseError::KeyNotFound);
    }

    SECTION("GetStorageSize returns size in bytes") {
        (void)db.Store("key", 42);
        
        auto result = db.GetStorageSize("key");
        REQUIRE(result.has_value());
        REQUIRE(*result > 0);  // Should have some data
    }
}

// ============================================================================
// Compression Tests
// ============================================================================

TEST_CASE("Database - Compression behavior", "[database][compression]") {
    SECTION("Small data is not compressed by default") {
        DatabaseConfig config;
        config.compression_threshold = 100;
        config.auto_compress = true;
        Database db(config);

        std::string small_data = "short";  // 5 bytes < 100 threshold
        (void)db.Store("small", small_data);

        auto size_result = db.GetStorageSize("small");
        REQUIRE(size_result.has_value());
        
        // Small data should be stored uncompressed (roughly same size)
        REQUIRE(*size_result < 20);  // Not compressed overhead
    }

    SECTION("Large data is compressed by default") {
        DatabaseConfig config;
        config.compression_threshold = 50;
        config.auto_compress = true;
        Database db(config);

        // Create repetitive string (compresses well)
        std::string large_data(200, 'A');  // "AAAA..." 200 bytes
        (void)db.Store("large", large_data);

        auto size_result = db.GetStorageSize("large");
        REQUIRE(size_result.has_value());
        
        // Compressed size should be much smaller than original
        REQUIRE(*size_result < large_data.size());
    }

    SECTION("Compression can be disabled") {
        DatabaseConfig config;
        config.auto_compress = false;
        Database db(config);

        std::string data(200, 'A');
        (void)db.Store("data", data);

        auto size_result = db.GetStorageSize("data");
        REQUIRE(size_result.has_value());
        
        // Should be stored uncompressed (roughly same size)
        REQUIRE(*size_result >= data.size());
    }
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_CASE("Database - Error handling", "[database][errors]") {
    Database db;

    SECTION("Load non-existent key returns error") {
        auto result = db.Load<int>("missing");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Load with wrong type returns error") {
        (void)db.Store("number", 42);
        
        // Try to load as string (wrong type)
        auto result = db.Load<std::string>("number");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == DatabaseError::DeserializationFailed);
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_CASE("Database - Edge cases", "[database][edge]") {
    Database db;

    SECTION("Empty string") {
        std::string empty = "";
        (void)db.Store("empty", empty);

        auto result = db.Load<std::string>("empty");
        REQUIRE(result.has_value());
        REQUIRE(*result == "");
    }

    SECTION("Empty vector") {
        std::vector<int> empty_vec;
        (void)db.Store("empty_vec", empty_vec);

        auto result = db.Load<std::vector<int>>("empty_vec");
        REQUIRE(result.has_value());
        REQUIRE(result->empty());
    }

    SECTION("Zero value") {
        (void)db.Store("zero", 0);

        auto result = db.Load<int>("zero");
        REQUIRE(result.has_value());
        REQUIRE(*result == 0);
    }

    SECTION("Negative numbers") {
        (void)db.Store("negative", -42);

        auto result = db.Load<int>("negative");
        REQUIRE(result.has_value());
        REQUIRE(*result == -42);
    }

    SECTION("Very large number") {
        (void)db.Store("large", 9999999);

        auto result = db.Load<int>("large");
        REQUIRE(result.has_value());
        REQUIRE(*result == 9999999);
    }

    SECTION("Special characters in string") {
        std::string special = "Hello\nWorld\t!@#$%^&*()";
        (void)db.Store("special", special);

        auto result = db.Load<std::string>("special");
        REQUIRE(result.has_value());
        REQUIRE(*result == special);
    }

    SECTION("Unicode string") {
        std::string unicode = "Hello 世界 🌍";
        (void)db.Store("unicode", unicode);

        auto result = db.Load<std::string>("unicode");
        REQUIRE(result.has_value());
        REQUIRE(*result == unicode);
    }
}

// ============================================================================
// Multiple Operations
// ============================================================================

TEST_CASE("Database - Multiple operations", "[database]") {
    Database db;

    SECTION("Store multiple different types") {
        (void)db.Store("int", 42);
        (void)db.Store("double", 3.14);
        (void)db.Store("string", std::string("hello"));
        (void)db.Store("bool", true);

        REQUIRE(db.Size() == 4);
        REQUIRE(*db.Load<int>("int") == 42);
        REQUIRE(*db.Load<double>("double") == 3.14);
        REQUIRE(*db.Load<std::string>("string") == "hello");
        REQUIRE(*db.Load<bool>("bool") == true);
    }

    SECTION("Overwrite existing key") {
        (void)db.Store("key", 100);
        REQUIRE(*db.Load<int>("key") == 100);

        (void)db.Store("key", 200);
        REQUIRE(*db.Load<int>("key") == 200);
        REQUIRE(db.Size() == 1);  // Still just one key
    }

    SECTION("Store, delete, store again") {
        (void)db.Store("key", 1);
        REQUIRE(db.Exists("key"));

        db.Delete("key");
        REQUIRE_FALSE(db.Exists("key"));

        (void)db.Store("key", 2);
        REQUIRE(db.Exists("key"));
        REQUIRE(*db.Load<int>("key") == 2);
    }
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_CASE("Database - Configuration", "[database][config]") {
    SECTION("Update configuration") {
        Database db;
        
        DatabaseConfig new_config;
        new_config.verbose = true;
        new_config.compression_threshold = 500;
        
        db.SetConfig(new_config);
        
        REQUIRE(db.GetConfig().verbose == true);
        REQUIRE(db.GetConfig().compression_threshold == 500);
    }

    SECTION("Verbose mode doesn't affect functionality") {
        DatabaseConfig config;
        config.verbose = true;
        Database db(config);

        (void)db.Store("key", 42);
        auto result = db.Load<int>("key");
        REQUIRE(result.has_value());
        REQUIRE(*result == 42);
    }
}

// ============================================================================
// FindKeys Pattern Matching Tests
// ============================================================================

TEST_CASE("Database - FindKeys pattern matching", "[database][findkeys]") {
    Database db;
    
    SECTION("FindKeys with exact match") {
        (void)db.Store("player:alice", 100);
        (void)db.Store("world:main", 200);
        
        auto keys = db.FindKeys("player:alice");
        REQUIRE(keys.size() == 1);
        REQUIRE(keys[0] == "player:alice");
    }
    
    SECTION("FindKeys with wildcard") {
        (void)db.Store("player:alice", 1);
        (void)db.Store("player:bob", 2);
        (void)db.Store("world:main", 3);
        (void)db.Store("npc:guard", 4);
        
        auto players = db.FindKeys("player:*");
        REQUIRE(players.size() == 2);
        REQUIRE(std::find(players.begin(), players.end(), "player:alice") != players.end());
        REQUIRE(std::find(players.begin(), players.end(), "player:bob") != players.end());
    }
    
    SECTION("FindKeys with no matches") {
        (void)db.Store("player:alice", 1);
        
        auto keys = db.FindKeys("npc:*");
        REQUIRE(keys.empty());
    }
    
    SECTION("FindKeys in empty database") {
        auto keys = db.FindKeys("*");
        REQUIRE(keys.empty());
    }
}


namespace {

/// Helper to generate a temp DB path and clean up 
struct TempDB {
    std::string path;

    TempDB(const std::string& name = "test_db") {
        static int counter = 0;
        path = "/tmp/cse498_" + name + "_" + std::to_string(counter++) + ".db";
        Cleanup(); 
    }

    ~TempDB() { Cleanup(); }

    void Cleanup() {
        std::remove(path.c_str());
        std::remove((path + "-wal").c_str());
        std::remove((path + "-shm").c_str());
    }
};

} // namespace

TEST_CASE("SQLite Database - Construction", "[database][sqlite]") {
    TempDB tmp("construct");

    SECTION("Construct with file path") {
        Database db(tmp.path);
        REQUIRE(db.Size() == 0);
        REQUIRE_FALSE(db.Exists("anything"));
    }

    SECTION("Construct with config") {
        DatabaseConfig config;
        config.db_path = tmp.path;
        config.wal_mode = true;

        Database db(config);
        REQUIRE(db.Size() == 0);
        REQUIRE(db.GetConfig().db_path == tmp.path);
    }
}

TEST_CASE("SQLite Database - Store and Load primitives", "[database][sqlite]") {
    TempDB tmp("primitives");
    Database db(tmp.path);

    SECTION("int") {
        REQUIRE(db.Store("score", 42).has_value());
        auto r = db.Load<int>("score");
        REQUIRE(r.has_value());
        REQUIRE(*r == 42);
    }

    SECTION("double") {
        REQUIRE(db.Store("pi", 3.14159).has_value());
        auto r = db.Load<double>("pi");
        REQUIRE(r.has_value());
        REQUIRE(*r == 3.14159);
    }

    SECTION("bool") {
        REQUIRE(db.Store("flag", true).has_value());
        auto r = db.Load<bool>("flag");
        REQUIRE(r.has_value());
        REQUIRE(*r == true);
    }

    SECTION("char") {
        REQUIRE(db.Store("letter", 'Z').has_value());
        auto r = db.Load<char>("letter");
        REQUIRE(r.has_value());
        REQUIRE(*r == 'Z');
    }

    SECTION("string") {
        std::string name = "Alice";
        REQUIRE(db.Store("name", name).has_value());
        auto r = db.Load<std::string>("name");
        REQUIRE(r.has_value());
        REQUIRE(*r == "Alice");
    }
}

TEST_CASE("SQLite Database - Store and Load containers", "[database][sqlite]") {
    TempDB tmp("containers");
    Database db(tmp.path);

    SECTION("vector<int>") {
        std::vector<int> v = {10, 20, 30};
        REQUIRE(db.Store("vec", v).has_value());
        auto r = db.Load<std::vector<int>>("vec");
        REQUIRE(r.has_value());
        REQUIRE(*r == v);
    }

    SECTION("vector<string>") {
        std::vector<std::string> v = {"Alice", "Bob"};
        REQUIRE(db.Store("names", v).has_value());
        auto r = db.Load<std::vector<std::string>>("names");
        REQUIRE(r.has_value());
        REQUIRE(*r == v);
    }

    SECTION("map<string,int>") {
        std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
        REQUIRE(db.Store("scores", m).has_value());
        auto r = db.Load<std::map<std::string, int>>("scores");
        REQUIRE(r.has_value());
        REQUIRE(*r == m);
    }

    SECTION("unordered_map<string,double>") {
        std::unordered_map<std::string, double> m = {{"x", 1.5}, {"y", 2.5}};
        REQUIRE(db.Store("pos", m).has_value());
        auto r = db.Load<std::unordered_map<std::string, double>>("pos");
        REQUIRE(r.has_value());
        REQUIRE(*r == m);
    }
}

TEST_CASE("SQLite Database - Exists and Delete", "[database][sqlite]") {
    TempDB tmp("exists_delete");
    Database db(tmp.path);

    SECTION("Exists returns false for missing key") {
        REQUIRE_FALSE(db.Exists("nope"));
    }

    SECTION("Exists returns true after Store") {
        (void)db.Store("k", 1);
        REQUIRE(db.Exists("k"));
    }

    SECTION("Delete removes key") {
        (void)db.Store("k", 1);
        REQUIRE(db.Delete("k"));
        REQUIRE_FALSE(db.Exists("k"));
    }

    SECTION("Delete returns false for missing key") {
        REQUIRE_FALSE(db.Delete("nope"));
    }
}

TEST_CASE("SQLite Database - Update", "[database][sqlite]") {
    TempDB tmp("update");
    Database db(tmp.path);

    SECTION("Update fails if key missing") {
        auto r = db.Update("missing", 99);
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Update changes value") {
        (void)db.Store("counter", 10);
        REQUIRE(db.Update("counter", 20).has_value());
        REQUIRE(*db.Load<int>("counter") == 20);
    }

    SECTION("Update with diff-backed storage") {
        DatabaseConfig config;
        config.db_path = tmp.path;
        config.auto_compress = true;
        config.compression_threshold = 1;

        Database diff_db(config);

        std::string base = MakeRandomishString(240);
        std::string updated = base;
        updated[120] = updated[120] == '!' ? '?' : '!';

        REQUIRE(diff_db.Store("blob", base).has_value());
        REQUIRE(diff_db.Update("blob", updated).has_value());

        auto loaded = diff_db.Load<std::string>("blob");
        REQUIRE(loaded.has_value());
        REQUIRE(*loaded == updated);
    }
}

TEST_CASE("SQLite Database - Size and Clear", "[database][sqlite]") {
    TempDB tmp("size_clear");
    Database db(tmp.path);

    SECTION("Size starts at 0") {
        REQUIRE(db.Size() == 0);
    }

    SECTION("Size tracks inserts") {
        (void)db.Store("a", 1);
        (void)db.Store("b", 2);
        REQUIRE(db.Size() == 2);
    }

    SECTION("Overwrite does not increase size") {
        (void)db.Store("k", 1);
        (void)db.Store("k", 2);
        REQUIRE(db.Size() == 1);
    }

    SECTION("Clear removes all entries") {
        (void)db.Store("a", 1);
        (void)db.Store("b", 2);
        db.Clear();
        REQUIRE(db.Size() == 0);
        REQUIRE_FALSE(db.Exists("a"));
    }
}

TEST_CASE("SQLite Database - ListKeys and FindKeys", "[database][sqlite]") {
    TempDB tmp("keys");
    Database db(tmp.path);

    (void)db.Store("player:alice", 1);
    (void)db.Store("player:bob", 2);
    (void)db.Store("world:main", 3);

    SECTION("ListKeys returns all keys") {
        auto keys = db.ListKeys();
        REQUIRE(keys.size() == 3);
        REQUIRE(std::find(keys.begin(), keys.end(), "player:alice") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "player:bob") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "world:main") != keys.end());
    }

    SECTION("FindKeys with wildcard") {
        auto players = db.FindKeys("player:*");
        REQUIRE(players.size() == 2);
    }

    SECTION("FindKeys exact match") {
        auto keys = db.FindKeys("world:main");
        REQUIRE(keys.size() == 1);
        REQUIRE(keys[0] == "world:main");
    }

    SECTION("FindKeys no matches") {
        auto keys = db.FindKeys("npc:*");
        REQUIRE(keys.empty());
    }
}

TEST_CASE("SQLite Database - GetStorageSize", "[database][sqlite]") {
    TempDB tmp("storage_size");
    Database db(tmp.path);

    SECTION("Error for missing key") {
        auto r = db.GetStorageSize("nope");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Returns positive size") {
        (void)db.Store("k", 42);
        auto r = db.GetStorageSize("k");
        REQUIRE(r.has_value());
        REQUIRE(*r > 0);
    }
}

TEST_CASE("SQLite Database - Compression", "[database][sqlite]") {
    TempDB tmp("compression");

    SECTION("Large repetitive data is compressed") {
        DatabaseConfig config;
        config.db_path = tmp.path;
        config.auto_compress = true;
        config.compression_threshold = 50;
        Database db(config);

        std::string big(200, 'A');
        (void)db.Store("big", big);

        auto sz = db.GetStorageSize("big");
        REQUIRE(sz.has_value());
        REQUIRE(*sz < big.size());

        // Verify round-trip
        auto loaded = db.Load<std::string>("big");
        REQUIRE(loaded.has_value());
        REQUIRE(*loaded == big);
    }
}

TEST_CASE("SQLite Database - Error handling", "[database][sqlite]") {
    TempDB tmp("errors");
    Database db(tmp.path);

    SECTION("Load missing key") {
        auto r = db.Load<int>("missing");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Load wrong type") {
        (void)db.Store("num", 42);
        auto r = db.Load<std::string>("num");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::DeserializationFailed);
    }
}

TEST_CASE("SQLite Database - Edge cases", "[database][sqlite]") {
    TempDB tmp("edge");
    Database db(tmp.path);

    SECTION("Empty string") {
        (void)db.Store("empty", std::string(""));
        auto r = db.Load<std::string>("empty");
        REQUIRE(r.has_value());
        REQUIRE(r->empty());
    }

    SECTION("Empty vector") {
        std::vector<int> v;
        (void)db.Store("ev", v);
        auto r = db.Load<std::vector<int>>("ev");
        REQUIRE(r.has_value());
        REQUIRE(r->empty());
    }

    SECTION("Zero and negative") {
        (void)db.Store("zero", 0);
        (void)db.Store("neg", -42);
        REQUIRE(*db.Load<int>("zero") == 0);
        REQUIRE(*db.Load<int>("neg") == -42);
    }

    SECTION("Special characters in string") {
        std::string special = "Hello\nWorld\t!@#$%^&*()";
        (void)db.Store("special", special);
        REQUIRE(*db.Load<std::string>("special") == special);
    }

    SECTION("Unicode string") {
        std::string unicode = "Hello 世界 🌍";
        (void)db.Store("unicode", unicode);
        REQUIRE(*db.Load<std::string>("unicode") == unicode);
    }

    SECTION("Long key name") {
        std::string long_key(500, 'k');
        (void)db.Store(long_key, 99);
        REQUIRE(db.Exists(long_key));
        REQUIRE(*db.Load<int>(long_key) == 99);
    }
}

TEST_CASE("SQLite Database - Multiple operations", "[database][sqlite]") {
    TempDB tmp("multi_ops");
    Database db(tmp.path);

    SECTION("Store multiple types") {
        (void)db.Store("i", 42);
        (void)db.Store("d", 3.14);
        (void)db.Store("s", std::string("hello"));
        (void)db.Store("b", true);

        REQUIRE(db.Size() == 4);
        REQUIRE(*db.Load<int>("i") == 42);
        REQUIRE(*db.Load<double>("d") == 3.14);
        REQUIRE(*db.Load<std::string>("s") == "hello");
        REQUIRE(*db.Load<bool>("b") == true);
    }

    SECTION("Overwrite existing key") {
        (void)db.Store("k", 100);
        (void)db.Store("k", 200);
        REQUIRE(db.Size() == 1);
        REQUIRE(*db.Load<int>("k") == 200);
    }

    SECTION("Store, delete, store again") {
        (void)db.Store("k", 1);
        db.Delete("k");
        (void)db.Store("k", 2);
        REQUIRE(*db.Load<int>("k") == 2);
    }
}

// ============================================================================
// SQLite Persistence Tests (data survives across Database instances)
// ============================================================================

TEST_CASE("SQLite Database - Persistence across instances", "[database][sqlite][persistence]") {
    TempDB tmp("persist");

    SECTION("Data survives destruction and reload") {
        {
            Database db(tmp.path);
            (void)db.Store("player:hp", 100);
            (void)db.Store("player:name", std::string("Alice"));
            (void)db.Store("player:scores", std::vector<int>{10, 20, 30});
        }  // db destroyed, SQLite connection closed

        {
            Database db2(tmp.path);
            REQUIRE(db2.Size() == 3);
            REQUIRE(*db2.Load<int>("player:hp") == 100);
            REQUIRE(*db2.Load<std::string>("player:name") == "Alice");
            REQUIRE(*db2.Load<std::vector<int>>("player:scores") == std::vector<int>{10, 20, 30});
        }
    }

    SECTION("Overwrite persists") {
        {
            Database db(tmp.path);
            (void)db.Store("val", 1);
        }
        {
            Database db2(tmp.path);
            REQUIRE(*db2.Load<int>("val") == 1);
            (void)db2.Store("val", 2);
        }
        {
            Database db3(tmp.path);
            REQUIRE(*db3.Load<int>("val") == 2);
        }
    }

    SECTION("Delete persists") {
        {
            Database db(tmp.path);
            (void)db.Store("a", 1);
            (void)db.Store("b", 2);
            db.Delete("a");
        }
        {
            Database db2(tmp.path);
            REQUIRE_FALSE(db2.Exists("a"));
            REQUIRE(db2.Exists("b"));
            REQUIRE(db2.Size() == 1);
        }
    }

    SECTION("Clear persists") {
        {
            Database db(tmp.path);
            (void)db.Store("a", 1);
            (void)db.Store("b", 2);
            db.Clear();
        }
        {
            Database db2(tmp.path);
            REQUIRE(db2.Size() == 0);
        }
    }
}

// ============================================================================
// SQLite Transaction Tests
// ============================================================================

TEST_CASE("SQLite Database - Transactions", "[database][sqlite][transactions]") {
    TempDB tmp("txn");

    SECTION("Commit makes writes visible") {
        Database db(tmp.path);
        REQUIRE(db.BeginTransaction().has_value());
        (void)db.Store("x", 10);
        (void)db.Store("y", 20);
        REQUIRE(db.Commit().has_value());

        REQUIRE(*db.Load<int>("x") == 10);
        REQUIRE(*db.Load<int>("y") == 20);
    }

    SECTION("Rollback discards writes") {
        Database db(tmp.path);
        (void)db.Store("before", 1);

        REQUIRE(db.BeginTransaction().has_value());
        (void)db.Store("during", 2);
        REQUIRE(db.Rollback().has_value());

        REQUIRE(db.Exists("before"));
        REQUIRE_FALSE(db.Exists("during"));
    }

    SECTION("Commit persists across instances") {
        {
            Database db(tmp.path);
            REQUIRE(db.BeginTransaction().has_value());
            (void)db.Store("committed", 42);
            REQUIRE(db.Commit().has_value());
        }
        {
            Database db2(tmp.path);
            REQUIRE(*db2.Load<int>("committed") == 42);
        }
    }

    SECTION("Double begin fails") {
        Database db(tmp.path);
        REQUIRE(db.BeginTransaction().has_value());
        auto r = db.BeginTransaction();
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::TransactionFailed);
    }

    SECTION("Commit without begin fails") {
        Database db(tmp.path);
        auto r = db.Commit();
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::TransactionFailed);
    }

    SECTION("Rollback without begin fails") {
        Database db(tmp.path);
        auto r = db.Rollback();
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::TransactionFailed);
    }
}

// ============================================================================
// SQLite Type Metadata Tests
// ============================================================================

TEST_CASE("SQLite Database - Type metadata (GetType)", "[database][sqlite]") {
    TempDB tmp("type_meta");
    Database db(tmp.path);

    SECTION("Primitive type tags") {
        (void)db.Store("i", 42);
        (void)db.Store("d", 3.14);
        (void)db.Store("b", true);
        (void)db.Store("c", 'X');
        (void)db.Store("s", std::string("hi"));

        REQUIRE(*db.GetType("i") == "int");
        REQUIRE(*db.GetType("d") == "double");
        REQUIRE(*db.GetType("b") == "bool");
        REQUIRE(*db.GetType("c") == "char");
        REQUIRE(*db.GetType("s") == "string");
    }

    SECTION("Container type tags") {
        (void)db.Store("v", std::vector<int>{1, 2});
        (void)db.Store("m", std::map<std::string, int>{{"a", 1}});
        (void)db.Store("u", std::unordered_map<std::string, int>{{"b", 2}});

        REQUIRE(*db.GetType("v") == "vector");
        REQUIRE(*db.GetType("m") == "map");
        REQUIRE(*db.GetType("u") == "unordered_map");
    }

    SECTION("GetType on missing key") {
        auto r = db.GetType("nope");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::KeyNotFound);
    }

    SECTION("Type metadata persists across instances") {
        (void)db.Store("val", 99);
        REQUIRE(*db.GetType("val") == "int");

        // Reopen
        Database db2(tmp.path);
        REQUIRE(*db2.GetType("val") == "int");
    }
}

// ============================================================================
// SQLite SaveToFile / LoadFromFile Tests
// ============================================================================

TEST_CASE("SQLite Database - SaveToFile and LoadFromFile", "[database][sqlite]") {
    TempDB tmp_db("savefile_db");
    std::string save_path = "/tmp/cse498_savefile_test.bin";

    SECTION("Round-trip: SQLite -> file -> new in-memory DB") {
        // Store data in SQLite-backed DB
        {
            Database db(tmp_db.path);
            (void)db.Store("a", 1);
            (void)db.Store("b", std::string("hello"));
            (void)db.Store("c", std::vector<int>{5, 6, 7});

            auto r = db.SaveToFile(save_path);
            REQUIRE(r.has_value());
        }

        // Load into a fresh in-memory DB
        Database mem_db;
        auto r = mem_db.LoadFromFile(save_path);
        REQUIRE(r.has_value());

        REQUIRE(mem_db.Size() == 3);
        REQUIRE(*mem_db.Load<int>("a") == 1);
        REQUIRE(*mem_db.Load<std::string>("b") == "hello");
        REQUIRE(*mem_db.Load<std::vector<int>>("c") == std::vector<int>{5, 6, 7});

        std::remove(save_path.c_str());
    }

    SECTION("Round-trip: in-memory -> file -> new SQLite DB") {
        Database mem_db;
        (void)mem_db.Store("x", 42);
        (void)mem_db.Store("y", std::string("world"));

        REQUIRE(mem_db.SaveToFile(save_path).has_value());

        // Load into SQLite-backed DB
        Database sqlite_db(tmp_db.path);
        REQUIRE(sqlite_db.LoadFromFile(save_path).has_value());

        REQUIRE(*sqlite_db.Load<int>("x") == 42);
        REQUIRE(*sqlite_db.Load<std::string>("y") == "world");

        std::remove(save_path.c_str());
    }

    SECTION("LoadFromFile overwrites on key collision") {
        Database db(tmp_db.path);
        (void)db.Store("k", 1);
        REQUIRE(db.SaveToFile(save_path).has_value());

        // Modify the value in-place
        (void)db.Store("k", 999);
        REQUIRE(*db.Load<int>("k") == 999);

        // Reload from file — should restore to 1
        REQUIRE(db.LoadFromFile(save_path).has_value());
        REQUIRE(*db.Load<int>("k") == 1);

        std::remove(save_path.c_str());
    }

    SECTION("SaveToFile to invalid path returns IOError") {
        Database db(tmp_db.path);
        auto r = db.SaveToFile("/nonexistent/dir/file.bin");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::IOError);
    }

    SECTION("LoadFromFile from missing file returns IOError") {
        Database db(tmp_db.path);
        auto r = db.LoadFromFile("/nonexistent/file.bin");
        REQUIRE_FALSE(r.has_value());
        REQUIRE(r.error() == DatabaseError::IOError);
    }
}

// ============================================================================
// SQLite Custom Type Registration Tests
// ============================================================================

TEST_CASE("SQLite Database - RegisterType and custom objects", "[database][sqlite]") {
    TempDB tmp("custom_type");

    struct Player {
        int id;
        std::string name;
        double health;

        bool operator==(const Player& o) const {
            return id == o.id && name == o.name && health == o.health;
        }
    };

    auto register_player = [](Database& db) {
        db.RegisterType<Player>("Player",
            [](const Player& p) {
                Serializer s;
                return s.Serialize(p.id) + s.Serialize(p.name) + s.Serialize(p.health);
            },
            [](const std::string& data) -> std::optional<Player> {
                Serializer s;
                size_t pos = 0;
                auto id = s.DeserializeAt<int>(data, pos);
                auto name = s.DeserializeAt<std::string>(data, pos);
                auto health = s.DeserializeAt<double>(data, pos);
                if (!id || !name || !health) return std::nullopt;
                return Player{*id, *name, *health};
            }
        );
    };

    SECTION("Store and Load custom type") {
        Database db(tmp.path);
        register_player(db);
        Player alice{1, "Alice", 100.0};
        REQUIRE(db.Store("player:1", alice).has_value());

        auto loaded = db.Load<Player>("player:1");
        REQUIRE(loaded.has_value());
        REQUIRE(*loaded == alice);
    }

    SECTION("Custom type with GetType") {
        Database db(tmp.path);
        register_player(db);
        Player bob{2, "Bob", 85.5};
        (void)db.Store("player:2", bob);
        REQUIRE(*db.GetType("player:2") == "Player");
    }

    SECTION("Custom type persists (re-register to load)") {
        Player charlie{3, "Charlie", 50.0};
        {
            Database db(tmp.path);
            register_player(db);
            (void)db.Store("player:3", charlie);
        }
        {
            Database db2(tmp.path);
            register_player(db2);  // must re-register type
            auto loaded = db2.Load<Player>("player:3");
            REQUIRE(loaded.has_value());
            REQUIRE(*loaded == charlie);
        }
    }

    SECTION("IsTypeRegistered") {
        Database db(tmp.path);
        register_player(db);
        REQUIRE(db.IsTypeRegistered("Player"));
        REQUIRE_FALSE(db.IsTypeRegistered("Enemy"));
    }
}

// ============================================================================
// SQLite Stress / Multi-key Tests
// ============================================================================

TEST_CASE("SQLite Database - Stress test with many keys", "[database][sqlite]") {
    TempDB tmp("stress");
    Database db(tmp.path);

    const int N = 200;
    for (int i = 0; i < N; ++i) {
        (void)db.Store("key:" + std::to_string(i), i * 10);
    }

    REQUIRE(db.Size() == N);

    for (int i = 0; i < N; ++i) {
        auto r = db.Load<int>("key:" + std::to_string(i));
        REQUIRE(r.has_value());
        REQUIRE(*r == i * 10);
    }

    auto all = db.FindKeys("key:*");
    REQUIRE(all.size() == N);
}


TEST_CASE("DataGrid: Store and Load basic grid", "[database][datagrid]") {
    Database db;

    DataGrid grid(2, 3);
    grid.Insert(0, 0, 1.0);
    grid.Insert(0, 1, 2.0);
    grid.Insert(0, 2, 3.0);
    grid.Insert(1, 0, 4.0);
    grid.Insert(1, 1, 5.0);
    grid.Insert(1, 2, 6.0);

    auto store_result = db.Store("grid:basic", grid);
    REQUIRE(store_result.has_value());

    auto loaded = db.Load<DataGrid>("grid:basic");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->NumRows() == 2);
    REQUIRE(loaded->NumCols() == 3);

    REQUIRE(loaded->At(0, 0).AsDouble() == 1.0);
    REQUIRE(loaded->At(0, 1).AsDouble() == 2.0);
    REQUIRE(loaded->At(0, 2).AsDouble() == 3.0);
    REQUIRE(loaded->At(1, 0).AsDouble() == 4.0);
    REQUIRE(loaded->At(1, 1).AsDouble() == 5.0);
    REQUIRE(loaded->At(1, 2).AsDouble() == 6.0);
}

TEST_CASE("DataGrid: Store and Load mixed types", "[database][datagrid]") {
    Database db;

    DataGrid grid(2, 2);
    grid.Insert(0, 0, 42.0);
    grid.Insert(0, 1, std::string("hello"));
    grid.Insert(1, 0, true);
    grid.Insert(1, 1, 3.14);

    auto store_result = db.Store("grid:mixed", grid);
    REQUIRE(store_result.has_value());

    auto loaded = db.Load<DataGrid>("grid:mixed");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->NumRows() == 2);
    REQUIRE(loaded->NumCols() == 2);

    REQUIRE(loaded->At(0, 0).AsDouble() == 42.0);
    REQUIRE(loaded->At(0, 1).AsString() == "hello");
    REQUIRE(loaded->At(1, 0).AsBool() == true);
    REQUIRE(loaded->At(1, 1).AsDouble() == 3.14);
}

TEST_CASE("DataGrid: Store and Load string-only grid", "[database][datagrid]") {
    Database db;

    DataGrid grid(1, 3);
    grid.Insert(0, 0, std::string("alpha"));
    grid.Insert(0, 1, std::string("beta"));
    grid.Insert(0, 2, std::string("gamma"));

    (void)db.Store("grid:strings", grid);

    auto loaded = db.Load<DataGrid>("grid:strings");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->At(0, 0).AsString() == "alpha");
    REQUIRE(loaded->At(0, 1).AsString() == "beta");
    REQUIRE(loaded->At(0, 2).AsString() == "gamma");
}

TEST_CASE("DataGrid: Store and Load bool-only grid", "[database][datagrid]") {
    Database db;

    DataGrid grid(2, 2);
    grid.Insert(0, 0, true);
    grid.Insert(0, 1, false);
    grid.Insert(1, 0, false);
    grid.Insert(1, 1, true);

    (void)db.Store("grid:bools", grid);

    auto loaded = db.Load<DataGrid>("grid:bools");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->At(0, 0).AsBool() == true);
    REQUIRE(loaded->At(0, 1).AsBool() == false);
    REQUIRE(loaded->At(1, 0).AsBool() == false);
    REQUIRE(loaded->At(1, 1).AsBool() == true);
}

TEST_CASE("DataGrid: Update existing grid", "[database][datagrid]") {
    Database db;

    DataGrid grid1(1, 2);
    grid1.Insert(0, 0, 1.0);
    grid1.Insert(0, 1, 2.0);
    (void)db.Store("grid:update", grid1);

    DataGrid grid2(1, 2);
    grid2.Insert(0, 0, 10.0);
    grid2.Insert(0, 1, 20.0);
    auto update_result = db.Update("grid:update", grid2);
    REQUIRE(update_result.has_value());

    auto loaded = db.Load<DataGrid>("grid:update");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->At(0, 0).AsDouble() == 10.0);
    REQUIRE(loaded->At(0, 1).AsDouble() == 20.0);
}

TEST_CASE("DataGrid: Type metadata is DataGrid", "[database][datagrid]") {
    Database db;

    DataGrid grid(1, 1);
    grid.Insert(0, 0, 99.0);
    (void)db.Store("grid:meta", grid);

    auto type = db.GetType("grid:meta");
    REQUIRE(type.has_value());
    REQUIRE(*type == "DataGrid");
}

TEST_CASE("DataGrid: DataGrid type is registered by default", "[database][datagrid]") {
    Database db;
    REQUIRE(db.IsTypeRegistered("DataGrid"));
}

TEST_CASE("DataGrid: Store and Load grid built with Append", "[database][datagrid]") {
    Database db;

    DataGrid grid(0, 3);
    grid.Append({Datum(1.0), Datum(std::string("two")), Datum(true)});
    grid.Append({Datum(4.0), Datum(std::string("five")), Datum(false)});

    (void)db.Store("grid:appended", grid);

    auto loaded = db.Load<DataGrid>("grid:appended");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->NumRows() == 2);
    REQUIRE(loaded->NumCols() == 3);
    REQUIRE(loaded->At(0, 0).AsDouble() == 1.0);
    REQUIRE(loaded->At(0, 1).AsString() == "two");
    REQUIRE(loaded->At(0, 2).AsBool() == true);
    REQUIRE(loaded->At(1, 0).AsDouble() == 4.0);
    REQUIRE(loaded->At(1, 1).AsString() == "five");
    REQUIRE(loaded->At(1, 2).AsBool() == false);
}

TEST_CASE("DataGrid: SQLite db Store and Load", "[database][datagrid][sqlite]") {
    TempDB tmp("test_datagrid_sqlite");
    Database db(tmp.path);

    DataGrid grid(2, 2);
    grid.Insert(0, 0, std::string("cell00"));
    grid.Insert(0, 1, 3.14);
    grid.Insert(1, 0, true);
    grid.Insert(1, 1, std::string("cell11"));

    (void)db.Store("grid:sqlite", grid);

    auto loaded = db.Load<DataGrid>("grid:sqlite");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->NumRows() == 2);
    REQUIRE(loaded->NumCols() == 2);
    REQUIRE(loaded->At(0, 0).AsString() == "cell00");
    REQUIRE(loaded->At(0, 1).AsDouble() == Approx(3.14));
    REQUIRE(loaded->At(1, 0).AsBool() == true);
    REQUIRE(loaded->At(1, 1).AsString() == "cell11");
}


// WorldPosition Serialization

TEST_CASE("WorldPosition: Store and Load basic position", "[database][worldposition]") {
    Database db;

    WorldPosition pos(3.5, 7.25);
    auto store_result = db.Store("pos:basic", pos);
    REQUIRE(store_result.has_value());

    auto loaded = db.Load<WorldPosition>("pos:basic");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == 3.5);
    REQUIRE(loaded->Y() == 7.25);
}

TEST_CASE("WorldPosition: Store and Load origin", "[database][worldposition]") {
    Database db;

    WorldPosition pos(0.0, 0.0);
    (void)db.Store("pos:origin", pos);

    auto loaded = db.Load<WorldPosition>("pos:origin");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == 0.0);
    REQUIRE(loaded->Y() == 0.0);
}

TEST_CASE("WorldPosition: Store and Load negative coordinates", "[database][worldposition]") {
    Database db;

    WorldPosition pos(-10.5, -20.75);
    (void)db.Store("pos:negative", pos);

    auto loaded = db.Load<WorldPosition>("pos:negative");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == -10.5);
    REQUIRE(loaded->Y() == -20.75);
}

TEST_CASE("WorldPosition: Store and Load large coordinates", "[database][worldposition]") {
    Database db;

    WorldPosition pos(999999.123, 888888.456);
    (void)db.Store("pos:large", pos);

    auto loaded = db.Load<WorldPosition>("pos:large");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == Approx(999999.123));
    REQUIRE(loaded->Y() == Approx(888888.456));
}

TEST_CASE("WorldPosition: Update existing position", "[database][worldposition]") {
    Database db;

    WorldPosition pos1(1.0, 2.0);
    (void)db.Store("pos:update", pos1);

    WorldPosition pos2(10.0, 20.0);
    auto update_result = db.Update("pos:update", pos2);
    REQUIRE(update_result.has_value());

    auto loaded = db.Load<WorldPosition>("pos:update");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == 10.0);
    REQUIRE(loaded->Y() == 20.0);
}

TEST_CASE("WorldPosition: Type metadata is WorldPosition", "[database][worldposition]") {
    Database db;

    WorldPosition pos(1.0, 2.0);
    (void)db.Store("pos:meta", pos);

    auto type = db.GetType("pos:meta");
    REQUIRE(type.has_value());
    REQUIRE(*type == "WorldPosition");
}

TEST_CASE("WorldPosition: Type is registered by default", "[database][worldposition]") {
    Database db;
    REQUIRE(db.IsTypeRegistered("WorldPosition"));
}

TEST_CASE("WorldPosition: Multiple positions stored and loaded independently", "[database][worldposition]") {
    Database db;

    WorldPosition p1(1.0, 2.0);
    WorldPosition p2(3.0, 4.0);
    WorldPosition p3(5.0, 6.0);

    (void)db.Store("pos:a", p1);
    (void)db.Store("pos:b", p2);
    (void)db.Store("pos:c", p3);

    auto l1 = db.Load<WorldPosition>("pos:a");
    auto l2 = db.Load<WorldPosition>("pos:b");
    auto l3 = db.Load<WorldPosition>("pos:c");

    REQUIRE(l1.has_value());
    REQUIRE(l2.has_value());
    REQUIRE(l3.has_value());

    REQUIRE(*l1 == p1);
    REQUIRE(*l2 == p2);
    REQUIRE(*l3 == p3);
}

TEST_CASE("WorldPosition: SQLite-backed Store and Load", "[database][worldposition][sqlite]") {
    TempDB tmp("test_worldpos_sqlite");
    Database db(tmp.path);

    WorldPosition pos(42.5, 99.75);
    (void)db.Store("pos:sqlite", pos);

    auto loaded = db.Load<WorldPosition>("pos:sqlite");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->X() == 42.5);
    REQUIRE(loaded->Y() == 99.75);
}

TEST_CASE("WorldPosition: SQLite persistence across instances", "[database][worldposition][sqlite]") {
    TempDB tmp("test_worldpos_persist");

    {
        Database db(tmp.path);
        (void)db.Store("pos:persist", WorldPosition(11.1, 22.2));
    }

    {
        Database db(tmp.path);
        auto loaded = db.Load<WorldPosition>("pos:persist");
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->X() == Approx(11.1));
        REQUIRE(loaded->Y() == Approx(22.2));
    }
}

TEST_CASE("WorldPosition: FindKeys with position pattern", "[database][worldposition]") {
    Database db;

    (void)db.Store("pos:player:1", WorldPosition(1.0, 1.0));
    (void)db.Store("pos:player:2", WorldPosition(2.0, 2.0));
    (void)db.Store("pos:npc:1", WorldPosition(5.0, 5.0));

    auto player_positions = db.FindKeys("pos:player:*");
    REQUIRE(player_positions.size() == 2);

    auto all_positions = db.FindKeys("pos:*");
    REQUIRE(all_positions.size() == 3);
}

// Location Serialization 

TEST_CASE("Location: Store and Load WorldPosition variant", "[database][location]") {
    Database db;

    Location loc(WorldPosition(3.5, 7.25));
    auto store_result = db.Store("loc:wp", loc);
    REQUIRE(store_result.has_value());

    auto loaded = db.Load<Location>("loc:wp");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->IsPosition());
    REQUIRE(loaded->AsWorldPosition().X() == 3.5);
    REQUIRE(loaded->AsWorldPosition().Y() == 7.25);
}

TEST_CASE("Location: Store and Load ItemID variant", "[database][location]") {
    Database db;

    Location loc(ItemID{42});
    (void)db.Store("loc:item", loc);

    auto loaded = db.Load<Location>("loc:item");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->IsItemID());
    REQUIRE(loaded->AsItemID() == 42);
}

TEST_CASE("Location: Store and Load AgentID variant", "[database][location]") {
    Database db;

    Location loc(AgentID{99});
    (void)db.Store("loc:agent", loc);

    auto loaded = db.Load<Location>("loc:agent");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->IsAgentID());
    REQUIRE(loaded->AsAgentID() == 99);
}

TEST_CASE("Location: Update from WorldPosition to AgentID", "[database][location]") {
    Database db;

    Location loc1(WorldPosition(1.0, 2.0));
    (void)db.Store("loc:update", loc1);

    Location loc2(AgentID{55});
    auto update_result = db.Update("loc:update", loc2);
    REQUIRE(update_result.has_value());

    auto loaded = db.Load<Location>("loc:update");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->IsAgentID());
    REQUIRE(loaded->AsAgentID() == 55);
}

TEST_CASE("Location: Update from ItemID to WorldPosition", "[database][location]") {
    Database db;

    Location loc1(ItemID{10});
    (void)db.Store("loc:update2", loc1);

    Location loc2(WorldPosition(50.5, 60.5));
    auto update_result = db.Update("loc:update2", loc2);
    REQUIRE(update_result.has_value());

    auto loaded = db.Load<Location>("loc:update2");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->IsPosition());
    REQUIRE(loaded->AsWorldPosition().X() == 50.5);
    REQUIRE(loaded->AsWorldPosition().Y() == 60.5);
}

TEST_CASE("Location: Type metadata is Location", "[database][location]") {
    Database db;

    Location loc(WorldPosition(1.0, 2.0));
    (void)db.Store("loc:meta", loc);

    auto type = db.GetType("loc:meta");
    REQUIRE(type.has_value());
    REQUIRE(*type == "Location");
}

TEST_CASE("Location: Type is registered by default", "[database][location]") {
    Database db;
    REQUIRE(db.IsTypeRegistered("Location"));
}

TEST_CASE("Location: Large IDs round-trip correctly", "[database][location]") {
    Database db;

    Location loc_item(ItemID{999999999});
    Location loc_agent(AgentID{123456789});

    (void)db.Store("loc:bigitem", loc_item);
    (void)db.Store("loc:bigagent", loc_agent);

    auto li = db.Load<Location>("loc:bigitem");
    auto la = db.Load<Location>("loc:bigagent");

    REQUIRE(li.has_value());
    REQUIRE(li->IsItemID());
    REQUIRE(li->AsItemID() == 999999999);

    REQUIRE(la.has_value());
    REQUIRE(la->IsAgentID());
    REQUIRE(la->AsAgentID() == 123456789);
}

TEST_CASE("Location: Multiple variant types stored independently", "[database][location]") {
    Database db;

    (void)db.Store("loc:a", Location(WorldPosition(1.0, 2.0)));
    (void)db.Store("loc:b", Location(ItemID{10}));
    (void)db.Store("loc:c", Location(AgentID{20}));

    auto a = db.Load<Location>("loc:a");
    auto b = db.Load<Location>("loc:b");
    auto c = db.Load<Location>("loc:c");

    REQUIRE(a->IsPosition());
    REQUIRE(b->IsItemID());
    REQUIRE(c->IsAgentID());

    REQUIRE(a->AsWorldPosition().X() == 1.0);
    REQUIRE(b->AsItemID() == 10);
    REQUIRE(c->AsAgentID() == 20);
}

TEST_CASE("Location: SQLite-backed Store and Load all variants", "[database][location][sqlite]") {
    TempDB tmp("test_location_sqlite");
    Database db(tmp.path);

    (void)db.Store("loc:wp", Location(WorldPosition(5.5, 6.6)));
    (void)db.Store("loc:item", Location(ItemID{77}));
    (void)db.Store("loc:agent", Location(AgentID{88}));

    auto wp = db.Load<Location>("loc:wp");
    auto item = db.Load<Location>("loc:item");
    auto agent = db.Load<Location>("loc:agent");

    REQUIRE(wp.has_value());
    REQUIRE(wp->IsPosition());
    REQUIRE(wp->AsWorldPosition().X() == Approx(5.5));
    REQUIRE(wp->AsWorldPosition().Y() == Approx(6.6));

    REQUIRE(item.has_value());
    REQUIRE(item->IsItemID());
    REQUIRE(item->AsItemID() == 77);

    REQUIRE(agent.has_value());
    REQUIRE(agent->IsAgentID());
    REQUIRE(agent->AsAgentID() == 88);
}

TEST_CASE("Location: SQLite persistence across instances", "[database][location][sqlite]") {
    TempDB tmp("test_location_persist");

    {
        Database db(tmp.path);
        (void)db.Store("loc:persist", Location(ItemID{333}));
    }

    {
        Database db(tmp.path);
        auto loaded = db.Load<Location>("loc:persist");
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->IsItemID());
        REQUIRE(loaded->AsItemID() == 333);
    }
}
