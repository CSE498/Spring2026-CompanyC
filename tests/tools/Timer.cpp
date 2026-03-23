#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/Timer.hpp"

#include <chrono>
#include <thread>
#include <string>

using namespace cse498;

//edge cases to add 
//start(A) called twice without stopping
//stop(A) called before Start(A) 
TEST_CASE("Timer records named durations and simple stats", "[core]")
{
  Timer timer; //create fresh timer instance for each test case run

  SECTION("HasData is false until a measurement is recorded") //testing behavior before any timings are recorded
  {
    CHECK(timer.HasData("A") == false); // asking about a timer that hasnt been started/stopped
    CHECK(timer.Count("A") == 0); //no completed runs recorded

    //stats on unknown/empty timer should return 0 for all below stats
    CHECK(timer.Last("A") == 0.0);
    CHECK(timer.Min("A") == 0.0);
    CHECK(timer.Max("A") == 0.0);
    CHECK(timer.Average("A") == 0.0);
  }
  SECTION("Reset on an unknown timer does nothing") //added from peer review
  {
    CHECK(timer.HasData("NeverStarted") == false);
    CHECK(timer.Count("NeverStarted") == 0);

    timer.Reset("NeverStarted"); // should not crash or change anything

    CHECK(timer.HasData("NeverStarted") == false);
    CHECK(timer.Count("NeverStarted") == 0);
    CHECK(timer.Last("NeverStarted") == 0.0);
    CHECK(timer.Min("NeverStarted") == 0.0);
    CHECK(timer.Max("NeverStarted") == 0.0);
    CHECK(timer.Average("NeverStarted") == 0.0);
  }

  SECTION("Querying a timer while it is still running returns no completed data") //added from peer review
  {
    timer.Start("A");

    CHECK(timer.HasData("A") == false);
    CHECK(timer.Count("A") == 0);
    CHECK(timer.Last("A") == 0.0);
    CHECK(timer.Min("A") == 0.0);
    CHECK(timer.Max("A") == 0.0);
    CHECK(timer.Average("A") == 0.0);

    timer.Stop("A");
  }

  SECTION("Single timing run updates count and last") //testing one start/stop
  {
    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(2)); //so measured time >0
    timer.Stop("A");

    REQUIRE(timer.HasData("A") == true);
    REQUIRE(timer.Count("A") == 1);

    const double last = timer.Last("A");
    CHECK(last > 0.0); //duration >0 (sleep for)

    CHECK(timer.Min("A") == Approx(last));
    CHECK(timer.Max("A") == Approx(last));
    CHECK(timer.Average("A") == Approx(last));
  }

  SECTION("Multiple runs update min/max/average") //multiple runs should update min/max/avg
  {
    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(2)); //shorter
    timer.Stop("A");

    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(6)); //longer to test min!=max
    timer.Stop("A");

    REQUIRE(timer.Count("A") == 2);

    const double minv = timer.Min("A");
    const double maxv = timer.Max("A");
    const double avgv = timer.Average("A");

    CHECK(minv > 0.0);
    //avg should be btwn min and max
    CHECK(maxv >= minv); 
    CHECK(avgv >= minv);
    CHECK(avgv <= maxv);
  }

  SECTION("Different names track independently") //multiple timer names should not interfere 
  {
    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    timer.Stop("A");

    timer.Start("B");
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    timer.Stop("B");

    //each timer should have its own count (1 for A 1 for B)
    REQUIRE(timer.Count("A") == 1);
    REQUIRE(timer.Count("B") == 1);

    CHECK(timer.Last("A") > 0.0);
    CHECK(timer.Last("B") > 0.0);
  }

  SECTION("Reset clears a single timer name")
  {
    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    timer.Stop("A");

    REQUIRE(timer.HasData("A") == true); //check a has data before reset.

    timer.Reset("A");

    //after reset, A should act like it was never recorded
    CHECK(timer.HasData("A") == false);
    CHECK(timer.Count("A") == 0);
    CHECK(timer.Last("A") == 0.0);
  }

  SECTION("ResetAll clears all timers") //resets all timers
  {
    timer.Start("A");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    timer.Stop("A");

    timer.Start("B");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    timer.Stop("B");

    REQUIRE(timer.HasData("A") == true); //check both have data before reset all
    REQUIRE(timer.HasData("B") == true);

    timer.ResetAll();

    CHECK(timer.HasData("A") == false);
    CHECK(timer.HasData("B") == false);
    CHECK(timer.Count("A") == 0);
    CHECK(timer.Count("B") == 0);
  }
}
