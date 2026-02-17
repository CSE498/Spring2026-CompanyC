#include "../../source/tools/DataLog.hpp"

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

TEST_CASE("DataLog tracks numeric stats and reset behavior", "[core]") {
  cse498::DataLog log;

  SECTION("Test empty") {
    CHECK(log.IsEmpty() == true);
    CHECK(log.Count() == 0);
  }

  SECTION("Test single value") {
    log.Add(42.5);

    REQUIRE(log.IsEmpty() == false);
    REQUIRE(log.Count() == 1);

    CHECK(log.Mean() == Approx(42.5));
    CHECK(log.Median() == Approx(42.5));
    CHECK(log.Min() == Approx(42.5));
    CHECK(log.Max() == Approx(42.5));
  }

  SECTION("Test mean") {
    log.Add(10.0);
    log.Add(2.0);
    log.Add(4.0);
    log.Add(-2.0);
    log.Add(8.0);

    REQUIRE(log.Count() == 5);
    CHECK(log.Mean() == Approx(4.4));
  }

  SECTION("Test min") {
    log.Add(10.0);
    log.Add(2.0);
    log.Add(4.0);
    log.Add(-2.0);
    log.Add(8.0);

    REQUIRE(log.Count() == 5);
    CHECK(log.Min() == Approx(-2.0));
  }

  SECTION("Test max") {
    log.Add(10.0);
    log.Add(2.0);
    log.Add(4.0);
    log.Add(-2.0);
    log.Add(8.0);

    REQUIRE(log.Count() == 5);
    CHECK(log.Max() == Approx(10.0));
  }

  SECTION("Test medians") {
    // odd sized median
    cse498::DataLog odd_log;
    odd_log.Add(10.0);
    odd_log.Add(2.0);
    odd_log.Add(4.0);
    odd_log.Add(-2.0);
    odd_log.Add(8.0);

    REQUIRE(odd_log.Count() == 5);
    CHECK(odd_log.Median() == Approx(4.0));

    // even sized median, takes average of middle 2 values
    cse498::DataLog even_log;
    even_log.Add(1.0);
    even_log.Add(3.0);
    even_log.Add(9.0);
    even_log.Add(5.0);

    REQUIRE(even_log.Count() == 4);
    CHECK(even_log.Median() == Approx(4.0));
  }

  SECTION("Test clear") {
    log.Add(7.0);
    log.Add(11.0);
    log.Clear();

    CHECK(log.IsEmpty() == true);
    CHECK(log.Count() == 0);

    log.Add(3.0);
    REQUIRE(log.IsEmpty() == false);
    REQUIRE(log.Count() == 1);
    CHECK(log.Mean() == Approx(3.0));
    CHECK(log.Min() == Approx(3.0));
    CHECK(log.Max() == Approx(3.0));
  }
}
