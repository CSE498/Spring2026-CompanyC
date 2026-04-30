/**
 * @file DataLog.hpp
 * @author Group 23
 *
 * @brief Tracks logged samples and provides simple summary statistics.
 *
 * DataLog defaults to double for the common numeric use case:
 *   DataLog log;              // same as DataLog<double>
 *
 * The sample type can be overridden when another value type needs to be
 * stored, such as world positions for external analysis:
 *   DataLog<WorldPosition> positions;
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../core/Database.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498 {

/// Defaults to double 
template <typename T = double>
class DataLog {
 private:
  /// Stores the logged values
  std::vector<T> mDataValues;
  std::unordered_map<std::string, std::vector<T>> mNamedDataValues;
  std::size_t mSnapshotCount = 0;

  [[nodiscard]] static bool IsValid(const T& value) noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return std::isfinite(value);
    }
    return true;
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

  constexpr DataLog() = default;

  /**
   * @brief Add a value to the log.
   *
   * Floating-point NaN and infinity values are ignored. Other value types are
   * stored as-is.
   */
  void Add(const T& value) {
    if (IsValid(value)) {
      mDataValues.push_back(value);
    }
  }

  /**
   * @brief Add a named value to the log.
   */
  void Add(const std::string& name, const T& value) {
    if (IsValid(value)) {
      mNamedDataValues[name].push_back(value);
    }
  }

  /**
   * @brief Add one aligned sample across multiple named series.
   */
  void AddSnapshot(const std::unordered_map<std::string, T>& values) {
    for (auto& [name, series] : mNamedDataValues) {
      if (values.find(name) == values.end()) {
        series.push_back(T{});
      }
    }

    for (const auto& [name, value] : values) {
      auto& series = mNamedDataValues[name];
      if (series.size() < mSnapshotCount) {
        series.resize(mSnapshotCount, T{});
      }
      series.push_back(IsValid(value) ? value : T{});
    }

    ++mSnapshotCount;
  }

  /**
   * @brief Return the mean of logged values.
   *
   * Intended for numeric sample types. Non-numeric logs can still use Add(),
   * Values(), Count(), IsEmpty(), and Clear() for external analysis.
   *
   * Returns 0.0 if the log is empty.
   */
  [[nodiscard]] double Mean() const
    requires std::is_arithmetic_v<T>
  {
    if (mDataValues.empty()) {
      return 0.0;
    }
    const double sum = std::accumulate(mDataValues.begin(), mDataValues.end(), 0.0);
    return sum / mDataValues.size();
  }

  /**
   * @brief Return the median of logged values.
   *
   * Intended for numeric sample types.
   *
   * Returns 0.0 if the log is empty.
   */
  [[nodiscard]] double Median() const
    requires std::is_arithmetic_v<T>
  {
    if (mDataValues.empty()) {
      return 0.0;
    }

    std::vector<T> partitioned_values = mDataValues;
    const auto middle_it = partitioned_values.begin() + (partitioned_values.size() / 2);
    std::nth_element(partitioned_values.begin(), middle_it, partitioned_values.end());

    if (partitioned_values.size() % 2 == 1) {
      return *middle_it;
    }

    const double upper = *middle_it;
    const double lower = *std::max_element(partitioned_values.begin(), middle_it);
    return std::midpoint(lower, upper);
  }

  /**
   * @brief Return the minimum logged value.
   *
   * Available for sample types with a total ordering.
   *
   * Returns T{} if the log is empty.
   */
  [[nodiscard]] T Min() const
    requires std::totally_ordered<T>
  {
    if (mDataValues.empty()) {
      return T{};
    }
    return *std::min_element(mDataValues.begin(), mDataValues.end());
  }

  /**
   * @brief Return the maximum logged value.
   *
   * Available for sample types with a total ordering.
   *
   * Returns T{} if the log is empty.
   */
  [[nodiscard]] T Max() const
    requires std::totally_ordered<T>
  {
    if (mDataValues.empty()) {
      return T{};
    }
    return *std::max_element(mDataValues.begin(), mDataValues.end());
  }

  /**
   * @brief Access all logged values.
   */
  [[nodiscard]] constexpr const std::vector<T>& Values() const noexcept {
    return mDataValues;
  }

  /**
   * @brief Access logged values for one named series.
   */
  [[nodiscard]] const std::vector<T>& Values(const std::string& name) const noexcept {
    static const std::vector<T> empty;
    const auto it = mNamedDataValues.find(name);
    return it == mNamedDataValues.end() ? empty : it->second;
  }

  /**
   * @brief Return the names of all named series.
   */
  [[nodiscard]] std::vector<std::string> Names() const {
    std::vector<std::string> names;
    names.reserve(mNamedDataValues.size());
    for (const auto& entry : mNamedDataValues) {
      names.push_back(entry.first);
    }
    std::sort(names.begin(), names.end());
    return names;
  }

  /**
   * @brief Return summary statistics for one named series.
   */
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
    const auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
    stats.min = *min_it;
    stats.max = *max_it;

    std::vector<T> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t mid = sorted.size() / 2;
    stats.median = sorted.size() % 2 == 1
      ? static_cast<double>(sorted[mid])
      : std::midpoint(static_cast<double>(sorted[mid - 1]), static_cast<double>(sorted[mid]));

    return stats;
  }

  /**
   * @brief Number of values currently in the log.
   */
  [[nodiscard]] constexpr std::size_t Count() const noexcept {
    return mDataValues.size();
  }

  /**
   * @brief Returns true if log is empty, else false.
   */
  [[nodiscard]] constexpr bool IsEmpty() const noexcept {
    return mDataValues.empty();
  }

  /**
   * @brief Removes all logged values.
   */
  void Clear() noexcept {
    mDataValues.clear();
    mNamedDataValues.clear();
    mSnapshotCount = 0;
  }

  /**
   * @brief Registers DataLog<WorldPosition> with the database.
   */
  void RegisterWithDatabase(cse498::Database& db)
    requires std::same_as<T, WorldPosition>
  {
    db.RegisterType<DataLog<T>>("DataLogWorldPosition",
        [](const DataLog<T>& log) -> std::string {
            cse498::Serializer s;
            std::string data = s.Serialize(static_cast<unsigned long>(log.Values().size()));
            for (const auto& world_pos : log.Values()) {
              data += s.Serialize(world_pos.X());
              data += s.Serialize(world_pos.Y());
            }
            return data;
        },
        [](const std::string& data) -> std::optional<DataLog<T>> {
            cse498::Serializer s;
            size_t pos = 0;
            auto count = s.DeserializeAt<unsigned long>(data, pos);
            if (!count) return std::nullopt;

            DataLog<T> log;
            for (unsigned long i = 0; i < *count; ++i) {
              auto x = s.DeserializeAt<double>(data, pos);
              auto y = s.DeserializeAt<double>(data, pos);
              if (!x || !y) return std::nullopt;
              log.Add(WorldPosition(*x, *y));
            }
            return log;
        }
    );
  }
};

}  // namespace cse498
