/**
 * @file DataLog.hpp
 * @author Group 23
 *
 * @brief Simple named data logger with summary statistics.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <map>
#include <numeric>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace cse498 {

template <typename T = double>
class DataLog {
 private:
  std::unordered_map<std::string, std::vector<T>> mData;
  std::size_t mSnapshotCount = 0;

  [[nodiscard]] static bool IsValid(const T& value) noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return std::isfinite(value);
    }
    return true;
  }

  template <typename MapT>
  void AddSnapshotValues(const MapT& values)
    requires std::is_convertible_v<typename MapT::mapped_type, T>
  {
    for (auto& [name, series] : mData) {
      if (!values.contains(name)) {
        series.push_back(T{});
      }
    }

    for (const auto& [name, value] : values) {
      auto& series = mData[name];
      if (series.size() < mSnapshotCount) {
        series.resize(mSnapshotCount, T{});
      }

      const T converted_value = static_cast<T>(value);
      series.push_back(IsValid(converted_value) ? converted_value : T{});
    }

    ++mSnapshotCount;
  }

 public:
  struct Stats {
    std::size_t count = 0;
    double sum = 0.0;
    double mean = 0.0;
    double median = 0.0;
    T min{};
    T max{};
  };

  void Add(const std::string& name, const T& value) {
    if (IsValid(value)) {
      mData[name].push_back(value);
    }
  }

  template <typename U>
  void AddSnapshot(const std::unordered_map<std::string, U>& values)
    requires std::is_convertible_v<U, T>
  {
    AddSnapshotValues(values);
  }

  template <typename U>
  void AddSnapshot(const std::map<std::string, U>& values)
    requires std::is_convertible_v<U, T>
  {
    AddSnapshotValues(values);
  }

  [[nodiscard]] const std::vector<T>& Values(const std::string& name) const noexcept {
    static const std::vector<T> empty;
    const auto it = mData.find(name);
    return it == mData.end() ? empty : it->second;
  }

  [[nodiscard]] Stats Summary(const std::string& name) const
    requires std::is_arithmetic_v<T> && std::totally_ordered<T>
  {
    const auto& values = Values(name);
    Stats stats;
    stats.count = values.size();
    if (values.empty()) {
      return stats;
    }

    stats.sum = std::accumulate(values.begin(), values.end(), 0.0);
    stats.mean = stats.sum / values.size();
    stats.min = *std::min_element(values.begin(), values.end());
    stats.max = *std::max_element(values.begin(), values.end());

    std::vector<T> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t mid = sorted.size() / 2;
    stats.median = sorted.size() % 2 == 1
      ? static_cast<double>(sorted[mid])
      : std::midpoint(static_cast<double>(sorted[mid - 1]), static_cast<double>(sorted[mid]));

    return stats;
  }

  [[nodiscard]] std::vector<std::string> Names() const {
    std::vector<std::string> names;
    names.reserve(mData.size());
    for (const auto& [name, values] : mData) {
      (void)values;
      names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
  }

  void Clear() noexcept {
    mData.clear();
    mSnapshotCount = 0;
  }
};

}  // namespace cse498
