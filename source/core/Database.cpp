/**
 * @file Database.cpp
 * @author Group-9
 **/

#include "Database.hpp"
#include "../tools/StringDiff.hpp"

#include <iostream>

namespace cse498 {

bool Database::Exists(const std::string& key) const {
    return EntryExists(key);
}

void Database::Clear() {
    if (mUsingSqlite) {
        mSqlite->ClearTable(kTableName);

    } else {
        mMemoryStorage.clear();
    }
}

size_t Database::Size() const {
    return EntryCount();
}

bool Database::Delete(const std::string& key) {
    return DeleteEntry(key);
}

std::vector<std::string> Database::ListKeys() const {
    return AllKeys();
}

std::vector<std::string> Database::FindKeys(const std::string& pattern) const {
    // No wildcard - exact match only
    if (pattern.find('*') == std::string::npos && pattern.find('?') == std::string::npos) {
        if (EntryExists(pattern)) {
            return {pattern};
        }
        return {};
    }

    // Wildcard matching - check each key
    auto keys = AllKeys();
    std::vector<std::string> matches;

    for (const auto& key : keys) {
        if (MatchesGlob(key, pattern)) {
            matches.push_back(key);
        }
    }
    return matches;
}

bool Database::MatchesGlob(const std::string& str, const std::string& pattern) const {
    size_t str_idx = 0;
    size_t pat_idx = 0;
    size_t star_idx = std::string::npos;
    size_t match_idx = 0;
    
    while (str_idx < str.size()) {
        if (pat_idx < pattern.size() && pattern[pat_idx] == '*') {
            // Found *, save position for potential backtracking
            star_idx = pat_idx;
            match_idx = str_idx;
            pat_idx++;
        } else if (pat_idx < pattern.size() && 
                   (pattern[pat_idx] == str[str_idx] || pattern[pat_idx] == '?')) {
            // Characters match or ? wildcard matches any single character
            str_idx++;
            pat_idx++;
        } else if (star_idx != std::string::npos) {
            // No match, but we have a * to backtrack to
            pat_idx = star_idx + 1;
            match_idx++;
            str_idx = match_idx;
        } else {
            // No match and no * to backtrack to
            return false;
        }
    }
    
    // Consume any remaining *s in pattern
    while (pat_idx < pattern.size() && pattern[pat_idx] == '*') {
        pat_idx++;
    }
    
    // Match succeeds if we've consumed entire pattern
    return pat_idx == pattern.size();
}

std::expected<size_t, DatabaseError> Database::GetStorageSize(const std::string& key) const {
    auto raw = ReadEntry(key);
    if (!raw) {
        return std::unexpected(raw.error());
    }
    return raw->size();
}

void Database::Log(const std::string& message) const {
    if (mConfig.verbose) {
        std::cerr << message << "\n" << std::flush;
    }
}

std::expected<std::vector<uint8_t>, DatabaseError>
Database::EncodeSnapshot(const std::string& serialized) const {
    Log("[EncodeSnapshot] Serialized size: " + std::to_string(serialized.size()) + " bytes");

    std::vector<uint8_t> final_data;

    if (mConfig.auto_compress && serialized.size() >= mConfig.compression_threshold) {
        Log("[EncodeSnapshot] Compressing snapshot");

        auto compressed = StringCompressor::CompressToBytes(serialized);
        if (compressed.empty()) {
            return std::unexpected(DatabaseError::CompressionFailed);
        }

        Log("[EncodeSnapshot] Compressed size: " + std::to_string(compressed.size()) + " bytes");

        final_data.push_back(static_cast<uint8_t>(StorageFormat::Compressed));
        final_data.insert(final_data.end(), compressed.begin(), compressed.end());
        
    } else {
        Log("[EncodeSnapshot] Storing raw snapshot");

        final_data.push_back(static_cast<uint8_t>(StorageFormat::Raw));
        final_data.insert(final_data.end(), serialized.begin(), serialized.end());
    }

    return final_data;
}

std::expected<std::vector<uint8_t>, DatabaseError>
Database::EncodeDiffValue(const std::string& base_serialized, const std::string& updated_serialized) const {
    auto patch = StringDiff::MakeDiff(base_serialized, updated_serialized);
    auto encoded_patch = StringDiff::EncodeDiff(patch);

    if (!encoded_patch) {
        return std::unexpected(DatabaseError::InvalidData);
    }

    const std::string diff_bundle = mSerializer.Serialize(base_serialized) + mSerializer.Serialize(*encoded_patch);

    std::vector<uint8_t> best_data;
    best_data.push_back(static_cast<uint8_t>(StorageFormat::DiffRaw));
    best_data.insert(best_data.end(), diff_bundle.begin(), diff_bundle.end());

    if (mConfig.auto_compress && diff_bundle.size() >= mConfig.compression_threshold) {
        auto compressed = StringCompressor::CompressToBytes(diff_bundle);

        if (!compressed.empty()) {
            std::vector<uint8_t> compressed_data;
            compressed_data.push_back(static_cast<uint8_t>(StorageFormat::DiffCompressed));
            compressed_data.insert(compressed_data.end(), compressed.begin(), compressed.end());

            if (compressed_data.size() < best_data.size()) {
                Log("[EncodeDiffValue] Storing compressed diff bundle");
                return compressed_data;
            }
        }
    }

    Log("[EncodeDiffValue] Storing raw diff bundle");
    return best_data;
}

std::expected<std::string, DatabaseError>
Database::DecodeStoredValue(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        return std::unexpected(DatabaseError::InvalidData);
    }

