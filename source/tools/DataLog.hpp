/**
 * @file DataLog.hpp
 * @author Muhammad Chohan
 *
 * @brief Tracks a series of useful data values over time and returns statistics
 *        such as mean, median, min, and max.
 */
#pragma once

#include <cstddef>
#include <vector>

namespace cse498 {

class DataLog {
 private:
  /// Stores the logged values used by the statistical queries.
  std::vector<double> mDataValues;
  double mSum = 0.0; // currently tracked for calculating mean 
  double mMin = 0.0;
  double mMax = 0.0;

 public:
  DataLog() = default;

  /**
   * @brief Add a data value to the log
   *
   * @param value The finite value to add
   *
   * @throws std::invalid_argument If value is NaN or infinite.
   */
  void Add(double value);

  /**
   * @brief Return the mean of values.
   *
   * @throws std::logic_error If the log is empty.
   */
  [[nodiscard]] double Mean() const;

  /**
   * @brief Return the median of values.
   *
   * @throws std::logic_error If the log is empty.
   */
  [[nodiscard]] double Median() const;

  /**
   * @brief Return the minimum value.
   *
   * @throws std::logic_error If the log is empty.
   */
  [[nodiscard]] double Min() const;

  /**
   * @brief Return the maximum value.
   *
   * @throws std::logic_error If the log is empty.
   */
  [[nodiscard]] double Max() const;

  /**
   * @brief Number of values currently in the log.
   */
  [[nodiscard]] std::size_t Count() const noexcept;

  /**
   * @brief Returns true if log is empty, else false.
   */
  [[nodiscard]] bool IsEmpty() const noexcept;

  /**
   * @brief Removes all logged values and resets tracked statistics.
   */
  void Clear() noexcept;

  // More functions may be added later for other useful statistics.
};

}  
