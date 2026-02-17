/**
 * @file RandomTest.cpp
 * @author Benjamin Forbes
 *
 * CITATION: Test cases are LLM-generated. Original class had asserts within the functions
 * to ensure proper behavior, so this is a redundancy to ensure Catch2 functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Random.h"

#include <limits>
#include <vector>

TEST_CASE("Random: seeded constructor produces deterministic sequences", "[Random]") {
    Random a(123456u);
    Random b(123456u);

    // Same calls in same order => identical results
    for (int i = 0; i < 200; ++i) {
        REQUIRE(a.GetInt(-50, 50) == b.GetInt(-50, 50));
    }

    for (int i = 0; i < 200; ++i) {
        const double da = a.GetDouble(-10.0, 10.0);
        const double db = b.GetDouble(-10.0, 10.0);
        REQUIRE(da == db);
    }

    for (int i = 0; i < 200; ++i) {
        REQUIRE(a.P(0.37) == b.P(0.37));
    }
}

TEST_CASE("Random: Seed() produces deterministic sequences", "[Random]") {
    Random a;
    Random b;

    a.Seed(42u);
    b.Seed(42u);

    for (int i = 0; i < 300; ++i) {
        REQUIRE(a.GetInt(0, 1000) == b.GetInt(0, 1000));
    }
}

TEST_CASE("Random: different seeds generally produce different sequences", "[Random]") {
    // This is probabilistic, but collisions for multiple draws are astronomically unlikely.
    Random a(1u);
    Random b(2u);

    bool anyDifferent = false;
    for (int i = 0; i < 50; ++i) {
        if (a.GetInt(0, std::numeric_limits<int>::max()) !=
            b.GetInt(0, std::numeric_limits<int>::max())) {
            anyDifferent = true;
            break;
        }
    }
    REQUIRE(anyDifferent);
}

TEST_CASE("GetInt: values are within inclusive bounds", "[Random][GetInt]") {
    Random r(7u);

    SECTION("Small range") {
        for (int i = 0; i < 2000; ++i) {
            int x = r.GetInt(10, 20);
            REQUIRE(x >= 10);
            REQUIRE(x <= 20);
        }
    }

    SECTION("Negative range") {
        for (int i = 0; i < 2000; ++i) {
            int x = r.GetInt(-20, -10);
            REQUIRE(x >= -20);
            REQUIRE(x <= -10);
        }
    }

    SECTION("Crossing zero") {
        for (int i = 0; i < 2000; ++i) {
            int x = r.GetInt(-5, 5);
            REQUIRE(x >= -5);
            REQUIRE(x <= 5);
        }
    }

    SECTION("Degenerate range min == max") {
        for (int i = 0; i < 200; ++i) {
            REQUIRE(r.GetInt(123, 123) == 123);
        }
    }
}

TEST_CASE("GetDouble: values are within bounds", "[Random][GetDouble]") {
    Random r(99u);

    SECTION("Typical range") {
        for (int i = 0; i < 4000; ++i) {
            double x = r.GetDouble(-1.5, 3.25);
            REQUIRE(x >= -1.5);
            REQUIRE(x <= 3.25); // uniform_real_distribution is usually [min,max)
        }
    }

    SECTION("Degenerate range min == max") {
        for (int i = 0; i < 200; ++i) {
            double x = r.GetDouble(2.0, 2.0);
            REQUIRE(x == 2.0);
        }
    }

    SECTION("Very large magnitude range (finite results)") {
        const double lo = -1e100;
        const double hi =  1e100;
        for (int i = 0; i < 2000; ++i) {
            double x = r.GetDouble(lo, hi);
            REQUIRE(x >= lo);
            REQUIRE(x <= hi);
            REQUIRE(std::isfinite(x));
        }
    }
}

TEST_CASE("P(probability): boundary behavior", "[Random][P]") {
    Random r(123u);

    SECTION("P(0) is always false") {
        for (int i = 0; i < 500; ++i) {
            REQUIRE_FALSE(r.P(0.0));
        }
    }

    SECTION("P(1) is always true") {
        for (int i = 0; i < 500; ++i) {
            REQUIRE(r.P(1.0));
        }
    }
}

TEST_CASE("P(probability): outputs are boolean and show variability for mid probabilities", "[Random][P]") {
    Random r(2024u);

    // For p=0.5 over many draws, seeing both outcomes is overwhelmingly likely.
    bool sawTrue = false;
    bool sawFalse = false;

    for (int i = 0; i < 2000; ++i) {
        bool v = r.P(0.5);
        sawTrue |= v;
        sawFalse |= !v;
        if (sawTrue && sawFalse) break;
    }

    REQUIRE(sawTrue);
    REQUIRE(sawFalse);
}

TEST_CASE("Random: copy semantics clone engine state", "[Random]") {
    Random a(777u);

    // Advance a a bit
    std::vector<int> prefix;
    for (int i = 0; i < 25; ++i) prefix.push_back(a.GetInt(0, 1000000));

    // Copy should clone the *current* internal state
    Random b = a;

    // Next values should match between a and b now
    for (int i = 0; i < 200; ++i) {
        REQUIRE(a.GetInt(-100, 100) == b.GetInt(-100, 100));
    }

    // If one advances further, they diverge
    int a_next = a.GetInt(0, 1000000);
    int b_next = b.GetInt(0, 1000000);
    // After equal number of calls above, they were aligned; this call keeps them aligned too.
    REQUIRE(a_next == b_next);

    // Now advance only one:
    int a_only = a.GetInt(0, 1000000);
    int b_still = b.GetInt(0, 1000000);
    REQUIRE(a_only == b_still); // still aligned because we advanced both once each
    // One more on a only:
    int a_only2 = a.GetInt(0, 1000000);
    int b_next2 = b.GetInt(0, 1000000);
    REQUIRE(a_only2 == b_next2); // aligned if calls remain paired

    // (This test mostly documents copy=stateful behavior; if you *don't* want this,
    // delete copy/move constructors or implement different semantics.)
}

TEST_CASE("Sanity: GetInt and GetDouble should not be constant for a normal range", "[Random]") {
    Random r(314159u);

    SECTION("GetInt produces at least two distinct values") {
        int first = r.GetInt(0, 100);
        bool sawDifferent = false;
        for (int i = 0; i < 500; ++i) {
            if (r.GetInt(0, 100) != first) {
                sawDifferent = true;
                break;
            }
        }
        REQUIRE(sawDifferent);
    }

    SECTION("GetDouble produces at least two distinct values") {
        double first = r.GetDouble(0.0, 1.0);
        bool sawDifferent = false;
        for (int i = 0; i < 500; ++i) {
            if (r.GetDouble(0.0, 1.0) != first) {
                sawDifferent = true;
                break;
            }
        }
        REQUIRE(sawDifferent);
    }
}
