#pragma once

#include <cstddef>
#include <functional>
#include <set>
#include <unordered_map>

namespace cse498 {

struct Event {
  size_t id;
  double time_scheduled;
  double priority;
  std::function<void()> callback_action;

  bool operator<(const Event &other) const {
    if (time_scheduled != other.time_scheduled)
      return time_scheduled < other.time_scheduled;
    return id < other.id;
  }
};

class TimedEventQueue {
public:
  size_t Schedule(Event event);
  size_t ProcessDue(double current_time);
  size_t Cancel(size_t event_id);

private:
  std::set<Event> m_events;
  std::unordered_map<size_t, std::set<Event>::iterator> m_id_lookup;
};

}; // namespace cse498
