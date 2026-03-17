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

std::vector<std::string> Database::FindKeys(const std::string& pattern) const {
    std::vector<std::string> matches;
    
    // No wildcard - exact match only
    if (pattern.find('*') == std::string::npos && pattern.find('?') == std::string::npos) {
        if (Exists(pattern)) {
            matches.push_back(pattern);
        }
        return matches;
    }
    
    // Wildcard matching - check each key
    for (const auto& [key, _] : mStorage) {
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