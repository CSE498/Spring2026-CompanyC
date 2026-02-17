/**
 * @file DataLog.cpp
 * @author Muhammad Chohan
 *
 * @brief Tracks a series of useful data values over time and returns statistics
 *        such as mean, median, min, and max.
 */
#include "DataLog.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

namespace cse498 {

void DataLog::Add(double value) {
  assert(std::isfinite(value));

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
  assert(!mDataValues.empty());
  return mSum / mDataValues.size();
}

double DataLog::Median() const {
  assert(!mDataValues.empty());

  // first sort the data values
  std::vector<double> sorted_values = mDataValues;
  std::sort(sorted_values.begin(), sorted_values.end());

  // find the middle of the sorted values
  const std::size_t middle = sorted_values.size() / 2;

  // if odd just return middle
  if (sorted_values.size() % 2 == 1) {
    return sorted_values[middle];
  }

  // if even then return average of the 2 middle values using std::midpoint
  const double upper = sorted_values[middle];
  const double lower = sorted_values[middle - 1];
  return std::midpoint(lower, upper);
}

double DataLog::Min() const {
  assert(!mDataValues.empty());
  return mMin;
}

double DataLog::Max() const {
  assert(!mDataValues.empty());
  return mMax;
}

std::size_t DataLog::Count() const { return mDataValues.size(); }

bool DataLog::IsEmpty() const { return mDataValues.empty(); }

void DataLog::Clear() {
  mDataValues.clear();
  mSum = 0.0;
  mMin = 0.0;
  mMax = 0.0;
}

}  
