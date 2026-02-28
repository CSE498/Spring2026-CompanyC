/**
 * @file TestMemofunction.cpp
 * @author Jose Antonio Hernandez-Martinez
 *
 * Unit tests for the Memofunction template.
 * Written with help from ChatGPT and other sources.
 */

#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/Memofunction.hpp"
#include <string>

// Simple functions to wrap
static int RandomMath(int x)
{
    x *= 3;
    x += 2;
    return x;
}

static int Square(int x)
{
    return x * x;
}

static std::string RepeatStar(const std::string &s)
{
    return s + "*";
}
namespace cse498
{

    TEST_CASE("Test Memofunction constructor + initial state", "[Memofunction]")
    {
        Memofunction<int, decltype(&RandomMath)> memo(RandomMath);
        REQUIRE(memo.size() == 0);
    }

    TEST_CASE("Test cache hit (same input does not grow cache)", "[Memofunction]")
    {
        Memofunction<int, decltype(&Square)> memo(Square);

        int a = memo(5);
        REQUIRE(a == 25);
        REQUIRE(memo.size() == 1);

        int b = memo(5);
        REQUIRE(b == 25);

        // Should still be 1 cached entry (hit)
        REQUIRE(memo.size() == 1);
    }

    TEST_CASE("Test cache grows with new keys until capacity", "[Memofunction]")
    {
        Memofunction<int, decltype(&RandomMath)> memo(RandomMath);

        REQUIRE(memo.size() == 0);

        REQUIRE(memo(5) == 17);
        REQUIRE(memo.size() == 1);

        REQUIRE(memo(7) == 23);
        REQUIRE(memo.size() == 2);

        REQUIRE(memo(9) == 29);
        REQUIRE(memo.size() == 3); // capacity is 3 in your class
    }

    TEST_CASE("Test FIFO eviction when capacity exceeded", "[Memofunction]")
    {
        // capacity is fixed at 3 for now
        Memofunction<int, decltype(&RandomMath)> memo(RandomMath);

        // Fill cache with 3 unique keys
        REQUIRE(memo(5) == 17);
        REQUIRE(memo(7) == 23);
        REQUIRE(memo(9) == 29);
        REQUIRE(memo.size() == 3);

        // This should evict the oldest key (5) because FIFO
        REQUIRE(memo(10) == 32);
        REQUIRE(memo.size() == 3);

        // If FIFO eviction happened, key=5 was evicted and re-inserting it
        // would still keep size 3 (evict another oldest).
        REQUIRE(memo(5) == 17);
        REQUIRE(memo.size() == 3);
    }

    TEST_CASE("Test Clear empties cache and resets order", "[Memofunction]")
    {
        Memofunction<int, decltype(&Square)> memo(Square);

        memo(2);
        memo(3);
        REQUIRE(memo.size() == 2);

        memo.clear();
        REQUIRE(memo.size() == 0);

        // After clear, adding again should behave like a fresh cache
        REQUIRE(memo(2) == 4);
        REQUIRE(memo.size() == 1);
    }

    TEST_CASE("Test Memofunction works with non-int Key and non-int result", "[Memofunction]")
    {
        // Key = std::string, result = std::string
        Memofunction<std::string, decltype(&RepeatStar)> memo(RepeatStar);

        REQUIRE(memo.size() == 0);

        auto r1 = memo(std::string("hi"));
        REQUIRE(r1 == "hi*");
        REQUIRE(memo.size() == 1);

        auto r2 = memo(std::string("hi"));
        REQUIRE(r2 == "hi*");
        REQUIRE(memo.size() == 1); // hit, size unchanged

        auto r3 = memo(std::string("yo"));
        REQUIRE(r3 == "yo*");
        REQUIRE(memo.size() == 2);
    }
}