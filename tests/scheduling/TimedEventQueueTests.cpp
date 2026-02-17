#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/scheduling/TimedEventQueue.hpp"

using namespace cse498;

// Helper to track callback execution
static std::vector<size_t> executed_events;

static void reset_tracking() {
    executed_events.clear();
}

static Event make_event(size_t id, double time) {
    return Event{
        .id = id,
        .time_scheduled = time,
        .priority = 1.0,
        .callback_action = [id]() { executed_events.push_back(id); }
    };
}

TEST_CASE("TimedEventQueue: Empty queue processes nothing", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    size_t count = eq.ProcessDue(100.0);

    REQUIRE(count == 0);
    REQUIRE(executed_events.empty());
}

TEST_CASE("TimedEventQueue: Schedule returns unique IDs", "[eventqueue]") {
    TimedEventQueue eq;

    size_t id1 = eq.Schedule(make_event(1, 10.0));
    size_t id2 = eq.Schedule(make_event(2, 20.0));
    size_t id3 = eq.Schedule(make_event(3, 30.0));

    REQUIRE(id1 != id2);
    REQUIRE(id2 != id3);
    REQUIRE(id1 != id3);
}

TEST_CASE("TimedEventQueue: ProcessDue executes events at or before current time", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    eq.Schedule(make_event(1, 10.0));
    eq.Schedule(make_event(2, 20.0));
    eq.Schedule(make_event(3, 30.0));

    size_t count = eq.ProcessDue(25.0);

    REQUIRE(count == 2);
    REQUIRE(executed_events.size() == 2);
    REQUIRE(executed_events[0] == 1);
    REQUIRE(executed_events[1] == 2);
}

TEST_CASE("TimedEventQueue: Events execute in time order", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    // Schedule out of order
    eq.Schedule(make_event(3, 30.0));
    eq.Schedule(make_event(1, 10.0));
    eq.Schedule(make_event(2, 20.0));

    eq.ProcessDue(100.0);

    REQUIRE(executed_events.size() == 3);
    REQUIRE(executed_events[0] == 1);
    REQUIRE(executed_events[1] == 2);
    REQUIRE(executed_events[2] == 3);
}

TEST_CASE("TimedEventQueue: Cancel removes scheduled event", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    eq.Schedule(make_event(1, 10.0));
    size_t id2 = eq.Schedule(make_event(2, 20.0));
    eq.Schedule(make_event(3, 30.0));

    size_t cancelled = eq.Cancel(id2);
    REQUIRE(cancelled == 1);

    eq.ProcessDue(100.0);

    REQUIRE(executed_events.size() == 2);
    REQUIRE(executed_events[0] == 1);
    REQUIRE(executed_events[1] == 3);
}

TEST_CASE("TimedEventQueue: Cancel nonexistent event returns 0", "[eventqueue]") {
    TimedEventQueue eq;

    size_t cancelled = eq.Cancel(99999);
    REQUIRE(cancelled == 0);
}

TEST_CASE("TimedEventQueue: ProcessDue removes executed events", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    eq.Schedule(make_event(1, 10.0));
    eq.Schedule(make_event(2, 20.0));

    eq.ProcessDue(15.0);  // Executes event 1
    REQUIRE(executed_events.size() == 1);

    reset_tracking();
    eq.ProcessDue(15.0);  // Event 1 already gone
    REQUIRE(executed_events.empty());

    eq.ProcessDue(25.0);  // Now event 2
    REQUIRE(executed_events.size() == 1);
    REQUIRE(executed_events[0] == 2);
}

TEST_CASE("TimedEventQueue: Same-time events execute in ID order", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    eq.Schedule(make_event(3, 10.0));
    eq.Schedule(make_event(1, 10.0));
    eq.Schedule(make_event(2, 10.0));

    eq.ProcessDue(10.0);

    REQUIRE(executed_events.size() == 3);
    REQUIRE(executed_events[0] == 1);
    REQUIRE(executed_events[1] == 2);
    REQUIRE(executed_events[2] == 3);
}

TEST_CASE("TimedEventQueue: Future events not executed", "[eventqueue]") {
    TimedEventQueue eq;
    reset_tracking();

    eq.Schedule(make_event(1, 100.0));

    eq.ProcessDue(50.0);

    REQUIRE(executed_events.empty());
}
