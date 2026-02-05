#include "StringDiff.hpp"
#include <functional> // for std::hash
#include <sstream>    // for std::ostringstream, std::istringstream
#include <stdexcept> //  for decode try/catch

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

//FORMAT: HASH | BASE_LEN | PREFIX_LEN | SUFFIX_LEN | REPLACEMENT
//EX:
//  Base = "Hello World" (len 11)
//  Updated = "Hello C++ World"
//  MakeDiff gives: prefix = 6, suffix = 5, replacement = "C++"
//
//EX ENCODED OUTPUT: 9876543210|11|6|5|C++
std::string StringDiff::EncodeDiff(const StringDiff::Diff& patch) {
    std::ostringstream oss;

    oss << patch.base_hash << '|';
    oss << patch.base_length << '|';
    oss << patch.prefix_length << '|';
    oss << patch.suffix_length << '|';
    oss << patch.replacement;

    return oss.str();
}




// HASH | BASE_LEN | PREFIX_LEN | SUFFIX_LEN | REPLACEMENT
// 4 seperators
std::optional<StringDiff::Diff> StringDiff::DecodeDiff(const std::string& encoded) {
    Diff patch;
    std::size_t start = 0;

    // HASH
    std::size_t sep_1 = encoded.find('|', start);
    if (sep_1 == std::string::npos) {
        return std::nullopt;
    }

    std::string hash_str = encoded.substr(start, sep_1 - start);
    std::size_t hash;    
    try {
        hash = std::stoull(hash_str);
        patch.base_hash = hash;
    } catch (...) {
        return std::nullopt;
    }

    start = sep_1 + 1;

    // BASE_LEN
    std::size_t sep_2 = encoded.find('|', start);
    if (sep_2 == std::string::npos) {
        return std::nullopt;
    }

    std::string base_len_str = encoded.substr(start, sep_2 - start);
    std::size_t base_len;
    try {
            base_len = std::stoull(base_len_str);
            patch.base_length = base_len;
    } catch (...) {
        return std::nullopt;
    }

    start = sep_2 + 1;

    // PREFIX_LEN
    std::size_t sep_3 = encoded.find('|', start);
    if (sep_3 == std::string::npos) {
        return std::nullopt;
    }

    std::string prefix_len_str = encoded.substr(start, sep_3 - start);
    std::size_t prefix_len;
    try {
        prefix_len = std::stoull(prefix_len_str);
        patch.prefix_length = prefix_len;
    } catch (...) {
        return std::nullopt;
    }

    start = sep_3 + 1;

    // SUFFIX_LEN
    std::size_t sep_4 = encoded.find('|', start);
    if (sep_4 == std::string::npos) {
        return std::nullopt;
    }

    std::string suffix_len_str = encoded.substr(start, sep_4 - start);
    std::size_t suffix_len;
    try {
        suffix_len = std::stoull(suffix_len_str);
        patch.suffix_length = suffix_len;
    } catch (...) {
        return std::nullopt;
    }

    start = sep_4 + 1;

    // REPLACEMENT
    std::string replacement = encoded.substr(start);
    patch.replacement = replacement;

    return patch;

}

} //namespace sim