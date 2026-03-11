/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simple database class for serializing,
 *  compressing and storing, as well as loading game states back from memory.
 * @note Status: PROPOSAL
 * @author Group-9
 **/

#pragma once

#include "../tools/Serializer.hpp"
#include "../tools/StringCompressor.hpp"
#include "../tools/StringDiff.hpp"
#include "../tools/Datum.hpp"
#include "../tools/DataGrid.hpp"

#include <string>
#include <unordered_map>
#include <optional>

namespace cse498 {
    class Database {
    public:

        //Constructors + Destructor

        Database() = default;
        explicit Database(const std::string & filepath);
        ~Database() = default;

        /// Serialize, diff, and compress obj, then store it under key.
        /// Overwrites any existing entry for that key.
        /// @return true on success.

        template <typename T>
        bool Store(const std::string & key, const T & obj);

        /// Load, decompress, and deserialize the object stored under key.
        /// @return The object, or std::nullopt if key does not exist.
        template <typename T>
        std::optional<T> Load(const std::string & key);

        /// Update the value stored under key.
        /// Behaves like Store() if the key exists; returns false otherwise.
        template <typename T>
        bool Update(const std::string & key, const T & obj);

        /// Delete the entry stored under key.
        /// @return false if the key does not exist.
        bool Delete(const std::string & key);

        /// Check whether an entry exists for the given key.
        bool Exists(const std::string & key) const;

        /// Clear all in-memory data.
        void Clear();

        /// Return number of stored keys.
        size_t Size() const;

    private:
        // Each key maps to a DataGrid holding the serialized/compressed Datum(s).
        std::unordered_map<std::string, DataGrid> mStorage;

        Serializer       mSerializer;
        StringCompressor mCompressor;
        StringDiff       mDiffer;

        size_t mSize;
    };
} // End namespace cse498