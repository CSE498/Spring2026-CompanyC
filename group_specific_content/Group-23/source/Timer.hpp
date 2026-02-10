/**
 * @file Timer.hpp
 * @author Lauren Phillips
 *
 * @brief Utility class for timing named sections of code and tracking simple stats. 
 * 
 * Note: implementation may change based on stats preferred by company.
 */

#pragma once
#include <chrono>
#include <string>
#include <unordered_map>

class Timer {
private:
  using Clock = std::chrono::steady_clock;

  struct TimerEntry {
    bool running = false;
    Clock::time_point start{};

    //stats for completed runs
    std::size_t count=0;
    double totalSeconds=0.0;
    double lastSeconds=0.0;
    double minSeconds=0.0;
    double maxSeconds=0.0;
  };

  ///timers kayed by name
  std::unordered_map<std::string,TimerEntry> mTimers;

public:
  Timer();
  ~Timer();

  /**
   * @brief begin timing a named section
   *
   */
  void Start(const std::string& name);

  /**
   * @brief stop timing a section and record the duration
   *
   */
  void Stop(const std::string& name);

  /**
   * @brief clear stored results
   *
   */
  void Reset(const std::string& name);

  /**
   * @brief clear ALL timers and recorded stats
   */
  void ResetAll();

  /**
   * @brief true if the timer has at least 1 measutement
   */
  bool HasData(const std::string& name)const;

  /**
   * @brief most recent recorded duration in seconds
   * NOTE: returns 0.0 if no data
   */
  double Last(const std::string& name) const;

  /**
   * @brief Fastest recorded run in seconds
   * NOTE: returns 0.0 if no data
   */
  double Min(const std::string& name)const;

  /**
   * @brief Slowest recorded run in seconds 
   * NOTE: returns 0.0 if no data
   */
  double Max(const std::string& name)const;

  /**
   * @brief avg duration across all runs in seconds
   * NOTE: returns 0.0 if no data
   */
  double Average(const std::string& name) const;

  /**
   * @brief # of completed runs
   * NOTE: returns 0.0 if no data
   */
  std::size_t Count(const std::string& name) const;
};
