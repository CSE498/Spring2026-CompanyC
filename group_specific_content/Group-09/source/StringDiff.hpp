#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <vector>

namespace sim {

/**
 * @class StringDiff
 * @brief Class for computing and applying string diffs
 */
class StringDiff {

public:
    /**
     * @struct Diff
     * @brief Represents a string diff patch
     */
    struct Diff {
        /// Hash of original base string
        std::size_t base_hash;      

        /// Length of base string
        std::size_t base_length;    

        /// Length of the unchanged start
        std::size_t prefix_length;  

        /// Length of the unchanged end
        std::size_t suffix_length;  

        /// New data to insert into middle
        std::string replacement;    
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
     * prefix, teh replacement, and suffix
     * 
     * @param base The original string the patch was created from
     * @param patch The diff to apply
     * @return std::optional<std::string> The reconstructed string, or std::nullopt if verification failed
     */
    static std::optional<std::string> ApplyDiff(const std::string& base, const Diff& patch);


    /**
     * @brief Encodes a patch into a string format for storage
     * 
     * Converts the Diff struct into a compact string representation that can be stored in Datum/Datagrid
     * 
     * @param patch The diff to encode
     * @return std::string The encoded string representation
     */
    static std::string EncodeDiff(const Diff& patch);



    /**
     * @brief Decodes an encoded string back into a Diff
     * 
     * Parses string created by EncodeDiff and reconstructs 
     * the original Diff struct. Validates format during parsing.
     * 
     * @param encoded The encoded diff string
     * @return std::optional<Diff> The reconstructed Diff or std::nullopt if malformed
     */
    static std::optional<Diff> DecodeDiff(const std::string& encoded);

private:
    /**
     * @brief Helper that computes a hash signature for a string
     * 
     * @param str The string to hash
     * @param std::size_t The computed hash value
     */
    static std::size_t ComputeHash(const std::string& str);

};
 
} //namespace sim