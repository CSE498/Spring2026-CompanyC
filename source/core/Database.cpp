/**
 * @file Database.cpp
 * @author Group-9
 **/

#include "Database.hpp"
#include <iostream>

namespace cse498 {

bool Database::Exists(const std::string& key) const {
    return mStorage.count(key) > 0;
}

void Database::Clear() {
    mStorage.clear();
}

size_t Database::Size() const {
    return mStorage.size();
}

bool Database::Delete(const std::string& key) {
    if (!Exists(key)) return false;
    mStorage.erase(key);
    return true;
}

std::vector<std::string> Database::ListKeys() const {
    std::vector<std::string> keys;
    keys.reserve(mStorage.size());
    
    for (const auto& [key, _] : mStorage) {
        keys.push_back(key);
    }
    
    return keys;
}

std::expected<size_t, DatabaseError> Database::GetStorageSize(const std::string& key) const {
    if (!Exists(key)) {
        return std::unexpected(DatabaseError::KeyNotFound);
    }
    
    return mStorage.at(key).size();
}

void Database::Log(const std::string& message) const {
    if (mConfig.verbose) {
        std::cerr << message << "\n" << std::flush;
    }
}

} // namespace cse498