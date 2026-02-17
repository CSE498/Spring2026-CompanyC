#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/scheduling/TieredScheduler.hpp"

using namespace cse498;

TEST_CASE("TieredScheduler: Empty scheduler returns NULL_ID", "[tiered]") {
    TieredScheduler ts;
    ts.ResetFrameBudgets(16.0);
    REQUIRE(ts.GetNext() == Scheduler::NULL_ID);
}

TEST_CASE("TieredScheduler: Critical tier runs before others", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::COSMETIC, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);
    ts.AddProcess(3, ProcessTier::CRITICAL, 1.0);

    ts.ResetFrameBudgets(100.0);

    REQUIRE(ts.GetNext() == 3);
}

TEST_CASE("TieredScheduler: Tiers run in priority order", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::COSMETIC, 1.0);
    ts.AddProcess(2, ProcessTier::ECONOMY, 1.0);
    ts.AddProcess(3, ProcessTier::GAMEPLAY, 1.0);
    ts.AddProcess(4, ProcessTier::CRITICAL, 1.0);

    ts.ResetFrameBudgets(1000.0);

    REQUIRE(ts.GetNext() == 4);  // CRITICAL first
}

TEST_CASE("TieredScheduler: Budget exhaustion moves to next tier", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);  // CRITICAL gets 40ms

    size_t id = ts.GetNext();
    REQUIRE(id == 1);
    ts.RecordExecutionTime(50.0);   // Exceeds 40ms budget
    ts.RecordExecutionTime(250.0);  // Exceeds soft limit too

    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: Soft budget allows CRITICAL to overshoot", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);  // CRITICAL gets 40ms

    size_t id = ts.GetNext();
    REQUIRE(id == 1);
    ts.RecordExecutionTime(50.0);  // Now at -10ms (within soft 250ms limit)

    id = ts.GetNext();
    REQUIRE(id == 1);  // CRITICAL should still run
}

TEST_CASE("TieredScheduler: Soft budget has a limit", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);  // CRITICAL gets 40ms

    size_t id = ts.GetNext();
    REQUIRE(id == 1);

    // Blow way past soft budget (40 - 300 = -260, which is < -250)
    ts.RecordExecutionTime(300.0);

    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: Non-critical tiers have hard budget", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::GAMEPLAY, 1.0);
    ts.AddProcess(2, ProcessTier::ECONOMY, 1.0);

    ts.ResetFrameBudgets(100.0);  // GAMEPLAY gets 30ms

    size_t id = ts.GetNext();
    REQUIRE(id == 1);

    ts.RecordExecutionTime(35.0);  // Exceeds budget, no soft limit

    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: RemoveProcess works across tiers", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);

    ts.RemoveProcess(1);

    REQUIRE(ts.GetNext() == 2);
}

TEST_CASE("TieredScheduler: Multiple processes in same tier use stride", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 10.0);
    ts.AddProcess(2, ProcessTier::CRITICAL, 1.0);

    ts.ResetFrameBudgets(1000.0);

    int count1 = 0, count2 = 0;
    for (int i = 0; i < 50; ++i) {
        size_t id = ts.GetNext();
        if (id == 1) count1++;
        else if (id == 2) count2++;
        ts.RecordExecutionTime(0.1);
    }

    REQUIRE(count1 > count2 * 5);
}

TEST_CASE("TieredScheduler: ResetFrameBudgets restores all budgets", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);

    (void)ts.GetNext();
    ts.RecordExecutionTime(500.0);
    (void)ts.GetNext();
    ts.RecordExecutionTime(500.0);

    ts.ResetFrameBudgets(100.0);

    REQUIRE(ts.GetNext() == 1);
}
