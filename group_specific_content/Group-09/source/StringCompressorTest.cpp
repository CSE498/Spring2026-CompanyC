/**
 * @file StringCompressorTest.cpp
 * @author Krist Veseli
 */

// Claude AI was used to help make the test cases below

// The test cases below are being compiled with:
// g++ -std=c++23 \
    -I/opt/homebrew/opt/catch2/include \
    -L/opt/homebrew/opt/catch2/lib \
    StringCompressorTest.cpp StringCompressor.cpp \
    -o test_compressor \
    -lCatch2Main -lCatch2

#include <catch2/catch_test_macros.hpp>
#include "StringCompressor.hpp"

using namespace cse498;

TEST_CASE("Basic compression and decompression", "[compression]") {
    std::string original = "TOBEORNOTTOBEORTOBEORNOT";
    auto compressed = StringCompressor::Compress(original);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == original);
}

TEST_CASE("Empty input handling", "[compression][error]") {
    SECTION("Compressing empty string") {
        std::string empty = "";
        auto compressed = StringCompressor::Compress(empty);
        REQUIRE(compressed.empty());
    }
    
    SECTION("Decompressing empty vector") {
        std::vector<uint16_t> empty;
        auto result = StringCompressor::Decompress(empty);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == CompressorError::EmptyInput);
    }
}

TEST_CASE("Single character compression", "[compression]") {
    std::string single = "A";
    auto compressed = StringCompressor::Compress(single);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == single);
}

TEST_CASE("Repeated patterns compress efficiently", "[compression]") {
    std::string repeated = "ababababababababab";
    auto compressed = StringCompressor::Compress(repeated);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == repeated);
    // LZW should achieve compression on repeated patterns
    REQUIRE(compressed.size() < repeated.size());
}

TEST_CASE("Corrupted data detection", "[compression][error]") {
    SECTION("Invalid codes in compressed data") {
        std::vector<uint16_t> corrupted = {9999, 8888, 7777};
        auto result = StringCompressor::Decompress(corrupted);
        
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == CompressorError::InvalidCode);
    }
    
    SECTION("Out-of-sequence code") {
        // Code that's way ahead of dictionary size
        std::vector<uint16_t> bad_sequence = {65, 500};
        auto result = StringCompressor::Decompress(bad_sequence);
        
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == CompressorError::InvalidCode);
    }
}

TEST_CASE("Long string handling", "[compression]") {
    std::string long_str(10000, 'A');
    auto compressed = StringCompressor::Compress(long_str);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == long_str);
    REQUIRE(result.value().size() == 10000);
}

TEST_CASE("Social media style data", "[compression][social]") {
    SECTION("Repetitive short strings") {
        std::string post = "LOL this is so funny LOL LOL";
        auto compressed = StringCompressor::Compress(post);
        auto result = StringCompressor::Decompress(compressed);
        
        REQUIRE(result.has_value());
        REQUIRE(result.value() == post);
    }
    
    SECTION("Message with special characters") {
        std::string message = "Hey! Check this out: https://example.com #cool";
        auto compressed = StringCompressor::Compress(message);
        auto result = StringCompressor::Decompress(compressed);
        
        REQUIRE(result.has_value());
        REQUIRE(result.value() == message);
    }
}

TEST_CASE("All ASCII characters", "[compression]") {
    std::string all_ascii;
    for (int i = 0; i < 256; i++) {
        all_ascii += static_cast<char>(i);
    }
    
    auto compressed = StringCompressor::Compress(all_ascii);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == all_ascii);
}

TEST_CASE("Random data (worst case for LZW)", "[compression]") {
    // Random-ish data that won't compress well
    std::string random = "aB3$xZ9@mQ7#nR2!pT8%wY4^";
    auto compressed = StringCompressor::Compress(random);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == random);
    // Random data might not compress, but shouldn't crash
}

TEST_CASE("Dictionary size limit", "[compression][limits]") {
    // Create a string that will fill up the dictionary
    std::string large;
    for (int i = 0; i < 5000; i++) {
        large += static_cast<char>('A' + (i % 26));
    }
    
    auto compressed = StringCompressor::Compress(large);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == large);
}

TEST_CASE("Newlines and whitespace", "[compression]") {
    std::string text = "Line 1\nLine 2\n\tTabbed\n  Spaces  \n";
    auto compressed = StringCompressor::Compress(text);
    auto result = StringCompressor::Decompress(compressed);
    
    REQUIRE(result.has_value());
    REQUIRE(result.value() == text);
}

TEST_CASE("Edge case: First code invalid", "[compression][error]") {
    // First code in compressed data is out of range
    std::vector<uint16_t> bad_first = {300}; // Valid but not in initial dict
    auto result = StringCompressor::Decompress(bad_first);
    
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == CompressorError::InvalidCode);
}

TEST_CASE("Compression ratio calculation", "[compression][metrics]") {
    std::string original = "aaaaaaaaaa"; // Should compress well
    auto compressed = StringCompressor::Compress(original);
    
    double ratio = StringCompressor::GetCompressionRatio(original, compressed);
    
    REQUIRE(ratio > 0.0);
    REQUIRE(ratio < 1.0); // Should achieve compression
}

TEST_CASE("Byte serialization for database storage", "[compression][serialization]") {
    std::string message = "Test message for database";
    
    SECTION("Round-trip through bytes") {
        auto bytes = StringCompressor::CompressToBytes(message);
        auto result = StringCompressor::DecompressFromBytes(bytes);
        
        REQUIRE(result.has_value());
        REQUIRE(result.value() == message);
    }
    
    SECTION("IsCompressed detection") {
        auto bytes = StringCompressor::CompressToBytes(message);
        REQUIRE(StringCompressor::IsCompressed(bytes));
        
        std::vector<uint8_t> random_bytes = {1, 2, 3, 4};
        REQUIRE_FALSE(StringCompressor::IsCompressed(random_bytes));
    }
}