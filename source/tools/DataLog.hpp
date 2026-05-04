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
#include <numeric>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../core/Database.hpp"
#include "../core/WorldPosition.hpp"

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

  void AddSnapshot(const std::unordered_map<std::string, T>& values) {
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

      series.push_back(IsValid(value) ? value : T{});
    }

    ++mSnapshotCount;
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

  [[nodiscard]] std::vector<std::string> Names() const {
    std::vector<std::string> names;
    names.reserve(mData.size());
    for (const auto& entry : mData) {
      names.push_back(entry.first);
    }
    std::sort(names.begin(), names.end());
    return names;
  }

  void Clear() noexcept {
    mData.clear();
    mSnapshotCount = 0;
  }

  /**
   * @brief Registers this DataLog type with the database.
   *
   * After registration, callers can store and load this log with
   * Database::Store and Database::Load.
   */
  void RegisterWithDatabase(cse498::Database& db)
    requires (std::same_as<T, double> || std::same_as<T, WorldPosition>)
  {
    using StoredData = std::unordered_map<std::string, std::vector<double>>;

    db.RegisterType<DataLog<T>>(
        std::same_as<T, WorldPosition> ? "DataLogWorldPosition" : "DataLogDouble",
        [](const DataLog<T>& log) -> std::string {
            cse498::Serializer s;
            StoredData data;
            if constexpr (std::same_as<T, double>) {
              data = log.mData;
            } else {
              for (const auto& [name, values] : log.mData) {
                for (const auto& value : values) {
                  data[name].push_back(value.X());
                  data[name].push_back(value.Y());
                }
              }
            }
            return s.Serialize(data);
        },
        [](const std::string& data) -> std::optional<DataLog<T>> {
            cse498::Serializer s;
            size_t pos = 0;
            auto parsed = s.DeserializeAt<StoredData>(data, pos);
            if (!parsed) return std::nullopt;

            DataLog<T> log;
            for (const auto& [name, coords] : *parsed) {
              if constexpr (std::same_as<T, double>) {
                log.mData[name] = coords;
              } else {
                if (coords.size() % 2 != 0) return std::nullopt;
                auto& values = log.mData[name];
                for (std::size_t i = 0; i < coords.size(); i += 2) {
                  values.emplace_back(coords[i], coords[i + 1]);
                }
              }
              log.mSnapshotCount = std::max(log.mSnapshotCount, log.mData[name].size());
            }
            return log;
        }
    );
  }
};

}  // namespace cse498
