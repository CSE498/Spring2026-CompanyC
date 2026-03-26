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

TEST_CASE("WeightedSet set_weight updates total weight and sampling ranges", "[WeightedSet][sampling]") {
    WeightedSet<int> s;
    REQUIRE(s.insert(1, 1.0));
    REQUIRE(s.insert(2, 2.0));
    REQUIRE(s.insert(3, 3.0));
    REQUIRE(s.total_weight() == Approx(6.0).epsilon(1e-12));

    // Update an existing element.
    REQUIRE(s.set_weight(2, 5.0));
    REQUIRE(s.total_weight() == Approx(9.0).epsilon(1e-12)); // 1.0 + 5.0 + 3.0

    // With keys {1,2,3}: cumulative [0,1] -> 1, (1,6] -> 2, (6,9] -> 3.
    auto it = s.sample_by_weight(0.0);
    REQUIRE(it->first == 1);
    it = s.sample_by_weight(1.0);
    REQUIRE(it->first == 1);
    it = s.sample_by_weight(1.0000001);
    REQUIRE(it->first == 2);
    it = s.sample_by_weight(6.0);
    REQUIRE(it->first == 2);
    it = s.sample_by_weight(6.0000001);
    REQUIRE(it->first == 3);
    it = s.sample_by_weight(9.0);
    REQUIRE(it->first == 3);

    // Updating a non-existent element should fail and not mutate totals.
    REQUIRE(!s.set_weight(99, 1.0));
    REQUIRE(s.total_weight() == Approx(9.0).epsilon(1e-12));
}

TEST_CASE("WeightedSet for_each uses lambdas and exposes stored weights", "[WeightedSet]") {
    WeightedSet<std::string> s;
    REQUIRE(s.insert("A", 0.5));
    REQUIRE(s.insert("B", 1.5));

    double sum = 0.0;
    std::size_t count = 0;

    // for_each is intended to be used with lambdas.
    s.for_each([&](const std::string& key, double w) {
        (void)key; // key is checked by behavior below
        sum += w;
        ++count;
    });

    REQUIRE(count == 2);
    REQUIRE(sum == Approx(2.0).epsilon(1e-12));
}

TEST_CASE("WeightedSet value semantics with operator==", "[WeightedSet]") {
    WeightedSet<int> a;
    WeightedSet<int> b;

    REQUIRE(a == b);

    a.insert(1, 1.0);
    b.insert(1, 1.0);
    REQUIRE(a == b);

    a.set_weight(1, 2.0);
    REQUIRE(a != b);
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

TEST_CASE("WeightedSet sample_by_weight works for single element", "[WeightedSet][sampling]") {
    WeightedSet<std::string> s;
    REQUIRE(s.insert("A", 2.3));
    REQUIRE(s.size() == 1);
    REQUIRE(s.total_weight() == Approx(2.3).epsilon(1e-12));

    auto it = s.sample_by_weight(0.0);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "A");

    it = s.sample_by_weight(1.0);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "A");

    it = s.sample_by_weight(2.3);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "A");
}

TEST_CASE("WeightedSet sample_random works for single element", "[WeightedSet][sampling]") {
    WeightedSet<int> s;
    REQUIRE(s.insert(42, 7.7));

    std::mt19937 rng(12345);
    auto it = s.sample_random(rng);
    REQUIRE(it != s.end());
    REQUIRE(it->first == 42);

    // Sampling should not mutate the set.
    REQUIRE(s.size() == 1);
    REQUIRE(s.total_weight() == Approx(7.7).epsilon(1e-12));
}

TEST_CASE("WeightedSet erase then insert updates sampling ranges", "[WeightedSet][sampling]") {
    WeightedSet<int> s;
    REQUIRE(s.insert(1, 1.0));
    REQUIRE(s.insert(2, 2.0));
    REQUIRE(s.insert(3, 3.0));
    REQUIRE(s.size() == 3);
    REQUIRE(s.total_weight() == Approx(6.0).epsilon(1e-12));

    // Erase an element and ensure total/size update.
    REQUIRE(s.erase(2));
    REQUIRE(s.size() == 2);
    REQUIRE(s.total_weight() == Approx(4.0).epsilon(1e-12)); // 1.0 + 3.0

    // With keys {1,3}: cumulative [0,1] -> 1, (1,4] -> 3.
    auto it = s.sample_by_weight(0.0);
    REQUIRE(it->first == 1);
    it = s.sample_by_weight(1.0);
    REQUIRE(it->first == 1);
    it = s.sample_by_weight(1.0000001);
    REQUIRE(it->first == 3);
    it = s.sample_by_weight(4.0);
    REQUIRE(it->first == 3);

    // Insert more elements after erasing, and ensure ranges reflect new totals.
    REQUIRE(s.insert(4, 4.0));
    REQUIRE(s.size() == 3);
    REQUIRE(s.total_weight() == Approx(8.0).epsilon(1e-12)); // 1.0 + 3.0 + 4.0

    // With keys {1,3,4}: cumulative [0,1] -> 1, (1,4] -> 3, (4,8] -> 4.
    it = s.sample_by_weight(0.0);
    REQUIRE(it->first == 1);
    it = s.sample_by_weight(3.0);
    REQUIRE(it->first == 3);
    it = s.sample_by_weight(4.0);
    REQUIRE(it->first == 3);
    it = s.sample_by_weight(4.0000001);
    REQUIRE(it->first == 4);
    it = s.sample_by_weight(8.0);
    REQUIRE(it->first == 4);
}

TEST_CASE("WeightedSet sample_by_weight skips positive cumulative past zero-weight entries", "[WeightedSet][sampling]") {
    WeightedSet<std::string> s;
    REQUIRE(s.insert("A", 0.0));
    REQUIRE(s.insert("B", 1.0));
    REQUIRE(s.size() == 2);
    REQUIRE(s.total_weight() == Approx(1.0).epsilon(1e-12));

    // Cumulative == 0 should select the first element whose running sum is >= cumulative.
    auto it = s.sample_by_weight(0.0);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "A");

    // Any positive cumulative should skip the zero-weight element and select "B".
    it = s.sample_by_weight(1e-12);
    REQUIRE(it != s.end());
    REQUIRE(it->first == "B");
}