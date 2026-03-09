#include "StringDiff.hpp"
#include <sstream>    // for std::ostringstream
#include <charconv>   // for std::from_chars

namespace cse498 {

//Private helper
uint64_t StringDiff::ComputeHash(const std::string& str) {

    //constants used by the 64-bit FNV-1a hash algorithm I researched.
    //keeps it deterministic, hash to verify the base string
    const uint64_t offset = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;

    uint64_t hash = offset;

    for (unsigned char byte : str) {
        hash ^= byte;   //mixes current byte into the hash
        hash *= prime; //spread the bits according to the alg
    }

    return hash;
}

// validates input, Rejects empty, partial parses like "123abc", and overflow.
bool StringDiff::TryParseU64(const char* begin, const char* end, uint64_t& out) {
    if (begin == end) return false;

    //attempt to parse
    const auto result = std::from_chars(begin, end, out);
    const char* parsed_end = result.ptr;
    const std::errc error = result.ec;

    const bool ok = (error == std::errc{});
    const bool consumed_all = (parsed_end == end);

    return ok && consumed_all;
}



StringDiff::Diff StringDiff::MakeDiff(const std::string& base, const std::string& updated) {
    Diff patch;

    // signature info
    patch.base_hash = ComputeHash(base);
    patch.base_length = base.length();

    // prefix length
    uint64_t prefix_length = 0;
    while (prefix_length < base.length() && prefix_length < updated.length() && base[prefix_length] == updated[prefix_length]) {
        prefix_length++;
    }

    patch.prefix_length = prefix_length;


    // suffix length
    uint64_t suffix_length = 0;
    uint64_t base_remaining = base.length() - prefix_length;
    uint64_t updated_remaining = updated.length() - prefix_length;

    while (suffix_length < base_remaining && suffix_length < updated_remaining && base[base.length() - 1 - suffix_length] == updated[updated.length() - 1 - suffix_length]) {
        suffix_length++;
    }

    patch.suffix_length = suffix_length;


    //get new middle part
    uint64_t start_pos = prefix_length;
    uint64_t end_pos = updated.length() - suffix_length;
    uint64_t middle_length = end_pos - start_pos;

    patch.replacement = updated.substr(start_pos, middle_length);
    return patch;
}


std::expected<std::string, DiffError> StringDiff::ApplyDiff(const std::string& base, const Diff& patch) {
    //length verification
    if (base.length() != patch.base_length) {
        return std::unexpected(DiffError::BaseLengthMismatch);
    }

    //hash verification
    if (ComputeHash(base) != patch.base_hash) {
        return std::unexpected(DiffError::BaseHashMismatch);
    }

    //validate prefix + suffix within base length 
    if (patch.prefix_length > base.length() || patch.suffix_length > base.length() - patch.prefix_length) {
        return std::unexpected(DiffError::InvalidPatchInvariant);
    }

    //validate replacement size
    if (patch.replacement.size() > MaxReplacementSize) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    //validate output size of prefix + suffix
    uint64_t prefix_plus_suffix = patch.prefix_length + patch.suffix_length;
    if (prefix_plus_suffix > MaxPatchedStringSize || patch.replacement.size() > MaxPatchedStringSize - prefix_plus_suffix) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    std::string prefix = base.substr(0, patch.prefix_length);
    uint64_t suffix_start = base.length() - patch.suffix_length;
    std::string suffix = base.substr(suffix_start);

    std::string result = prefix + patch.replacement + suffix;
    return result;
}

//FORMAT: HASH | BASE_LEN | PREFIX_LEN | SUFFIX_LEN | REP_LEN | REPLACEMENT
//EX:
//  Base = "Hello World" (len 11)
//  Updated = "Hello C++ World"
//  MakeDiff gives: prefix = 6, suffix = 5, replacement = "C++ "
//
//EX ENCODED OUTPUT: 9876543210|11|6|5|4|C++
std::expected<std::string, DiffError> StringDiff::EncodeDiff(const StringDiff::Diff& patch) {
    if (patch.replacement.size() > MaxReplacementSize) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    std::ostringstream oss;

    oss << patch.base_hash << '|';
    oss << patch.base_length << '|';
    oss << patch.prefix_length << '|';
    oss << patch.suffix_length << '|';
    oss << patch.replacement.size() << '|';
    oss << patch.replacement;

    std::string encoded = oss.str();

    if (encoded.size() > MaxEncodedDiffSize) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    return encoded;
}




// HASH | BASE_LEN | PREFIX_LEN | SUFFIX_LEN | REP_LEN | REPLACEMENT
// 5 separators
std::expected<StringDiff::Diff, DiffError> StringDiff::DecodeDiff(const std::string& encoded) {
    if (encoded.size() > MaxEncodedDiffSize) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    Diff patch;
    uint64_t start = 0;

    // HASH
    uint64_t sep_1 = encoded.find('|', start);
    if (sep_1 == std::string::npos) {
        return std::unexpected(DiffError::MalformedEncoding);
    }

    if (!TryParseU64(encoded.data() + start, encoded.data() + sep_1, patch.base_hash)) {
        return std::unexpected(DiffError::InvalidNumberField);
    }

    start = sep_1 + 1;

    // BASE_LEN
    uint64_t sep_2 = encoded.find('|', start);
    if (sep_2 == std::string::npos) {
        return std::unexpected(DiffError::MalformedEncoding);
    }

    if (!TryParseU64(encoded.data() + start, encoded.data() + sep_2, patch.base_length)) {
        return std::unexpected(DiffError::InvalidNumberField);
    }

    start = sep_2 + 1;

    // PREFIX_LEN
    uint64_t sep_3 = encoded.find('|', start);
    if (sep_3 == std::string::npos) {
        return std::unexpected(DiffError::MalformedEncoding);
    }

    if (!TryParseU64(encoded.data() + start, encoded.data() + sep_3, patch.prefix_length)) {
        return std::unexpected(DiffError::InvalidNumberField);
    }

    start = sep_3 + 1;

    // SUFFIX_LEN
    uint64_t sep_4 = encoded.find('|', start);
    if (sep_4 == std::string::npos) {
        return std::unexpected(DiffError::MalformedEncoding);
    }

    if (!TryParseU64(encoded.data() + start, encoded.data() + sep_4, patch.suffix_length)) {
        return std::unexpected(DiffError::InvalidNumberField);
    }

    //validate prefix/suffix
    if (patch.prefix_length > patch.base_length || patch.suffix_length > patch.base_length - patch.prefix_length) {
        return std::unexpected(DiffError::InvalidPatchInvariant);
    }

    start = sep_4 + 1;

    // REP_LEN
    uint64_t sep_5 = encoded.find('|', start);
    if (sep_5 == std::string::npos) {
        return std::unexpected(DiffError::MalformedEncoding);
    }

    uint64_t rep_len;
    if (!TryParseU64(encoded.data() + start, encoded.data() + sep_5, rep_len)) {
        return std::unexpected(DiffError::InvalidNumberField);
    }

    if (rep_len > MaxReplacementSize) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    //validate output size , prefix + suffix
    uint64_t prefix_plus_suffix = patch.prefix_length + patch.suffix_length;
    if (prefix_plus_suffix > MaxPatchedStringSize || rep_len > MaxPatchedStringSize - prefix_plus_suffix) {
        return std::unexpected(DiffError::PatchTooLarge);
    }

    start = sep_5 + 1;

    if (rep_len != encoded.size() - start) {
        return std::unexpected(DiffError::ReplacementLengthMismatch);
    }

    patch.replacement = encoded.substr(start, rep_len);
    return patch;
}

} //namespace cse498