/**
 * @file DataLog.hpp
 * @author Muhammad Chohan
 *
 * @brief Tracks a series of useful data values over time and returns statistics
 *        such as mean, median, min, and max.
 */
#pragma once

#include <vector>
#include <queue>  

class DataLog {
private:
  /// data storage
  std::vector<double> dataValues;

public:
  // Constructor/Destructor
  DataLog();
  ~DataLog();

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
   * @brief Clear all values
   */
  void Clear();

  // More functions may be added later for other useful statistics.
};
