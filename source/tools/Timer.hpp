#pragma once

#include <cstddef>       // size_t
#include <chrono>        // steady_clock, time_point, dur
#include <string>        // string
#include <unordered_map> // unordered_map
#include <utility>       // pair
#include <vector>        // vector
#include <type_traits>   // is_void_v

namespace cse498{

class Timer {
public:
  // snapshot of 1 timer's stats
  struct Stats {
    std::size_t count = 0;     //num of completed measurements
    double totalSeconds = 0.0; //sum of completed durs
    double lastSeconds = 0.0;  //most recent completed dur
    double minSeconds = 0.0;   //fastest completed dur
    double maxSeconds = 0.0;   //slowest completed dur
  };

  Timer() = default;
  ~Timer() = default;
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) noexcept = default;
  Timer& operator=(Timer&&) noexcept = default;

  void Start(const std::string& name);
  void Stop(const std::string& name);
  void Reset(const std::string& name);
  void ResetAll();

  bool HasData(const std::string& name) const;
  double Last(const std::string& name) const;
  double Min(const std::string& name) const;
  double Max(const std::string& name) const;
  double Average(const std::string& name) const;
  std::size_t Count(const std::string& name) const;

  // optional helper API to make Timer easier to integrate later on... may change based on company discussion/preference.
  Stats GetStats(const std::string& name) const;
  std::vector<std::pair<std::string, Stats>> GetAllStats() const;

  // Time a function/lambda under a name.
  // Works for both void and non-void returns. Utilized ChatGPT for this chunk: originally lambda only supported non void returns. Need to look into this more to 1. understand 2. test functionality
  template <typename Func, typename... Args>
  auto TimeCall(const std::string& name, Func&& fn, Args&&... args) {
    Start(name);

    // always call stop even on early exit/exceptions
    struct StopGuard {
      Timer& timer;
      const std::string& n;
      ~StopGuard() { timer.Stop(n); }
    } guard{*this, name};

    if constexpr (std::is_void_v<decltype(fn(std::forward<Args>(args)...))>) {
      fn(std::forward<Args>(args)...);
      return; // void
    } else {
      return fn(std::forward<Args>(args)...); // non-void
    }
  }

private:
  using Clock = std::chrono::steady_clock;

  struct Entry {
    bool running = false;          // true between Start and Stop
    Clock::time_point start{};     // start time for current run
    Stats stats{};                // accumulated stats for completed runs
  };

  std::unordered_map<std::string, Entry> mTimers;
};

} // namespace cse498