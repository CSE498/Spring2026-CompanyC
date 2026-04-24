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
#include <type_traits>
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

 public:
  constexpr DataLog() = default;

  /**
   * @brief Add a value to the log.
   *
   * Floating-point NaN and infinity values are ignored. Other value types are
   * stored as-is.
   */
  void Add(const T& value) {
    if constexpr (std::is_floating_point_v<T>) {
      if (!std::isfinite(value)) {
        return;
      }
    }

    mDataValues.push_back(value);
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
  constexpr void Clear() noexcept { mDataValues.clear(); }

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
