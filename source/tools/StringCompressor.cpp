/**
 * @file StringCompressor.cpp
 * @author Krist Veseli
 */

#include "StringCompressor.hpp"
#include <expected>
#include <unordered_map>
#include <vector>
#include <string>

// Claude AI was used to help make code below

namespace cse498 {

// Helper to fill the dictionary with the initial 256 ASCII characters
void StringCompressor::InitializeCompressionDict(std::unordered_map<std::string, uint16_t>& dict) {
    dict.clear();
    for (uint16_t i = 0; i < 256; i++) {
        dict[std::string(1, static_cast<char>(i))] = i;
    }
}

std::vector<uint16_t> StringCompressor::Compress(const std::string& input) {
    if (input.empty()) return {};

    std::vector<uint16_t> output;
    std::unordered_map<std::string, uint16_t> dict;
    InitializeCompressionDict(dict);

    uint16_t next_code = 256;
    std::string currentPattern = "";

    for (char c : input) {
        std::string pc = currentPattern + c;
        
        if (dict.contains(pc)) {
            currentPattern = pc;
        } else {
            output.push_back(dict[currentPattern]);

            if (next_code < MAX_DICT_SIZE) {
                dict[pc] = next_code++;
            }
            currentPattern = std::string(1, c);
        }
    }

    if (!currentPattern.empty()) {
        output.push_back(dict[currentPattern]);
    }

    return output;
}

// Helper to fill the dictionary with the initial 256 ASCII characters
void StringCompressor::InitializeDecompressionDict(std::unordered_map<uint16_t, std::string>& dict) {
    dict.clear();
    for (uint16_t i = 0; i < 256; i++) {
        dict[i] = std::string(1, static_cast<char>(i));
    }
}

std::expected<std::string, CompressorError> StringCompressor::Decompress(const std::vector<uint16_t>& compressed) {
    if (compressed.empty()) {
        return std::unexpected(CompressorError::EmptyInput);
    }

    std::unordered_map<uint16_t, std::string> dict;
    InitializeDecompressionDict(dict);

    uint16_t next_code = 256;
    
    // Check if the first code is valid before accessing
    if (!dict.contains(compressed[0])) {
        return std::unexpected(CompressorError::InvalidCode);
    }

    std::string prev = dict[compressed[0]];
    std::string output = prev;

    for (size_t i = 1; i < compressed.size(); ++i) {
        uint16_t k = compressed[i];
        std::string entry;

        if (dict.contains(k)) {
            entry = dict[k];
        } else if (k == next_code) {
            entry = prev + prev[0];
        } else {
            return std::unexpected(CompressorError::InvalidCode);
        }

        output += entry;

        if (next_code < MAX_DICT_SIZE) {
            dict[next_code++] = prev + entry[0];
        }

        prev = entry;
    }

    return output;
}

double StringCompressor::GetCompressionRatio(const std::string& original, 
                                const std::vector<uint16_t>& compressed) {
    if (original.empty()) return 0.0;
    size_t original_bytes = original.size();
    size_t compressed_bytes = compressed.size() * sizeof(uint16_t);
    return static_cast<double>(compressed_bytes) / original_bytes;
}

std::vector<uint8_t> StringCompressor::CompressToBytes(const std::string& input) {
    auto compressed = Compress(input);
    
    std::vector<uint8_t> bytes;
    bytes.push_back(MAGIC_BYTE_1);  // Magic number 'L'
    bytes.push_back(MAGIC_BYTE_2);  // Magic number 'Z'
    bytes.push_back(VERSION);       // Version
    
    // Store compressed data
    for (uint16_t code : compressed) {
        bytes.push_back((code >> 8) & 0xFF);
        bytes.push_back(code & 0xFF);
    }
    
    return bytes;
}

std::expected<std::string, CompressorError> StringCompressor::DecompressFromBytes(
    const std::vector<uint8_t>& data) {
    
    if (data.size() < 5) {  // magic(2) + version(1) + at least one code(2)
        return std::unexpected(CompressorError::CorruptedHeader);
    }
    
    // Check magic bytes
    if (data[0] != MAGIC_BYTE_1 || data[1] != MAGIC_BYTE_2) {
        return std::unexpected(CompressorError::CorruptedHeader);
    }
    
    // Check version
    if (data[2] != VERSION) {
        return std::unexpected(CompressorError::VersionIncompatibility);
    }
    
    // Check if remaining data has even number of bytes (pairs for uint16_t codes)
    if ((data.size() - 3) % 2 != 0) {
        return std::unexpected(CompressorError::CorruptedHeader);
    }
    
    // Convert bytes back to uint16_t codes (starting at index 3)
    std::vector<uint16_t> compressed;
    for (size_t i = 3; i + 1 < data.size(); i += 2) {
        uint16_t code = (static_cast<uint16_t>(data[i]) << 8) | data[i + 1];
        compressed.push_back(code);
    }
    
    return Decompress(compressed);
}

bool StringCompressor::IsCompressed(const std::vector<uint8_t>& data) {
    if (data.size() < 5) return false;
    return data[0] == MAGIC_BYTE_1 && 
           data[1] == MAGIC_BYTE_2 && 
           data[2] == VERSION;
}

} // namespace cse498