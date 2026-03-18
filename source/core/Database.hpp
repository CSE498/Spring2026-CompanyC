/**
 * @file Database.hpp
 * @author Group-9
 * @brief Key-value database
 * 
 * Simple, fast in-memory storage with automatic serialization and compression.
 * Uses naming conventions for organization.
 * 
 * Architecture:
 * - Serializer: Converts C++ objects ↔ strings
 * - StringCompressor: LZW compression for large data (>100 bytes)
 * - StringDiff: Compact update patches when replacing existing values
 * - Format byte: Explicit raw vs compressed distinction
 * 
 * Thread Safety: NOT thread-safe. Use external synchronization if accessing
 * from multiple threads (e.g., std::mutex or thread-local instances).
 * 
 * Key Naming Conventions:
 *   player:<id>                - Player data
 *   world:<name>:chunk:<x>:<y> - World chunks
 *   npc:<id>                   - NPCs
 *   config:<setting>           - Configuration
 *
 * Example:
 * @code
 * Database db;
 * db.Store("player:alice", playerObject);
 * auto player = db.Load<Player>("player:alice");
 * db.Update("player:alice", updatedPlayer);
 * 
 * // Find all players
 * auto players = db.FindKeys("player:*");
 * 
 * // Find all chunks in a world
 * auto chunks = db.FindKeys("world:main:chunk:*");
 * @endcode
 **/

#pragma once

#include "../tools/Serializer.hpp"
#include "../tools/StringCompressor.hpp"
#include "../tools/SQLiteConnection.hpp"
#include "../tools/Datum.hpp"

#include <string>
#include <unordered_map>
#include <expected>
#include <vector>
#include <cstdint>
#include <memory>
#include <shared_mutex>

namespace cse498 {

/// Error codes for Database operations
enum class DatabaseError {
    KeyNotFound,
    SerializationFailed,
    DeserializationFailed,
    CompressionFailed,
    DecompressionFailed,
    InvalidData,
    IOError,              
    TransactionFailed,    
    TypeMismatch          
};

/// Configuration for Database behavior
struct DatabaseConfig {
    size_t compression_threshold = 100;  // Min size (bytes) to trigger compression
    bool auto_compress = true;           // Enable automatic compression
    bool verbose = false;                // Enable debug logging to stderr

    std::string db_path = "";            // Empty = in-memory only 
    bool wal_mode = true;                // concurrent reads
    bool auto_flush = true;              // commit after every write. slower but faster
    bool store_type_metadata = true;     
};

/**
 * @brief High-level database for game state persistence
 * 
 * Orchestrates serialization and compression for storing C++ objects.
 */
class Database {
public:
    /// Default constructor
    Database() = default;
    
    /// Constructor with config
    explicit Database(const DatabaseConfig& config) : mConfig(config) {}
    
    ~Database() = default;

    // Prevent copying (storage contains unique data)
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /// Serialize and store an object (overwrites if key exists)
    template <typename T>
    std::expected<void, DatabaseError> Store(const std::string& key, const T& obj);

    /// Load and deserialize an object (read-only operation)
    template <typename T>
    [[nodiscard]] std::expected<T, DatabaseError> Load(const std::string& key) const;

    /// Update existing key (returns KeyNotFound if key doesn't exist)
    /// If a diff against the current value is smaller than a full replacement,
    /// the database stores a diff-backed entry instead of a full snapshot.
    template <typename T>
    std::expected<void, DatabaseError> Update(const std::string& key, const T& obj);

    /// Delete entry, returns false if key doesn't exist
    bool Delete(const std::string& key);

    /// Check if key exists
    [[nodiscard]] bool Exists(const std::string& key) const;

    /// Clear all data
    void Clear();

    /// Get number of entries
    [[nodiscard]] size_t Size() const;

    /// Get all keys
    [[nodiscard]] std::vector<std::string> ListKeys() const;

    /// Find keys matching pattern (supports * and ? wildcards)
    /// Examples: "player:*", "world:*:chunk:*", "config:???"
    [[nodiscard]] std::vector<std::string> FindKeys(const std::string& pattern) const;

    /// Get storage size in bytes for a key
    [[nodiscard]] std::expected<size_t, DatabaseError> GetStorageSize(const std::string& key) const;

