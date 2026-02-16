#pragma once

namespace cse498 {

struct Event {
  size_t id;
  double priority;
  std::function<void> callback_action;
};

class TimedEventQueue {
public:
  size_t Schedule(Event event);
  std::set ProcessDue(current_time);
  size_t Cancel(event_id);
};

}; // namespace cse498
