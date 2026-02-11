#include "StringCompressor.hpp"
#include <expected>
#include <map>
#include <vector>
#include <string>

// Claude AI was used to help make code below

// Helper to fill the dictionary with the initial 256 ASCII characters
void StringCompressor::InitializeCompressionDict(std::map<std::string, uint16_t>& dict) {
    dict.clear();
    for (int i = 0; i < 256; i++) {
        dict[std::string(1, static_cast<char>(i))] = static_cast<uint16_t>(i);
    }
}

std::vector<uint16_t> StringCompressor::Compress(const std::string& input) {
    if (input.empty()) return {};

    std::vector<uint16_t> output;
    // Consider using unordered map in the future for better performance
    std::map<std::string, uint16_t> dict;
    InitializeCompressionDict(dict);

    uint16_t next_code = 256;
    std::string p = "";

    for (char c : input) {
        std::string pc = p + c;
        
        if (dict.contains(pc)) {
            p = pc;
        } else {
            output.push_back(dict[p]);

            if (next_code < MAX_DICT_SIZE) {
                dict[pc] = next_code++;
            }
            p = std::string(1, c);
        }
    }

    if (!p.empty()) {
        output.push_back(dict[p]);
    }

    return output;
}

// Helper to fill the dictionary with the initial 256 ASCII characters
void StringCompressor::InitializeDecompressionDict(std::map<uint16_t, std::string>& dict) {
    dict.clear();
    for (int i = 0; i < 256; i++) {
        dict[static_cast<uint16_t>(i)] = std::string(1, static_cast<char>(i));
    }
}

std::expected<std::string, CompressorError> StringCompressor::Decompress(const std::vector<uint16_t>& compressed) {
    if (compressed.empty()) {
        return std::unexpected(CompressorError::EmptyInput);
    }

    std::map<uint16_t, std::string> dict;
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
    
    // Store original size (4 bytes)
    uint32_t orig_size = input.size();
    bytes.push_back((orig_size >> 24) & 0xFF);
    bytes.push_back((orig_size >> 16) & 0xFF);
    bytes.push_back((orig_size >> 8) & 0xFF);
    bytes.push_back(orig_size & 0xFF);
    
    // Store compressed data
    for (uint16_t code : compressed) {
        bytes.push_back((code >> 8) & 0xFF);
        bytes.push_back(code & 0xFF);
    }
    
    return bytes;
}

std::expected<std::string, CompressorError> StringCompressor::DecompressFromBytes(
    const std::vector<uint8_t>& data) {
    
    if (data.size() < 9) {  // magic(2) + version(1) + size(4) + at least one code(2)
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
    
    // Read original size (now at indices 3-6)
    uint32_t orig_size = (static_cast<uint32_t>(data[3]) << 24) |
                         (static_cast<uint32_t>(data[4]) << 16) |
                         (static_cast<uint32_t>(data[5]) << 8) |
                         static_cast<uint32_t>(data[6]);
    
    // Convert bytes back to uint16_t codes (now starting at index 7)
    std::vector<uint16_t> compressed;
    for (size_t i = 7; i + 1 < data.size(); i += 2) {
        uint16_t code = (static_cast<uint16_t>(data[i]) << 8) | data[i + 1];
        compressed.push_back(code);
    }
    
    return Decompress(compressed);
}

bool StringCompressor::IsCompressed(const std::vector<uint8_t>& data) {
    if (data.size() < 9) return false;
    return data[0] == MAGIC_BYTE_1 && 
           data[1] == MAGIC_BYTE_2 && 
           data[2] == VERSION;
}