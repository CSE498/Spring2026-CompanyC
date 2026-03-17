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
#include "../tools/Datum.hpp"

#include <string>
#include <unordered_map>
#include <expected>
#include <vector>
#include <cstdint>

namespace cse498 {

/// Error codes for Database operations
enum class DatabaseError {
    KeyNotFound,
    SerializationFailed,
    DeserializationFailed,
    CompressionFailed,
    DecompressionFailed,
    InvalidData
};

/// Configuration for Database behavior
struct DatabaseConfig {
    size_t compression_threshold = 100;  // Min size (bytes) to trigger compression
    bool auto_compress = true;           // Enable automatic compression
    bool verbose = false;                // Enable debug logging to stderr
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
        Compressed = 1
    };

    std::unordered_map<std::string, std::vector<uint8_t>> mStorage;  // Key → compressed bytes
    Serializer mSerializer;
    DatabaseConfig mConfig;

    void Log(const std::string& message) const;
    
    /// Glob-style pattern matching (supports * and ? wildcards)
    bool MatchesGlob(const std::string& str, const std::string& pattern) const;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
std::expected<void, DatabaseError> Database::Store(const std::string& key, const T& obj) {
    Log("[Store] Serializing key: " + key);

    // Serialize the object
    std::string serialized = mSerializer.Serialize(obj);
    
    Log("[Store] Serialized size: " + std::to_string(serialized.size()) + " bytes");

    // Prepare final storage with format header
    std::vector<uint8_t> final_data;
    
    if (mConfig.auto_compress && serialized.size() >= mConfig.compression_threshold) {
        Log("[Store] Compressing...");
        
        auto compressed = StringCompressor::CompressToBytes(serialized);
        if (compressed.empty()) {
            return std::unexpected(DatabaseError::CompressionFailed);
        }
        
        Log("[Store] Compressed size: " + std::to_string(compressed.size()) + " bytes");
        
        // Format: [Compressed byte][compressed data]
        final_data.push_back(static_cast<uint8_t>(StorageFormat::Compressed));
        final_data.insert(final_data.end(), compressed.begin(), compressed.end());
    } else {
        Log("[Store] Skipping compression (below threshold or disabled)");
        
        // Format: [Raw byte][serialized string as bytes]
        final_data.push_back(static_cast<uint8_t>(StorageFormat::Raw));
        final_data.insert(final_data.end(), serialized.begin(), serialized.end());
    }

    // Store in memory
    mStorage[key] = std::move(final_data);
    Log("[Store] Stored successfully");

    return {};
}

template <typename T>
std::expected<T, DatabaseError> Database::Load(const std::string& key) const {
    Log("[Load] Loading key: " + key);

    // Check existence
    if (!Exists(key)) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }

    const std::vector<uint8_t>& data = mStorage.at(key);
    
    // Validate minimum size (at least format byte)
    if (data.empty()) {
        return std::unexpected(DatabaseError::InvalidData);
    }
    
    // Read format byte
    StorageFormat format = static_cast<StorageFormat>(data[0]);
    
    std::string serialized;

    // Decompress if needed based on format byte
    if (format == StorageFormat::Compressed) {
        Log("[Load] Data is compressed, decompressing...");
        
        // Create payload vector for decompression
        std::vector<uint8_t> payload(data.begin() + 1, data.end());
        auto result = StringCompressor::DecompressFromBytes(payload);
        if (!result) {
            return std::unexpected(DatabaseError::DecompressionFailed);
        }
        
        serialized = *result;
        Log("[Load] Decompressed size: " + std::to_string(serialized.size()) + " bytes");
    } else if (format == StorageFormat::Raw) {
        Log("[Load] Data is not compressed");
        // Direct assignment
        serialized.assign(data.begin() + 1, data.end());
    } else {
        // Unknown format
        return std::unexpected(DatabaseError::InvalidData);
    }

    // Deserialize
    Log("[Load] Deserializing...");
    size_t pos = 0;
    auto obj = mSerializer.DeserializeAt<T>(serialized, pos);
    
    if (!obj.has_value()) {
        return std::unexpected(DatabaseError::DeserializationFailed);
    }

    Log("[Load] Loaded successfully");
    return *obj;
}

template <typename T>
std::expected<void, DatabaseError> Database::Update(const std::string& key, const T& obj) {
    // Check existence
    if (!Exists(key)) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }

    // Update is equivalent to Store with existence check
    return Store(key, obj);
}

} // namespace cse498