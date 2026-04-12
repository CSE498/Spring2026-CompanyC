#pragma once

#include "Scheduler.hpp"
#include <array>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace cse498 {

enum class ProcessTier { CRITICAL, GAMEPLAY, ECONOMY, COSMETIC, COUNT };

constexpr double SOFT_LIMIT_MS = 250.0;

class TieredScheduler {
public:
  /**
   * @brief Initialize with the budget percentages from Phase 1.1
   */
  TieredScheduler() {
    // Budgets: CRITICAL 40%, GAMEPLAY 30%, ECONOMY 20%, COSMETIC 10%
    m_tier_configs[static_cast<size_t>(ProcessTier::CRITICAL)].weight = 0.4;
    m_tier_configs[static_cast<size_t>(ProcessTier::GAMEPLAY)].weight = 0.3;
    m_tier_configs[static_cast<size_t>(ProcessTier::ECONOMY)].weight = 0.2;
    m_tier_configs[static_cast<size_t>(ProcessTier::COSMETIC)].weight = 0.1;

    for (auto &config : m_tier_configs) {
      config.remaining_ms = 0.0;
    }
  }

  /**
   * @brief Registers a process into a specific scheduler tier
   */
  void AddProcess(size_t id, ProcessTier tier, double priority) {
    if (m_id_to_tier.find(id) != m_id_to_tier.end()) {
      throw std::invalid_argument("Duplicate process ID");
    }
    size_t idx = static_cast<size_t>(tier);
    m_tiers[idx].AddProcess(id, priority);
    m_id_to_tier[id] = tier;
  }

  /**
   * @brief Refills the 'remaining_ms' for each tier at frame start
   */
  void ResetFrameBudgets(double total_frame_time_ms) {
    assert(total_frame_time_ms > 0 &&
           "total_frame_time_ms must be positive");
    for (size_t i = 0; i < static_cast<size_t>(ProcessTier::COUNT); ++i) {
      m_tier_configs[i].remaining_ms =
          total_frame_time_ms * m_tier_configs[i].weight;
    }
  }

  /**
   * @brief Finds the next process, respecting tier priority and remaining
   * budget
   */
  [[nodiscard]] size_t GetNext() {
    // Loop through tiers: CRITICAL -> GAMEPLAY -> ECONOMY -> COSMETIC
    for (size_t i = 0; i < static_cast<size_t>(ProcessTier::COUNT); ++i) {
      bool is_critical = (i == static_cast<size_t>(ProcessTier::CRITICAL));
      bool within_soft_budget =
          is_critical && m_tier_configs[i].remaining_ms > -SOFT_LIMIT_MS;
      // Check if this tier has any budget left
      // soft budget for CRITICAL to make the simulation smooth
      // hard budget for other tiers
      if (m_tier_configs[i].remaining_ms > 0 || within_soft_budget) {
        size_t id = m_tiers[i].GetNext();

        if (id != Scheduler::NULL_ID) {
          m_current_running_tier = static_cast<ProcessTier>(i);
          return id;
        }
      }
    }
    return Scheduler::NULL_ID;
  }

  /**
   * @brief Subtracts execution time from the tier that just ran
   */
  void RecordExecutionTime(double actual_ms) {
    if (!m_current_running_tier.has_value()) {
      throw std::logic_error(
          "RecordExecutionTime called before a successful GetNext()");
    }
    size_t idx = static_cast<size_t>(m_current_running_tier.value());
    m_tier_configs[idx].remaining_ms -= actual_ms;
  }

  /**
   * @brief Finds the correct tier and removes the process
   */
  void RemoveProcess(size_t id) {
    auto it = m_id_to_tier.find(id);
    if (it != m_id_to_tier.end()) {
      m_tiers[static_cast<size_t>(it->second)].RemoveProcess(id);
      m_id_to_tier.erase(it);
    }
  }

private:
  struct TierConfig {
    double weight{};
    double remaining_ms{};
  };

  // The core scheduler engines (Composition)
  std::array<Scheduler, static_cast<size_t>(ProcessTier::COUNT)> m_tiers;

  // Budget tracking
  std::array<TierConfig, static_cast<size_t>(ProcessTier::COUNT)>
      m_tier_configs;

  // Quick lookup for RemoveProcess
  std::unordered_map<size_t, ProcessTier> m_id_to_tier;

  // Track which tier was last picked by GetNext()
  std::optional<ProcessTier> m_current_running_tier;
};

} // namespace cse498
