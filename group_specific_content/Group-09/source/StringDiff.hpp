#pragma once

#include <string>
#include <expected>
#include <cstdint>

namespace sim {


enum class DiffError {
    BaseLengthMismatch,
    BaseHashMismatch,
    InvalidPatchInvariant,
    MalformedEncoding,
    InvalidNumberField,
    ReplacementLengthMismatch,
    UnsupportedVersion,
    PatchTooLarge
};



/**
 * @class StringDiff
 * @brief Class for computing and applying string diffs
 */
class StringDiff {

public:
    /// deleted default operator
    StringDiff() = delete;

    /**
     * @struct Diff
     * @brief Represents a string diff patch
     */
    struct Diff {
        /// Hash of original base string
        uint64_t base_hash;      

        /// Length of base string
        uint64_t base_length;    

        /// Length of the unchanged start
        uint64_t prefix_length;  

        /// Length of the unchanged end
        uint64_t suffix_length;  

        /// New data to insert into middle
        std::string replacement;    

        /// equality operator for debugging/testing later
        bool operator==(const Diff&) const = default;
    };

    /**
     * @brief Creates a patch that transforms base into updated
     * 
     * Computes common prefix and suffix between the two strings,
     * then stores only the changed middle part. Records signature for verification.
     * 
     * @param base The original string
     * @param updated The target string to transform into
     * @return Diff A patch that can reconstruct updated from base
     */
    static Diff MakeDiff(const std::string& base, const std::string& updated);


    /**
     * @brief Applies a patch to reconstruct the updated string
     *
     * Verifies that the patch matches the base string via signature,
     * then reconstructs the updated string by combining the base's
     * prefix, the replacement, and suffix
     *
     * @param base The original string the patch was created from
     * @param patch The diff to apply
     * @return std::expected<std::string, DiffError> The reconstructed string, or a DiffError on failure
     */
    static std::expected<std::string, DiffError> ApplyDiff(const std::string& base, const Diff& patch);


    /**
     * @brief Encodes a patch into a string format for storage
     *
     * Converts the Diff struct into a compact string representation that can be stored in Datum/Datagrid.
     * validates that the replacement and encoded output stay within size limits.
     *
     * @param patch The diff to encode
     * @return std::expected<std::string, DiffError> The encoded string, or PatchTooLarge if limits are exceeded
     */
    static std::expected<std::string, DiffError> EncodeDiff(const Diff& patch);



    /**
     * @brief Decodes an encoded string back into a Diff
     *
     * Parses string created by EncodeDiff and reconstructs
     * the original Diff struct. Validates format during parsing.
     *
     * @param encoded The encoded diff string
     * @return std::expected<Diff, DiffError> The reconstructed Diff, or a DiffError if malformed
     */
    static std::expected<Diff, DiffError> DecodeDiff(const std::string& encoded);

private:

    //in bytes
    static constexpr uint64_t MaxReplacementSize = 1024 * 1024;         //1 MiB
    static constexpr uint64_t MaxEncodedDiffSize = 1024 * 1024;         //1 MiB
    static constexpr uint64_t MaxPatchedStringSize = 8 * 1024 * 1024;   //8 MiB


    /**
     * @brief Helper that computes a hash signature for a string
     * 
     * @param str The string to hash
     * @param std::uint64_t The computed hash value
     */
    static std::uint64_t ComputeHash(const std::string& str);


    /**
     * @brief Helper to try and parse uint64_t
     * 
     * @param begin Pointer to start of char range
     * @param end Poitner to end of char range
     * @param out The parsed output
     * 
     * @return bool if parsed successfully
     */
    static bool TryParseU64(const char* begin, const char* end, uint64_t& out);

};
 
} //namespace sim