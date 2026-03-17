/**
 * @file Database.cpp
 * @brief Test suite for the Database class
 * @author Group 9
 */

#include "catch2/catch.hpp"
#include "../../source/core/Database.hpp"
#include "../../source/tools/Datum.hpp"

using namespace cse498;

// Claude AI was used to help writing test cases & thinking of edge cases.

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