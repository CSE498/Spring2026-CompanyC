/**
 * @file StringCompressor.hpp
 * @author Krist Veseli
 * 
 * @brief Implements LZW compression to reduce the footprint of serialized 
 *        world states and agent logs before database storage. LZW is a 
 *        dictionary based compression algorithm. Only a single number is 
 *        needed to represent a substring.
 *        Standard Character-Set -> The 256 ASCII code chars
 *        Claude AI was used to help make code below & to make the
 *        function definitions.
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <expected>

namespace cse498 {

enum class CompressorError {
    EmptyInput,
    InvalidCode,
    CorruptedHeader,
    VersionIncompatibility
};

class StringCompressor {
private:
    // Prevent instantiation since all functions are static
    StringCompressor() = delete;
    StringCompressor(const StringCompressor&) = delete;
    StringCompressor& operator=(const StringCompressor&) = delete;
    
    // 4096 is standard for 12-bit LZW, but can be increased for larger worlds.
    static constexpr size_t MAX_DICT_SIZE = 4096;
    
    // Helper to re-initialize dictionary with basic ASCII (0-255)
    // uint16_t only uses 2 bytes per code (Instead of 4 or 8 with 'int')
    static void InitializeCompressionDict(std::unordered_map<std::string, uint16_t>& dict);
    static void InitializeDecompressionDict(std::unordered_map<uint16_t, std::string>& dict);

public:
    static constexpr uint8_t VERSION = 1;
    static constexpr uint8_t MAGIC_BYTE_1 = 0x4C; // 'L' for LZW
    static constexpr uint8_t MAGIC_BYTE_2 = 0x5A; // 'Z'
    
    /**
     * Compresses a standard string into a vector of 16-bit unsigned integers.
     * uint16_t allows for dictionary codes up to 65,535.
     */
    [[nodiscard]] static std::vector<uint16_t> Compress(const std::string& input);
    
    /**
     * Reconstructs the original string from the compressed vector of codes.
     */
    [[nodiscard]] static std::expected<std::string, CompressorError> Decompress(
        const std::vector<uint16_t>& compressed);
    
    /**
     * Calculates compression ratio (compressed_size / original_size)
     */
    [[nodiscard]] static double GetCompressionRatio(const std::string& original, 
                                    const std::vector<uint16_t>& compressed);
    
    /**
     * Compresses string and converts to byte vector for database storage.
     * Format: [MAGIC_BYTE_1][MAGIC_BYTE_2][VERSION][compressed_codes...]
     */
    [[nodiscard]] static std::vector<uint8_t> CompressToBytes(const std::string& input);
    
    /**
     * Decompresses from byte vector back to original string
     */
    [[nodiscard]] static std::expected<std::string, CompressorError> DecompressFromBytes(
        const std::vector<uint8_t>& data);
    
    /**
     * Checks if data has valid compression header
     */
    [[nodiscard]] static bool IsCompressed(const std::vector<uint8_t>& data);
};

} // namespace cse498