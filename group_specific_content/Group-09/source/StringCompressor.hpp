/**
 * @file StringCompressor.hpp
 * @author Krist Veseli
 * 
 * @brief Implements LZW compression to reduce the footprint of serialized 
 *        world states and agent logs before database storage.
 */

// LZW is a dictionary based compression algorithm
// Only a single number is needed to represent a substring
// Standard Character-Set -> The 256 ASCII code chars

#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <expected>

enum class CompressorError {
    InvalidCode,
    CorruptedHeader,
    EmptyInput
};

class StringCompressor {
private:
    // 4096 is standard for 12-bit LZW, but can be increased for larger worlds.
    static constexpr size_t MAX_DICT_SIZE = 4096;

    // Helper to re-initialize dictionary with basic ASCII (0-255)
    // uint16_t only uses 2 bytes per code (Instead of 4 or 8 with 'int')
    static void InitializeCompressionDict(std::map<std::string, uint16_t>& dict);
    static void InitializeDecompressionDict(std::map<uint16_t, std::string>& dict);

public:
    /**
     * Compresses a standard string into a vector of 16-bit unsigned integers.
     * uint16_t allows for dictionary codes up to 65,535.
     */
    static std::vector<uint16_t> Compress(const std::string& input);

    /**
     * Reconstructs the original string from the compressed vector of codes.
     */
    static std::expected<std::string, CompressorError> Decompress(const std::vector<uint16_t>& compressed);

};

