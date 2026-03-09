/**
 * @file DataLog.cpp
 * @author Muhammad Chohan
 *
 * @brief Tracks a series of useful data values over time and returns statistics
 *        such as mean, median, min, and max.
 */
#include "DataLog.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace cse498 {

void DataLog::Add(double value) {
  if (!std::isfinite(value)) {
    throw std::invalid_argument("DataLog only accepts finite values.");
  }

  mDataValues.push_back(value);
  mSum += value;

  if (mDataValues.size() == 1) {
    mMin = value;
    mMax = value;
    return;
  }

  if (value < mMin) mMin = value;
  if (value > mMax) mMax = value;
}

double DataLog::Mean() const {
  if (mDataValues.empty()) {
    throw std::logic_error("Mean requires at least one logged value.");
  }
  return mSum / mDataValues.size();
}

double DataLog::Median() const {
  if (mDataValues.empty()) {
    throw std::logic_error("Median requires at least one logged value.");
  }

  // Partition around the middle instead of fully sorting the copy.
  std::vector<double> partitioned_values = mDataValues;
  const auto middle_it =
      partitioned_values.begin() + (partitioned_values.size() / 2);
  std::nth_element(partitioned_values.begin(), middle_it,
                   partitioned_values.end());

  if (partitioned_values.size() % 2 == 1) {
    return *middle_it;
  }

  // For even counts, the lower median is the largest value in the lower
  // partition, while the upper median is the middle element itself.
  const double upper = *middle_it;
  const double lower =
      *std::max_element(partitioned_values.begin(), middle_it);
  return std::midpoint(lower, upper);
}

double DataLog::Min() const {
  if (mDataValues.empty()) {
    throw std::logic_error("Min requires at least one logged value.");
  }
  return mMin;
}

double DataLog::Max() const {
  if (mDataValues.empty()) {
    throw std::logic_error("Max requires at least one logged value.");
  }
  return mMax;
}

std::size_t DataLog::Count() const noexcept { return mDataValues.size(); }

bool DataLog::IsEmpty() const noexcept { return mDataValues.empty(); }

void DataLog::Clear() noexcept {
  mDataValues.clear();
  mSum = 0.0;
  mMin = 0.0;
  mMax = 0.0;
}

}  
