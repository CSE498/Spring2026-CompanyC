#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "../core/AgentBase.hpp"

namespace cse498 {

  class SmartAgent : public AgentBase {
  protected:
    static constexpr size_t max_plan_length = 8;

    std::array<size_t, max_plan_length> planned_actions{};
    size_t planned_action_count = 0;
    bool plan_requested = false;
    std::string last_notification_message;
    std::string last_notification_type = "none";

    [[nodiscard]] bool IsKnownAction(size_t action_id) const noexcept
    {
      if (action_id == 0) return true;

      for (const auto & action_entry : action_map) {
        if (action_entry.second == action_id) return true;
      }

      return false;
    }

    void RequestPlan(const WorldGrid & grid)
    {
      (void) grid;

      plan_requested = true;

      // here is where our llm call will be implemented, including parsing it's response.
    }

    [[nodiscard]] size_t PopNextAction() noexcept
    {
      const size_t next_action = planned_actions[0];

      for (size_t i = 1; i < planned_action_count; ++i) {
        planned_actions[i - 1] = planned_actions[i];
      }

      --planned_action_count;
      return next_action;
    }

  public:
    SmartAgent(size_t id, const std::string & name, const WorldBase & world)
      : AgentBase(id, name, world) { }
    ~SmartAgent() = default;

    [[nodiscard]] bool Initialize() override { return true; }

    [[nodiscard]] bool HasPlan() const noexcept { return planned_action_count != 0; }
    [[nodiscard]] bool IsPlanRequested() const noexcept { return plan_requested; }
    [[nodiscard]] size_t GetPlannedActionCount() const noexcept { return planned_action_count; }

    bool QueuePlannedAction(size_t action_id) noexcept
    {
      if (action_id == 0 || !IsKnownAction(action_id)) return false;
      if (planned_action_count >= planned_actions.size()) return false;

      planned_actions[planned_action_count++] = action_id;
      plan_requested = false;
      return true;
    }

    void ClearPlan() noexcept
    {
      planned_action_count = 0;
      plan_requested = false;
    }

    [[nodiscard]] const std::string & GetLastNotificationMessage() const noexcept
    {
      return last_notification_message;
    }

    [[nodiscard]] const std::string & GetLastNotificationType() const noexcept
    {
      return last_notification_type;
    }

    [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override
    {
      if (action_result == 0) ClearPlan();

      if (!HasPlan()) {
        if (!plan_requested) RequestPlan(grid);
        return 0;
      }

      return PopNextAction();
    }

    void Notify(const std::string & message, const std::string & msg_type="none") override
    {
      last_notification_message = message;
      last_notification_type = msg_type;
      ClearPlan();
    }
  };

}
