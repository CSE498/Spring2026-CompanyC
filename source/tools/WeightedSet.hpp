/**
* @author George Almeida
* @file WeightedSet.h
* @brief A simple implementation of a weighted set data structure that maintains unique elements 
* with associated positive weights. Supports insertion, deletion, lookup, and sampling by cumulative weight.
* 
* This was written with the help of GitHub Copilot. Implmentation and design decisions were made by the author.
*/

#pragma once

#include <map>
#include <cassert>
#include <stdexcept>
#include <concepts>
#include <type_traits>
#include <random>
#include <cmath>
#include <limits>
#include <utility>

namespace cse498 {

// WeightedSet<T> - maintain unique elements each with an associated positive weight.
// This is an initial, simple implementation that stores elements in a std::map
// and keeps the total weight. Sampling by cumulative weight is supported but
// currently implemented with linear iteration (O(n)).
template <typename T, typename Compare = std::less<T>>
class WeightedSet {
public:
    using value_type = T;
    using weight_type = double;
    using map_type = std::map<T, weight_type, Compare>;
    using const_iterator = typename map_type::const_iterator;

    static_assert(std::is_floating_point_v<weight_type>, "WeightedSet::weight_type must be floating point");

    WeightedSet() = default;
    ~WeightedSet() = default;
    WeightedSet(const WeightedSet&) = default;
    WeightedSet(WeightedSet&&) noexcept = default;
    WeightedSet& operator=(const WeightedSet&) = default;
    WeightedSet& operator=(WeightedSet&&) noexcept = default;

    // Value semantics: two sets are equal iff their stored key->weight mappings are equal.
    bool operator==(const WeightedSet& other) const noexcept {
        return m_map == other.m_map;
    }
    bool operator!=(const WeightedSet& other) const noexcept { return !(*this == other); }

    void swap(WeightedSet& other) noexcept {
        using std::swap;
        swap(m_map, other.m_map);
        swap(m_total, other.m_total);
    }

    // Insert a new element with the given non-negative weight. Returns true on success.
    // If the element already exists, returns false.
    bool insert(const T& value, weight_type weight) {
        // Negative weights are a programmer error: assert in debug; allow zero.
        assert(weight >= 0.0 && "WeightedSet::insert: weight must be non-negative");
        
        auto it = m_map.find(value);
        if (it != m_map.end()) {
            return false; // duplicate
        }
        m_map.emplace(value, weight);
        m_total += weight;
        return true;
    }

    // Erase an element. Returns true if an element was erased, false if it was not found.
    bool erase(const T& value) {
        auto it = m_map.find(value);
        if (it == m_map.end()) return false;
        m_total -= it->second;
        m_map.erase(it);
        return true;
    }

    // Update the weight of an existing element. Returns true if updated, false if not found.
    bool set_weight(const T& value, weight_type weight) {
        // Negative weights are a programmer error: assert in debug; allow zero.
        assert(weight >= 0.0 && "WeightedSet::set_weight: weight must be non-negative");

        auto it = m_map.find(value);
        if (it == m_map.end()) return false;

        m_total -= it->second;
        it->second = weight;
        m_total += weight;
        return true;
    }

    // Find element. If not found, returns end().
    const_iterator find(const T& value) const noexcept {
        return m_map.find(value);
    }

    // Number of elements.
    std::size_t size() const noexcept { return m_map.size(); }
    bool empty() const noexcept { return m_map.empty(); }

    void clear() noexcept {
        m_map.clear();
        m_total = 0.0;
    }

    // Total weight of all elements.
    weight_type total_weight() const noexcept { return m_total; }

    // Sample by cumulative weight. The argument must be in the closed interval [0, total_weight()].
    // Returns an iterator to the selected element. If the set is empty or the weight is out
    // of range, an assertion will fail.
    const_iterator sample_by_weight(weight_type cumulative) const {
        // If empty, return end() rather than asserting so callers can handle gracefully.
        if (empty()) return m_map.cend();

        // Out of range cumulative values are considered invalid; return end().
        if (cumulative < 0.0 || cumulative > m_total) return m_map.cend();

        weight_type acc = 0.0;
        for (auto it = m_map.cbegin(); it != m_map.cend(); ++it) {
            acc += it->second;
            if (cumulative <= acc) return it;
        }

        // Due to floating point rounding, we may fall out of the loop when cumulative == m_total.
        // In that case return the last element.
        auto it = m_map.cend();
        --it;
        return it;
    }

    // Convenience: sample using a random number generator. Produces a uniform real in [0, total_weight()).
    template <std::uniform_random_bit_generator URBG>
    const_iterator sample_random(URBG& rng) const {
        if (empty()) return m_map.cend();

        // If every element has weight 0, there's no meaningful "random by weight" to do.
        // In that degenerate case, just return the first element.
        if (m_total == 0.0) return m_map.cbegin();

        // Use a distribution over [0, m_total). Clamp in case floating-point quirks
        // ever produce a value slightly above m_total.
        std::uniform_real_distribution<weight_type> dist(0.0, m_total);
        weight_type x = dist(rng);
        if (x > m_total) x = m_total;
        return sample_by_weight(x);
    }

    const_iterator begin() const noexcept { return m_map.cbegin(); }
    const_iterator end() const noexcept { return m_map.cend(); }

    // Lambda-friendly iteration: calls fn(key, weight) for every stored element.
    template <typename Fn>
    void for_each(Fn&& fn) const {
        for (const auto& [k, w] : m_map) {
            fn(k, w);
        }
    }

private:
    map_type m_map;
    weight_type m_total = 0.0;
};

} // namespace cse498

