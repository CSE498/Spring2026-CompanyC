/**
 * @file Database.hpp
 * @author Group-9
 * @brief A simple database class for serializing,
 *  compressing and storing, as well as loading game states back from memory.
 * Uses Serializer to convert objects to/from strings -> Then uses
 * StringCompressor to compress large data -> Datum is used indirectly through
 * Serializer
 * Not currently integrated:
 * - StringDiff: Differential updates would require metadata to distinguish
 *   full vs diff storage. Planned enhancement for future optimization.
 * - DataGrid: 2D grid structure used by other groups for game logic (tiles,
 *   boards, etc.). Not needed for key-value object storage. Can be stored
 *   as a custom type if serialization is added.
 **/

#pragma once

#include "../tools/Serializer.hpp"
#include "../tools/StringCompressor.hpp"
#include "../tools/StringDiff.hpp"
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
    InvalidData,
    DiffFailed
};

/// Configuration for Database behavior
struct DatabaseConfig {
    size_t compression_threshold = 100;  // Min size to compress
    bool auto_compress = true;           // Enable compression
    bool use_diffs = true;               // Enable diff updates
    double diff_threshold = 0.5;         // Use diff if < 50% of full
    bool verbose = false;                // Debug logging
};

/**
 * @brief High-level database for game state persistence
 * 
 * Orchestrates serialization, compression, and differential updates.
 */
class Database {
public:
    /// Default constructor
    Database() = default;
    
    /// Constructor with config
    explicit Database(const DatabaseConfig& config) : mConfig(config) {}
    
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /// Serialize and store an object
    template <typename T>
    std::expected<void, DatabaseError> Store(const std::string& key, const T& obj);

    /// Load and deserialize an object
    template <typename T>
    std::expected<T, DatabaseError> Load(const std::string& key);

    /// Update using diff if beneficial, otherwise full store
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

    std::unordered_map<std::string, std::vector<uint8_t>> mStorage;  // Key -> compressed bytes
    std::unordered_map<std::string, std::string> mSerializedCache;   // For diff calculation
    Serializer mSerializer;
    DatabaseConfig mConfig;

    void Log(const std::string& message) const;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
std::expected<void, DatabaseError> Database::Store(const std::string& key, const T& obj) {
    Log("[Store] Serializing key: " + key);

    // Serialize
    std::string serialized = mSerializer.Serialize(obj);
    
    Log("[Store] Serialized size: " + std::to_string(serialized.size()) + " bytes");

    // Cache for diffs
    if (mConfig.use_diffs) {
        mSerializedCache[key] = serialized;
        Log("[Store] Cached serialized data for diff");
    }

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

    // Store
    mStorage[key] = std::move(final_data);
    Log("[Store] Stored successfully");

    return {};
}

template <typename T>
std::expected<T, DatabaseError> Database::Load(const std::string& key) {
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
    std::vector<uint8_t> payload(data.begin() + 1, data.end());
    
    std::string serialized;

    // Decompress if needed based on format byte
    if (format == StorageFormat::Compressed) {
        Log("[Load] Data is compressed, decompressing...");
        
        auto result = StringCompressor::DecompressFromBytes(payload);
        if (!result) {
            return std::unexpected(DatabaseError::DecompressionFailed);
        }
        
        serialized = *result;
        Log("[Load] Decompressed size: " + std::to_string(serialized.size()) + " bytes");
    } else if (format == StorageFormat::Raw) {
        Log("[Load] Data is not compressed");
        // Convert bytes back to string
        serialized.assign(payload.begin(), payload.end());
    } else {
        // Unknown format
        return std::unexpected(DatabaseError::InvalidData);
    }

    // Cache for diffs
    if (mConfig.use_diffs) {
        mSerializedCache[key] = serialized;
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
    Log("[Update] Updating key: " + key);

    // Check existence
    if (!Exists(key)) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }

    // *****For now, just use Store(). Diff optimization can be added later
    // with proper metadata tracking ******
    Log("[Update] Using full Store");
    return Store(key, obj);
}

} // namespace cse498