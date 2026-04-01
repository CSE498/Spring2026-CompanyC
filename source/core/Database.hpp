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
#include "../tools/DataGrid.hpp"
#include "WorldPosition.hpp"
#include "Location.hpp"

#include <string>
#include <unordered_map>
#include <expected>
#include <vector>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <typeindex>

namespace cse498 {

namespace detail {
/// Type trait: true for types the Serializer handles natively (primitives + containers)
template <typename T> struct is_builtin_serializable : std::false_type {};
template <> struct is_builtin_serializable<int> : std::true_type {};
template <> struct is_builtin_serializable<double> : std::true_type {};
template <> struct is_builtin_serializable<bool> : std::true_type {};
template <> struct is_builtin_serializable<char> : std::true_type {};
template <> struct is_builtin_serializable<std::string> : std::true_type {};
template <> struct is_builtin_serializable<long long> : std::true_type {};
template <> struct is_builtin_serializable<unsigned long long> : std::true_type {};
template <> struct is_builtin_serializable<float> : std::true_type {};
template <> struct is_builtin_serializable<long> : std::true_type {};
template <> struct is_builtin_serializable<unsigned int> : std::true_type {};
template <> struct is_builtin_serializable<unsigned long> : std::true_type {};
template <typename T> struct is_builtin_serializable<std::vector<T>> : std::true_type {};
template <typename K, typename V> struct is_builtin_serializable<std::map<K,V>> : std::true_type {};
template <typename K, typename V> struct is_builtin_serializable<std::unordered_map<K,V>> : std::true_type {};
template <typename... Ts>
struct is_builtin_serializable<std::variant<Ts...>>
    : std::bool_constant<(is_builtin_serializable<Ts>::value && ...)> {};
} // namespace detail

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
    Database() {InitCoreTypes();}

    /// Constructor with config
    explicit Database(const DatabaseConfig& config);

    /// Convenience constructor
    explicit Database(const std::string& db_path);

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

    /// Register a custom type for serialization.
    /// Other teams call this to enable Store/Load of their game objects.
    template <typename T>
    void RegisterType(const std::string& type_id, std::function<std::string(const T&)> serialize_fn, std::function<std::optional<T>(const std::string&)> deserialize_fn) {
        mSerializer.RegisterType<T>(type_id, std::move(serialize_fn), std::move(deserialize_fn));
        mTypeIdMap[std::type_index(typeid(T))] = type_id;
    }

    /// Check if a custom type is registered
    [[nodiscard]] bool IsTypeRegistered(const std::string& type_id) const {
        return mSerializer.IsTypeRegistered(type_id);
    }

    /// get const ref for serializer obj
    [[nodiscard]] const Serializer& GetSerializer() const { return mSerializer; }

    /// Get the type tag for a stored key (e.g., "int", "string", "vector", "MyAgent")
    [[nodiscard]] std::expected<std::string, DatabaseError> GetType(const std::string& key) const;

    /// Begin a transaction. All writes between Begin and Commit are atomic.
    /// If the program crashes before Commit, all writes since Begin are rolled back.
    std::expected<void, DatabaseError> BeginTransaction();

    /// Commit all writes since BeginTransaction.
    std::expected<void, DatabaseError> Commit();

    /// Rollback all writes since BeginTransaction.
    std::expected<void, DatabaseError> Rollback();

    /// save in memory db to binary
    /// Format: [entry_count:uint64]([key_len:uint64][key_bytes][value_len:uint64][value_bytes])*
    std::expected<void, DatabaseError> SaveToFile(const std::string& filepath) const;

    /// load from binary to in mem db
    std::expected<void, DatabaseError> LoadFromFile(const std::string& filepath);

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
    std::unordered_map<std::string, std::string> mTypeMetadata;  // in-memory type tag storage
    std::unique_ptr<SQLiteConnection> mSqlite;
    bool mUsingSqlite = false;
    bool mInTransaction = false;

    // In-memory transaction snapshots (captured at BeginTransaction, restored on Rollback)
    std::unordered_map<std::string, std::vector<uint8_t>> mSnapshotStorage;
    std::unordered_map<std::string, std::string> mSnapshotMetadata;

    Serializer mSerializer;
    DatabaseConfig mConfig;
    mutable std::shared_mutex mMutex;
    std::unordered_map<std::type_index, std::string> mTypeIdMap;  // typeid(T) -> registered type_id

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

    static std::string DeriveTypeTag(const std::string& serialized);

    void InitSqlite();
    void InitCoreTypes();
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
std::expected<void, DatabaseError> Database::Store(const std::string& key, const T& obj) {
    Log("[Store] Serializing key: " + key);


    std::string serialized;
    if constexpr (detail::is_builtin_serializable<T>::value) {
        serialized = mSerializer.Serialize(obj);
    } else {
        auto type_it = mTypeIdMap.find(std::type_index(typeid(T)));
        if (type_it == mTypeIdMap.end()) {
            return std::unexpected(DatabaseError::SerializationFailed);
        }
        serialized = mSerializer.Serialize(type_it->second, obj);
    }

    std::string type_tag;
    if (mConfig.store_type_metadata) {
        type_tag = DeriveTypeTag(serialized);
    }

    auto final_data = EncodeSnapshot(serialized);
    if (!final_data) {
        return std::unexpected(final_data.error());
    }

    auto result = WriteEntry(key, *final_data, type_tag);
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
    std::optional<T> obj;
    if constexpr (detail::is_builtin_serializable<T>::value) {
        size_t pos = 0;
        obj = mSerializer.DeserializeAt<T>(*serialized, pos);
        
    } else {
        auto type_it = mTypeIdMap.find(std::type_index(typeid(T)));
        if (type_it != mTypeIdMap.end()) {
            obj = mSerializer.Deserialize<T>(type_it->second, *serialized);
        }
    }

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
    std::string updated_serialized;
    if constexpr (detail::is_builtin_serializable<T>::value) {
        updated_serialized = mSerializer.Serialize(obj);
    } else {
        auto type_it = mTypeIdMap.find(std::type_index(typeid(T)));
        if (type_it == mTypeIdMap.end()) {
            return std::unexpected(DatabaseError::SerializationFailed);
        }
        updated_serialized = mSerializer.Serialize(type_it->second, obj);
    }

    std::string type_tag;
    if (mConfig.store_type_metadata) {
        type_tag = DeriveTypeTag(updated_serialized);
    }

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
        auto result = WriteEntry(key, *diff_value, type_tag);

        if (!result) return std::unexpected(result.error());

    } else {
        Log("[Update] Storing full snapshot update");
        auto result = WriteEntry(key, *full_snapshot, type_tag);

        if (!result) return std::unexpected(result.error());
    }

    return {};
}

} // namespace cse498