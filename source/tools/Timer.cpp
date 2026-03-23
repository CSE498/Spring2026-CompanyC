#include "Timer.hpp"

#include <cassert>     // assert()
#include <algorithm>   // sort

namespace cse498 {

void Timer::Start(const std::string& name) {
  // start is allowed to create the timer entry if it does not exist
  Entry& entry = mTimers[name];

  // Programmer error: starting an already-running timer.
  assert(!entry.running && "Timer::Start called while timer is already running");

  entry.running = true;
  entry.start = Clock::now();
}

void Timer::Stop(const std::string& name) {
  // stop should NOT create a new name implicitly
  auto it = mTimers.find(name);
  assert(it != mTimers.end() && "Timer::Stop called for unknown timer name");

  Entry& entry = it->second;

  // programmer error: stopping a timer that is not running.
  assert(entry.running && "Timer::Stop called while timer is not running");

  const auto end = Clock::now();
  const double seconds = std::chrono::duration<double>(end - entry.start).count();

  entry.running = false;

  // Update stats (completed runs only)
  Timer::Stats& s = entry.stats;
  s.lastSeconds = seconds;
  s.totalSeconds += seconds;
  ++s.count;

  if (s.count == 1) {
    s.minSeconds = seconds;
    s.maxSeconds = seconds;
  } else {
    if (seconds < s.minSeconds) s.minSeconds = seconds;
    if (seconds > s.maxSeconds) s.maxSeconds = seconds;
  }
}

void Timer::Reset(const std::string& name) {
  mTimers.erase(name);
}

void Timer::ResetAll() {
  mTimers.clear();
}

bool Timer::HasData(const std::string& name) const {
  auto it = mTimers.find(name);
  return (it != mTimers.end()) && (it->second.stats.count > 0);
}

double Timer::Last(const std::string& name) const {
  auto it = mTimers.find(name);
  if (it == mTimers.end() || it->second.stats.count == 0) return 0.0;
  return it->second.stats.lastSeconds;
}

double Timer::Min(const std::string& name) const {
  auto it = mTimers.find(name);
  if (it == mTimers.end() || it->second.stats.count == 0) return 0.0;
  return it->second.stats.minSeconds;
}

double Timer::Max(const std::string& name) const {
  auto it = mTimers.find(name);
  if (it == mTimers.end() || it->second.stats.count == 0) return 0.0;
  return it->second.stats.maxSeconds;
}

double Timer::Average(const std::string& name) const {
  auto it = mTimers.find(name);
  if (it == mTimers.end() || it->second.stats.count == 0) return 0.0;

  const auto& s = it->second.stats;
  return s.totalSeconds / static_cast<double>(s.count);
}

std::size_t Timer::Count(const std::string& name) const {
  auto it = mTimers.find(name);
  if (it == mTimers.end()) return 0;
  return it->second.stats.count;
}

Timer::Stats Timer::GetStats(const std::string& name) const {
  // returns a copy
  auto it = mTimers.find(name);
  if (it == mTimers.end()) return Stats{};
  return it->second.stats;
}

std::vector<std::pair<std::string, Timer::Stats>> Timer::GetAllStats() const {
  // export all stats as value types so other modules can render/print/store them
  std::vector<std::pair<std::string, Timer::Stats>> out;
  out.reserve(mTimers.size());

  for (const auto& [name, entry] : mTimers) {
    out.push_back({name, entry.stats});
  }
  return out;
}

} // namespace cse498