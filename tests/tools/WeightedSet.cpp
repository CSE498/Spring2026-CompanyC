/**
* @file TestWeightedSet.cpp
* @author George Almeida
* 
* This unit test file was written with the help of GitHub Copilot. The test cases and design decisions were made by the author.
*/

#include "catch2/catch.hpp"

#include "tools/WeightedSet.hpp"

#include <random>
#include <string>
#include <utility>

using cse498::WeightedSet;

TEST_CASE("WeightedSet basic insert/erase/find/size/clear/total_weight", "[WeightedSet]") {
    WeightedSet<std::string> s;
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
    REQUIRE(s.total_weight() == 0.0);

    // Insert elements
    REQUIRE(s.insert("A", 0.5));
    REQUIRE(s.insert("B", 3.1));
    REQUIRE(s.insert("C", 1.5));

    REQUIRE(!s.empty());
    REQUIRE(s.size() == 3);
    REQUIRE(s.total_weight() == Approx(5.1).epsilon(1e-12));

    // Duplicate insert fails
    REQUIRE(!s.insert("A", 1.0));

    // Find existing and non-existing
    auto itA = s.find("A");
    REQUIRE(itA != s.end());
    REQUIRE(itA->first == "A");

    auto itX = s.find("X");
    REQUIRE(itX == s.end());

    // Erase existing
    REQUIRE(s.erase("B"));
    REQUIRE(s.size() == 2);
    REQUIRE(s.total_weight() == Approx(2.0).epsilon(1e-12)); // A(0.5) + C(1.5)

    // Erase non-existing returns false
    REQUIRE(!s.erase("B"));

    s.clear();
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
    REQUIRE(s.total_weight() == 0.0);
}

TEST_CASE("WeightedSet sample_by_weight deterministic boundaries", "[WeightedSet][sampling]") {
    WeightedSet<std::string> s;
    REQUIRE(s.insert("A", 0.5));
    REQUIRE(s.insert("B", 3.1));
    REQUIRE(s.insert("C", 1.5));

    // cumulative ranges: [0.0, 0.5] -> A, (0.5, 3.6] -> B, (3.6, 5.1] -> C
    auto it = s.sample_by_weight(0.0);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "A");

    it = s.sample_by_weight(0.5);
    REQUIRE(it->first == "A");

    it = s.sample_by_weight(0.5000001);
    REQUIRE(it->first == "B");

    it = s.sample_by_weight(3.6);
    REQUIRE(it->first == "B");

    it = s.sample_by_weight(3.6000001);
    REQUIRE(it->first == "C");

    it = s.sample_by_weight(5.1);
    REQUIRE(it->first == "C");
}

TEST_CASE("WeightedSet sample_random returns element in set", "[WeightedSet][sampling]") {
    WeightedSet<int> s;
    s.insert(1, 1.0);
    s.insert(2, 2.0);
    s.insert(3, 3.0);

    std::mt19937 rng(12345);
    auto it = s.sample_random(rng);
    REQUIRE(it != s.end());
    int val = it->first;
    REQUIRE((val == 1 || val == 2 || val == 3));

    // total weight unchanged
    REQUIRE(s.total_weight() == Approx(6.0).epsilon(1e-12));
}