    const StorageFormat format = static_cast<StorageFormat>(data[0]);

    if (format == StorageFormat::Raw) {
        Log("[DecodeStoredValue] Decoding raw snapshot");
        return std::string(data.begin() + 1, data.end());
    }

    if (format == StorageFormat::Compressed) {
        Log("[DecodeStoredValue] Decoding compressed snapshot");
        std::vector<uint8_t> payload(data.begin() + 1, data.end());

        auto result = StringCompressor::DecompressFromBytes(payload);
        if (!result) {
            return std::unexpected(DatabaseError::DecompressionFailed);
        }

        return *result;
    }

    std::string diff_bundle;
    if (format == StorageFormat::DiffRaw) {
        Log("[DecodeStoredValue] Decoding raw diff bundle");
        diff_bundle.assign(data.begin() + 1, data.end());

    } else if (format == StorageFormat::DiffCompressed) {
        Log("[DecodeStoredValue] Decoding compressed diff bundle");
        std::vector<uint8_t> payload(data.begin() + 1, data.end());

        auto result = StringCompressor::DecompressFromBytes(payload);
        if (!result) {
            return std::unexpected(DatabaseError::DecompressionFailed);
        }

        diff_bundle = *result;

    } else {
        return std::unexpected(DatabaseError::InvalidData);
    }

    size_t pos = 0;
    auto base_serialized = mSerializer.DeserializeAt<std::string>(diff_bundle, pos);
    auto encoded_patch = mSerializer.DeserializeAt<std::string>(diff_bundle, pos);

    if (!base_serialized || !encoded_patch || pos != diff_bundle.size()) {
        return std::unexpected(DatabaseError::InvalidData);
    }

    auto patch = StringDiff::DecodeDiff(*encoded_patch);
    if (!patch) {
        return std::unexpected(DatabaseError::InvalidData);
    }

    auto updated_serialized = StringDiff::ApplyDiff(*base_serialized, *patch);
    if (!updated_serialized) {
        return std::unexpected(DatabaseError::InvalidData);
    }

    return *updated_serialized;
}


std::expected<void, DatabaseError> Database::WriteEntry(const std::string& key, const std::vector<uint8_t>& value, const std::string& type_tag) {
    if (mUsingSqlite) {
        auto result = mSqlite->UpsertBlob(kTableName, key, value, type_tag);
        if (!result) {
            return std::unexpected(DatabaseError::IOError);
        }

        return {};
    }

    mMemoryStorage[key] = value;
    return {};
}

std::expected<std::vector<uint8_t>, DatabaseError> Database::ReadEntry(const std::string& key) const {
    if (mUsingSqlite) {
        auto result = mSqlite->GetBlob(kTableName, key);
        if (!result) {
            if (result.error() == SQLiteError::NotFound) {
                return std::unexpected(DatabaseError::KeyNotFound);
            }
            return std::unexpected(DatabaseError::IOError);
        }
        return *result;
    }

    auto it = mMemoryStorage.find(key);
    if (it == mMemoryStorage.end()) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }
    return it->second;
}

bool Database::DeleteEntry(const std::string& key) {
    if (mUsingSqlite) {
        auto exists = mSqlite->RowExists(kTableName, key);
        if (!exists.has_value() || !*exists) return false;

        auto result = mSqlite->DeleteRow(kTableName, key);
        return result.has_value();
    }

    return mMemoryStorage.erase(key) > 0;
}

bool Database::EntryExists(const std::string& key) const {
    if (mUsingSqlite) {
        auto result = mSqlite->RowExists(kTableName, key);
        return result.has_value() && *result;
    }

    return mMemoryStorage.count(key) > 0;
}

std::vector<std::string> Database::AllKeys() const {
    if (mUsingSqlite) {
        auto result = mSqlite->GetAllKeys(kTableName);
        if (!result) return {};
        return *result;
    }

    std::vector<std::string> keys;
    keys.reserve(mMemoryStorage.size());

    for (const auto& [key, _] : mMemoryStorage) {
        keys.push_back(key);
    }
    return keys;
}

size_t Database::EntryCount() const {
    if (mUsingSqlite) {
        auto result = mSqlite->GetRowCount(kTableName);
        if (!result) return 0;
        
        return *result;
    }
    return mMemoryStorage.size();
}

} // namespace cse498