    /// Update config
    void SetConfig(const DatabaseConfig& config) { mConfig = config; }

    /// Get current config
    [[nodiscard]] const DatabaseConfig& GetConfig() const { return mConfig; }

private:
    /// Storage format indicator
    enum class StorageFormat : uint8_t {
        Raw = 0,
        Compressed = 1,
        DiffRaw = 2,
        DiffCompressed = 3
    };

    static constexpr const char* kTableName = "kv_store";

    std::unordered_map<std::string, std::vector<uint8_t>> mMemoryStorage;
    std::unique_ptr<SQLiteConnection> mSqlite;  
    bool mUsingSqlite = false;

    Serializer mSerializer;
    DatabaseConfig mConfig;
    mutable std::shared_mutex mMutex;

    void Log(const std::string& message) const;
    [[nodiscard]] std::expected<std::vector<uint8_t>, DatabaseError> EncodeSnapshot(const std::string& serialized) const;
    [[nodiscard]] std::expected<std::vector<uint8_t>, DatabaseError> EncodeDiffValue(const std::string& base_serialized, const std::string& updated_serialized) const;
    [[nodiscard]] std::expected<std::string, DatabaseError> DecodeStoredValue(const std::vector<uint8_t>& data) const;
    
    /// Glob-style pattern matching (supports * and ? wildcards)
    bool MatchesGlob(const std::string& str, const std::string& pattern) const;

    std::expected<void, DatabaseError> WriteEntry(const std::string& key, const std::vector<uint8_t>& value, const std::string& type_tag);
    [[nodiscard]] std::expected<std::vector<uint8_t>, DatabaseError> ReadEntry(const std::string& key) const;
    bool DeleteEntry(const std::string& key);
    
    [[nodiscard]] bool EntryExists(const std::string& key) const;
    [[nodiscard]] std::vector<std::string> AllKeys() const;
    [[nodiscard]] size_t EntryCount() const;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
std::expected<void, DatabaseError> Database::Store(const std::string& key, const T& obj) {
    Log("[Store] Serializing key: " + key);


    std::string serialized = mSerializer.Serialize(obj);

    auto final_data = EncodeSnapshot(serialized);
    if (!final_data) {
        return std::unexpected(final_data.error());
    }

    auto result = WriteEntry(key, *final_data, "");
    if (!result) {
        return std::unexpected(result.error());
    }

    Log("[Store] Stored successfully");
    return {};
}

template <typename T>
std::expected<T, DatabaseError> Database::Load(const std::string& key) const {
    Log("[Load] Loading key: " + key);

    auto raw = ReadEntry(key);
    if (!raw) {
        return std::unexpected(raw.error());
    }

    auto serialized = DecodeStoredValue(*raw);
    if (!serialized) {
        return std::unexpected(serialized.error());
    }

    Log("[Load] Deserializing...");
    size_t pos = 0;
    auto obj = mSerializer.DeserializeAt<T>(*serialized, pos);

    if (!obj.has_value()) {
        return std::unexpected(DatabaseError::DeserializationFailed);
    }

    Log("[Load] Loaded successfully");
    return *obj;
}

template <typename T>
std::expected<void, DatabaseError> Database::Update(const std::string& key, const T& obj) {
    if (!EntryExists(key)) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }

    Log("[Update] Serializing updated value for key: " + key);
    const std::string updated_serialized = mSerializer.Serialize(obj);

    auto full_snapshot = EncodeSnapshot(updated_serialized);
    if (!full_snapshot) {
        return std::unexpected(full_snapshot.error());
    }

    auto current_raw = ReadEntry(key);
    if (!current_raw) {
        return std::unexpected(current_raw.error());
    }

    auto current_serialized = DecodeStoredValue(*current_raw);
    if (!current_serialized) {
        return std::unexpected(current_serialized.error());
    }

    auto diff_value = EncodeDiffValue(*current_serialized, updated_serialized);
    if (diff_value && diff_value->size() < full_snapshot->size()) {
        Log("[Update] Storing diff-backed update");
        auto result = WriteEntry(key, *diff_value, "");

        if (!result) return std::unexpected(result.error());

    } else {
        Log("[Update] Storing full snapshot update");
        auto result = WriteEntry(key, *full_snapshot, "");

        if (!result) return std::unexpected(result.error());
    }

    return {};
}

} // namespace cse498