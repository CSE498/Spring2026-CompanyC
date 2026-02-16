#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/scheduling/Scheduler.hpp"

using namespace cse498;

TEST_CASE("Scheduler: Empty scheduler returns NULL_ID", "[scheduler]") {
    Scheduler s;
    REQUIRE(s.GetNext() == Scheduler::NULL_ID);
    REQUIRE(s.IsEmpty());
}

TEST_CASE("Scheduler: Single process always returns same ID", "[scheduler]") {
    Scheduler s;
    s.AddProcess(42, 1.0);

    REQUIRE(s.GetNext() == 42);
    REQUIRE(s.GetNext() == 42);
    REQUIRE(s.GetNext() == 42);
}

TEST_CASE("Scheduler: Higher priority runs more often", "[scheduler]") {
    Scheduler s;
    s.AddProcess(1, 10.0);  // high priority
    s.AddProcess(2, 1.0);   // low priority

    int count1 = 0, count2 = 0;
    for (int i = 0; i < 100; ++i) {
        size_t id = s.GetNext();
        if (id == 1) count1++;
        else if (id == 2) count2++;
    }

    // Process 1 should run ~10x more than process 2
    REQUIRE(count1 > count2 * 5);
}

TEST_CASE("Scheduler: Remove process works", "[scheduler]") {
    Scheduler s;
    s.AddProcess(1, 1.0);
    s.AddProcess(2, 1.0);

    s.RemoveProcess(1);

    REQUIRE(s.GetNumProcesses() == 1);
    REQUIRE(s.GetNext() == 2);
    REQUIRE(s.GetNext() == 2);
}

TEST_CASE("Scheduler: UpdatePriority changes scheduling frequency", "[scheduler]") {
    Scheduler s;
    s.AddProcess(1, 1.0);
    s.AddProcess(2, 1.0);

    // Initially equal
    int count1 = 0, count2 = 0;
    for (int i = 0; i < 20; ++i) {
        size_t id = s.GetNext();
        if (id == 1) count1++;
        else count2++;
    }
    REQUIRE(count1 == count2);

    // Now boost process 1
    s.UpdatePriority(1, 10.0);

    count1 = 0; count2 = 0;
    for (int i = 0; i < 100; ++i) {
        size_t id = s.GetNext();
        if (id == 1) count1++;
        else count2++;
    }
    REQUIRE(count1 > count2 * 5);
}

TEST_CASE("Scheduler: PeekNext doesn't advance", "[scheduler]") {
    Scheduler s;
    s.AddProcess(1, 1.0);
    s.AddProcess(2, 1.0);

    size_t first_peek = s.PeekNext();
    size_t second_peek = s.PeekNext();
    REQUIRE(first_peek == second_peek);

    size_t actual = s.GetNext();
    REQUIRE(actual == first_peek);
}

TEST_CASE("Scheduler: Clear removes all processes", "[scheduler]") {
    Scheduler s;
    s.AddProcess(1, 1.0);
    s.AddProcess(2, 1.0);
    s.AddProcess(3, 1.0);

    s.Clear();

    REQUIRE(s.IsEmpty());
    REQUIRE(s.GetNumProcesses() == 0);
    REQUIRE(s.GetNext() == Scheduler::NULL_ID);
}
