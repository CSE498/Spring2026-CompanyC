/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief RobinHoodMap: an open-addressing hash map using Robin Hood hashing for fast lookups.
 * @note Status: IMPLEMENTED (Milestone: initial working class + tests)
 * @note Key property: insertions may rehash and invalidate pointers/references/iterators.
 * @note Contributor: Shamar Dotson (Group 5 - AI Agents)
 */


// Note: Comments were refined with AI assistance; design + implementation are my own.
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cse498 {


template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEq = std::equal_to<Key>>
class RobinHoodMap {
 public:
  using key_type = Key;
  using mapped_type = T;
  using size_type = std::size_t;

  RobinHoodMap() { Init_(kDefaultBuckets); }

  explicit RobinHoodMap(size_type initial_capacity) {
    // Ensure enough buckets for roughly initial_capacity at max load.
    size_type buckets = BucketsForItems_(initial_capacity);
    Init_(buckets);
  }

  [[nodiscard]] size_type Size() const { return size_; }
  [[nodiscard]] bool Empty() const { return size_ == 0; }

  void Clear() {
    buckets_.assign(buckets_.size(), Bucket{});
    size_ = 0;
  }

  [[nodiscard]] float LoadFactor() const {
    return buckets_.empty() ? 0.0f
                            : static_cast<float>(size_) /
                                  static_cast<float>(buckets_.size());
  }


// Load factor threshold used to trigger rehash/growth. Must be in (0, 1) to keep probing fast
// and guarantee open-addressing insertion succeeds; invalid values are treated as programmer error.
  void MaxLoadFactor(float f) {
    assert(f > 0.0f && f < 1.0f);
    max_load_ = f;
  }

  [[nodiscard]] float MaxLoadFactor() const { return max_load_; }


// Ensures the table has enough buckets to store ~n elements without exceeding the max load factor.
// This is a performance hint to reduce repeated rehashing during many inserts. If we need more
// buckets, we rebuild the table at a larger size (which will invalidate pointers/references).
  void Reserve(size_type n) {
    size_type desired = BucketsForItems_(n);
    if (desired > buckets_.size()) {
      Rehash(desired);
    }
  }

// Rebuilds the table to use a new bucket count (rounded up to a power of two).
// All existing entries are moved into a fresh bucket array using the same Robin Hood
// insertion logic. This improves performance when growing and is also used by Reserve().
  void Rehash(size_type bucket_count) {
    bucket_count = NextPow2_(bucket_count);
    if (bucket_count < kMinBuckets) bucket_count = kMinBuckets;

    std::vector<Bucket> new_buckets(bucket_count);
    const size_type new_mask = bucket_count - 1;

    for (auto& b : buckets_) {
      if (!b.entry.has_value()) continue;
       InsertInto_(new_buckets, new_mask, std::move(*b.entry), b.hash);
    }

    buckets_ = std::move(new_buckets);
    mask_ = new_mask;
    // size_ stays the same
  }


// Lookup operation for open-addressed Robin Hood hashing.
// Returns a pointer to the mapped value if found, otherwise nullptr.
// Uses the Robin Hood "early-exit" rule (if a bucket's stored probe distance is smaller than
// our current probe distance, the key cannot appear later in the probe sequence).
  T* Find(const Key& key) {
    return const_cast<T*>(
        static_cast<const RobinHoodMap*>(this)->Find(key));
  }

  const T* Find(const Key& key) const {
    if (buckets_.empty() || size_ == 0) return nullptr;

    const size_type h = hasher_(key);
    size_type idx = h & mask_;
    std::uint32_t dist = 0;

    while (true) {
      const Bucket& b = buckets_[idx];
      if (!b.entry.has_value()) {
        // Empty bucket ends probe chain.
        return nullptr;
      }
      if (b.dist < dist) {
        // Robin Hood early-exit rule.
        return nullptr;
      }
      if (b.hash == h && key_eq_(b.entry->key, key)) {
        return &b.entry->value;
      }
      idx = (idx + 1) & mask_;
      ++dist;
      // In a correct table, we should never probe more than bucket_count.
      assert(dist < buckets_.size());
    }
  }

  [[nodiscard]] bool Contains(const Key& key) const {
    return Find(key) != nullptr;
  }

  T& At(const Key& key) {
    T* p = Find(key);
    if (p == nullptr) throw std::out_of_range("RobinHoodMap::At missing key");
    return *p;
  }

  const T& At(const Key& key) const {
    const T* p = Find(key);
    if (p == nullptr) throw std::out_of_range("RobinHoodMap::At missing key");
    return *p;
  }


  std::pair<T*, bool> Insert(const Key& key, const T& value) {
    MaybeGrow_();
    Entry e{key, value};
    return InsertEntry_(std::move(e));
  }

// Inserts a key/value pair if the key is not already present.
// Returns {pointer_to_value, inserted}, where inserted is false if the key already existed.
// May trigger growth/rehash to maintain the max load factor (which can invalidate pointers).
  std::pair<T*, bool> Insert(Key&& key, T&& value) {
    MaybeGrow_();
    Entry e{std::move(key), std::move(value)};
    return InsertEntry_(std::move(e));
  }

// Constructs the mapped value from forwarded arguments and inserts it if the
// key is not already present. Returns {pointer_to_value, inserted}.
// May trigger growth/rehash, which can invalidate existing pointers/references.
template <class... Args>
std::pair<T*, bool> Emplace(Key key, Args&&... args) {
  MaybeGrow_();
  Entry e{std::move(key), T(std::forward<Args>(args)...)};  
  return InsertEntry_(std::move(e));
}


std::pair<T*, bool> InsertOrAssign(const Key& key, T value) {
  if (T* p = Find(key)) {
    *p = std::move(value);
    return {p, false};
  }
  return Insert(key, std::move(value));
}

 private:
  struct Entry {
    Key key;
    T value;
  };

  struct Bucket {
    size_type hash = 0;
    std::uint32_t dist = 0;
    std::optional<Entry> entry;
  };

  static constexpr size_type kMinBuckets = 8;
  static constexpr size_type kDefaultBuckets = 16;

  std::vector<Bucket> buckets_;
  size_type mask_ = 0;
  size_type size_ = 0;
  float max_load_ = 0.80f;

  Hash hasher_{};
  KeyEq key_eq_{};

// Initializes internal storage to a power-of-two bucket count (minimum kMinBuckets).
// Power-of-two sizing lets us compute bucket indices with a fast bitmask (hash & mask_)
// instead of modulo. Resets size_ to empty and clears all buckets.
  void Init_(size_type bucket_count) {
    bucket_count = NextPow2_(bucket_count);
    if (bucket_count < kMinBuckets) bucket_count = kMinBuckets;
    buckets_.assign(bucket_count, Bucket{});
    mask_ = bucket_count - 1;
    size_ = 0;
  }

// Rounds x up to the next power of two (with 1 as the smallest result).
// Used to keep bucket counts as powers of two so masking works correctly.

  static size_type NextPow2_(size_type x) {
    if (x <= 1) return 1;
    --x;
    for (size_type s = 1; s < sizeof(size_type) * 8; s <<= 1) {
      x |= x >> s;
    }
    return x + 1;
  }

// Computes how many buckets we need to store n items while staying under max_load_.
// Returns a power-of-two bucket count (at least kMinBuckets) so indexing can use masking.
  size_type BucketsForItems_(size_type n) const {
    // buckets >= ceil(n / max_load_)
    const double needed = static_cast<double>(n) / static_cast<double>(max_load_);
    size_type buckets = static_cast<size_type>(needed + 1.0);
    if (buckets < kMinBuckets) buckets = kMinBuckets;
    return NextPow2_(buckets);
  }

// Ensures there will be enough capacity for one more insert without exceeding max_load_.
// If inserting would push the load factor too high, we grow by rehashing to a larger table.
// Growth happens before insertion, which keeps the pointer returned by Insert() stable
// for that call (though any prior pointers may be invalidated by the rehash).

  void MaybeGrow_() {
    if (buckets_.empty()) Init_(kDefaultBuckets);
    // Grow BEFORE insert so returned pointers are stable for this operation.
    const float lf_after =
        static_cast<float>(size_ + 1) / static_cast<float>(buckets_.size());
    if (lf_after > max_load_) {
      Rehash(buckets_.size() * 2);
    }
  }

std::pair<T*, bool> InsertEntry_(Entry&& entry) {
  size_type h = hasher_(entry.key);
  size_type idx = h & mask_;
  std::uint32_t dist = 0;

  std::optional<Entry> cur(std::move(entry));

  while (true) {
    Bucket& b = buckets_[idx];

    if (!b.entry.has_value()) {
      b.hash = h;
      b.dist = dist;
      b.entry = std::move(cur);
      ++size_;
      return {&b.entry->value, true};
    }

    // Key already exists
    if (b.hash == h && key_eq_(b.entry->key, cur->key)) {
      return {&b.entry->value, false};
    }

    // Robin Hood rule: if incoming entry probed farther, steal.
    if (b.dist < dist) {
      std::swap(b.hash, h);
      std::swap(b.dist, dist);
      std::swap(b.entry, cur);  // optional <-> optional
    }

    idx = (idx + 1) & mask_;

    ++dist;
    assert(dist < buckets_.size());
  }
}

  // Insert into external bucket array (during the rehash). This expects no duplicates expected.
  static void InsertInto_(std::vector<Bucket>& dst,
                          size_type mask,
                          Entry&& entry,
                          size_type h) {
    size_type idx = h & mask;
    std::uint32_t dist = 0;

    std::optional<Entry> cur(std::move(entry));

    while (true) {
      Bucket& b = dst[idx];

      if (!b.entry.has_value()) {
        b.hash = h;
        b.dist = dist;
        b.entry = std::move(cur);
        return;
      }

      if (b.dist < dist) {
        std::swap(b.hash, h);
        std::swap(b.dist, dist);
        std::swap(b.entry, cur);
      }

      idx = (idx + 1) & mask;
      ++dist;
      assert(dist < dst.size());
    }
  }
};

}
