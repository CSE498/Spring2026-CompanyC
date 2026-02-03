/**
 * @file StringDiff.cpp
 * @author Andrew Shilman
 * 
 * @brief Implementation of StringDiff for computing string diffs
 */

#include "StringDiff.hpp"
#include <functional> // for std::hash
#include <algorithm>  // for std::min
#include <sstream>    // for std::ostringstream, std::istringstream

namespace sim {

//Private helper
std::size_t StringDiff::ComputeHash(const std::string& str) {
    return std::hash<std::string>{}(str);
}



StringDiff::Diff StringDiff::MakeDiff(const std::string& base, const std::string& updated) {
    Diff patch;

    // signature info
    patch.base_hash = ComputeHash(base);
    patch.base_length = base.length();

    // prefix length
    std::size_t prefix_length = 0;
    while (prefix_length < base.length() && prefix_length < updated.length() && base[prefix_length] == updated[prefix_length]) {
        prefix_length++;
    }

    patch.prefix_length = prefix_length;


    // suffix length
    std::size_t suffix_length = 0;
    std::size_t base_remaining = base.length() - prefix_length;
    std::size_t updated_remaining = updated.length() - prefix_length;

    while (suffix_length < base_remaining && suffix_length < updated_remaining && base[base.length() - 1 - suffix_length] == updated[updated.length() - 1 - suffix_length]) {
        suffix_length++;
    }

    patch.suffix_length = suffix_length;


    //get new middle part
    std::size_t start_pos = prefix_length;
    std::size_t end_pos = updated.length() - suffix_length;
    std::size_t middle_length = end_pos - start_pos;

    patch.replacement = updated.substr(start_pos, middle_length);
    return patch;
}


std::optional<std::string> StringDiff::ApplyDiff(const std::string& base, const Diff& patch) {
    //length verification
    if (base.length() != patch.base_length) {
        return std::nullopt;
    }

    //hash verification
    if (ComputeHash(base) != patch.base_hash) {
        return std::nullopt;
    }

    //validate prefix + suffix within base length
    if (patch.prefix_length + patch.suffix_length > base.length()) {
        return std::nullopt;
    }

    std::string prefix = base.substr(0, patch.prefix_length);
    std::size_t suffix_start = base.length() - patch.suffix_length;
    std::string suffix = base.substr(suffix_start);

    std::string result = prefix + patch.replacement + suffix;
    return result;
}









}