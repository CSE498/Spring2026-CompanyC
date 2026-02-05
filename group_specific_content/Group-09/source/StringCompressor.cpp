#include "StringCompressor.hpp"
#include <expected>
#include <map>
#include <vector>
#include <string>

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