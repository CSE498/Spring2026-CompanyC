/**
 * @file Scheduler.hpp
 * @brief Stride scheduler for running processes at different frequencies.
 *
 * Think of it like a race where slower runners get a head start. Each process
 * has a "pass" value (how far they've run) and a "stride" (step size). We
 * always pick whoever is furthest behind. High priority = small steps = falls
 * behind quickly = gets picked more often.
 *
 * Example: A market (priority 10) updates 10x more often than grass growth
 * (priority 1). Both make progress, but the market gets more attention.
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <set>
#include <unordered_map>

namespace cse498 {

class Scheduler {
public:
  // Returned when scheduler is empty (no valid process to run)
  static constexpr size_t NULL_ID = std::numeric_limits<size_t>::max();

private:
  // Used to calculate stride. Stride = this / priority.
  // Bigger number = finer granularity between priorities.
  static constexpr double LARGE_CONSTANT = 1'000'000.0;

  // What we store in the sorted set. Lower pass = runs next.
  struct ProcessEntry {
    double pass;
    size_t id;

    // Sort by pass first. Use id as tiebreaker so behavior is deterministic.
    bool operator<(const ProcessEntry &other) const {
      if (pass != other.pass)
        return pass < other.pass;
      return id < other.id;
    }
  };

  // Everything we need to know about a process.
  struct ProcessInfo {
    double priority; // What the user set
    double
        stride;  // How much pass increases each time (smaller = more frequent)
    double pass; // Current position in virtual time
  };

  // Sorted container - begin() is always the next process to run.
  // Using set instead of priority_queue because we need to remove/update
  // entries.
  std::set<ProcessEntry> schedule_;

  // Quick lookup of process info by ID.
  std::unordered_map<size_t, ProcessInfo> processes_;

  // Get the lowest pass value, or 0 if empty.
  // New processes start here so they don't immediately hog the scheduler.
  [[nodiscard]] double GetMinPass() const {
    if (schedule_.empty())
      return 0.0;
    return schedule_.begin()->pass;
  }

public:
  Scheduler() = default;

  // -------------------------------------------------------------------------
  // Core API
  // -------------------------------------------------------------------------

  // Add a process to the scheduler.
  // Priority must be > 0. Higher priority = scheduled more often.
  void AddProcess(size_t id, double priority) {
    assert(priority > 0.0 && "Priority must be positive");
    assert(processes_.find(id) == processes_.end() && "Duplicate process ID");

    double stride = LARGE_CONSTANT / priority;
    double pass = GetMinPass();

    processes_[id] = ProcessInfo{priority, stride, pass};
    schedule_.insert(ProcessEntry{pass, id});
  }

  // Get the next process to run. Returns NULL_ID if scheduler is empty.
  // This also advances that process's virtual time (so it won't run again
  // immediately).
  [[nodiscard]] size_t GetNext() {
    if (schedule_.empty()) {
      return NULL_ID;
    }

    auto it = schedule_.begin();
    size_t id = it->id;
    schedule_.erase(it);

    ProcessInfo &info = processes_.at(id);
    info.pass += info.stride;

    schedule_.insert(ProcessEntry{info.pass, id});

    return id;
  }

  // Change a process's priority on the fly.
  // Useful when something becomes more/less important (e.g., player walks near
  // it).
  void UpdatePriority(size_t id, double new_priority) {
    assert(new_priority > 0.0 && "Priority must be positive");
    assert(processes_.find(id) != processes_.end() && "Process not found");

    ProcessInfo &info = processes_.at(id);

    schedule_.erase(ProcessEntry{info.pass, id});

    info.priority = new_priority;
    info.stride = LARGE_CONSTANT / new_priority;

    schedule_.insert(ProcessEntry{info.pass, id});
  }

  // Remove a process completely (e.g., entity was destroyed).
  void RemoveProcess(size_t id) {
    assert(processes_.find(id) != processes_.end() && "Process not found");

    ProcessInfo &info = processes_.at(id);
    schedule_.erase(ProcessEntry{info.pass, id});
    processes_.erase(id);
  }

  // -------------------------------------------------------------------------
  // Utility
  // -------------------------------------------------------------------------

  [[nodiscard]] size_t GetNumProcesses() const { return processes_.size(); }
  [[nodiscard]] bool IsEmpty() const { return processes_.empty(); }
  [[nodiscard]] bool HasProcess(size_t id) const {
    return processes_.find(id) != processes_.end();
  }

  [[nodiscard]] double GetPriority(size_t id) const {
    assert(processes_.find(id) != processes_.end() && "Process not found");
    return processes_.at(id).priority;
  }

  // See what's next without actually running it.
  [[nodiscard]] size_t PeekNext() const {
    if (schedule_.empty())
      return NULL_ID;
    return schedule_.begin()->id;
  }

  void Clear() {
    schedule_.clear();
    processes_.clear();
  }
};

} // namespace cse498
