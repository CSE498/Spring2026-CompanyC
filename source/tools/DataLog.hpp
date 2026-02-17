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
  /// data storage
  std::vector<double> mDataValues;
  double mSum = 0.0;
  double mMin = 0.0;
  double mMax = 0.0;

 public:
  DataLog() = default;
  ~DataLog() = default;

  /**
   * @brief Add a data value to the log
   *
   * @param value The value to add
   */
  void Add(double value);

  /**
   * @brief Return the mean of values.
   */
  double Mean() const;

  /**
   * @brief Return the median of values.
   */
  double Median() const;

  /**
   * @brief Return the minimum value.
   */
  double Min() const;

  /**
   * @brief Return the maximum value.
   */
  double Max() const;

  /**
   * @brief Number of values currently in the log.
   */
  std::size_t Count() const;

  /**
   * @brief Returns true if log is empty, else false.
   */
  bool IsEmpty() const;

  /**
   * @brief Clear all values
   */
  void Clear();

  // More functions may be added later for other useful statistics.
};

}  