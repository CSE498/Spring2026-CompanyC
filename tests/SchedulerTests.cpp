#define CATCH_CONFIG_MAIN
#include "../third-party/Catch/single_include/catch2/catch.hpp"
#include "../source/core/Scheduler.hpp"
#include "../source/core/TieredScheduler.hpp"

using namespace cse498;

// =============================================================================
// Base Scheduler Tests
// =============================================================================

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
    REQUIRE(count1 > count2 * 5);  // At least 5x more (some margin)
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
    REQUIRE(count1 == count2);  // Should be 10 each

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

// =============================================================================
// TieredScheduler Tests
// =============================================================================

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

    ts.ResetFrameBudgets(100.0);  // Plenty of budget

    // Critical should run first
    REQUIRE(ts.GetNext() == 3);
}

TEST_CASE("TieredScheduler: Tiers run in priority order", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::COSMETIC, 1.0);
    ts.AddProcess(2, ProcessTier::ECONOMY, 1.0);
    ts.AddProcess(3, ProcessTier::GAMEPLAY, 1.0);
    ts.AddProcess(4, ProcessTier::CRITICAL, 1.0);

    ts.ResetFrameBudgets(1000.0);  // Large budget

    // With large budget, should go: CRITICAL -> GAMEPLAY -> ECONOMY -> COSMETIC
    // But within stride scheduler, need to exhaust higher tiers first
    size_t first = ts.GetNext();
    REQUIRE(first == 4);  // CRITICAL first
}

TEST_CASE("TieredScheduler: Budget exhaustion moves to next tier", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);  // CRITICAL gets 40ms, GAMEPLAY gets 30ms

    // Run critical and record time that exhausts its budget
    size_t id = ts.GetNext();
    REQUIRE(id == 1);
    ts.RecordExecutionTime(50.0);  // Exceeds 40ms budget

    // But CRITICAL has soft budget, so it might still run
    // Let's exhaust the soft budget too
    ts.RecordExecutionTime(250.0);  // Now at -260ms, past soft limit

    // Now should move to GAMEPLAY
    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: Soft budget allows CRITICAL to overshoot", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);  // CRITICAL gets 40ms

    // Exhaust normal budget but stay within soft limit
    size_t id = ts.GetNext();
    REQUIRE(id == 1);
    ts.RecordExecutionTime(50.0);  // Now at -10ms (within soft 250ms limit)

    // CRITICAL should still run (soft budget)
    id = ts.GetNext();
    REQUIRE(id == 1);
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

    // Now CRITICAL should be skipped, GAMEPLAY runs
    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: Non-critical tiers have hard budget", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::GAMEPLAY, 1.0);
    ts.AddProcess(2, ProcessTier::ECONOMY, 1.0);

    ts.ResetFrameBudgets(100.0);  // GAMEPLAY gets 30ms, ECONOMY gets 20ms

    size_t id = ts.GetNext();
    REQUIRE(id == 1);  // GAMEPLAY

    // Exhaust GAMEPLAY budget (no soft budget for non-critical)
    ts.RecordExecutionTime(35.0);  // Now at -5ms

    // Should skip to ECONOMY immediately
    id = ts.GetNext();
    REQUIRE(id == 2);
}

TEST_CASE("TieredScheduler: RemoveProcess works across tiers", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);

    ts.RemoveProcess(1);

    // CRITICAL tier empty, should go to GAMEPLAY
    REQUIRE(ts.GetNext() == 2);
}

TEST_CASE("TieredScheduler: Multiple processes in same tier use stride", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 10.0);  // high priority
    ts.AddProcess(2, ProcessTier::CRITICAL, 1.0);   // low priority

    ts.ResetFrameBudgets(1000.0);  // Large budget

    int count1 = 0, count2 = 0;
    for (int i = 0; i < 50; ++i) {
        size_t id = ts.GetNext();
        if (id == 1) count1++;
        else if (id == 2) count2++;
        ts.RecordExecutionTime(0.1);  // Tiny execution time
    }

    // Process 1 should run much more often
    REQUIRE(count1 > count2 * 5);
}

TEST_CASE("TieredScheduler: ResetFrameBudgets restores all budgets", "[tiered]") {
    TieredScheduler ts;
    ts.AddProcess(1, ProcessTier::CRITICAL, 1.0);
    ts.AddProcess(2, ProcessTier::GAMEPLAY, 1.0);

    ts.ResetFrameBudgets(100.0);

    // Exhaust all budgets
    (void)ts.GetNext();
    ts.RecordExecutionTime(500.0);
    (void)ts.GetNext();
    ts.RecordExecutionTime(500.0);

    // Reset
    ts.ResetFrameBudgets(100.0);

    // Should be able to run CRITICAL again
    REQUIRE(ts.GetNext() == 1);
}
