#include "../../source/tools/DataLog.hpp"

#include "catch2/catch.hpp"

#include <initializer_list>
#include <limits>
#include <stdexcept>

TEST_CASE("DataLog tracks numeric stats and reset behavior", "[core]") {
  cse498::DataLog log;
  const auto add_values = [](cse498::DataLog& target,
                             std::initializer_list<double> values) {
    for (double value : values) {
      target.Add(value);
    }
  };

  SECTION("Test empty") {
    CHECK(log.IsEmpty());
    CHECK(log.Count() == 0);
    CHECK_THROWS_AS(log.Mean(), std::logic_error);
    CHECK_THROWS_AS(log.Median(), std::logic_error);
    CHECK_THROWS_AS(log.Min(), std::logic_error);
    CHECK_THROWS_AS(log.Max(), std::logic_error);
  }

  SECTION("Test invalid inputs") {
    CHECK_THROWS_AS(log.Add(std::numeric_limits<double>::quiet_NaN()),
                    std::invalid_argument);
    CHECK_THROWS_AS(log.Add(std::numeric_limits<double>::infinity()),
                    std::invalid_argument);
    CHECK_THROWS_AS(log.Add(-std::numeric_limits<double>::infinity()),
                    std::invalid_argument);

    CHECK(log.IsEmpty());
    CHECK(log.Count() == 0);
  }

  SECTION("Test single value") {
    log.Add(42.5);

    CHECK_FALSE(log.IsEmpty());
    REQUIRE(log.Count() == 1);

    CHECK(log.Mean() == Approx(42.5));
    CHECK(log.Median() == Approx(42.5));
    CHECK(log.Min() == Approx(42.5));
    CHECK(log.Max() == Approx(42.5));
  }

  SECTION("Test mean") {
    add_values(log, {10.0, 2.0, 4.0, -2.0, 8.0});

    REQUIRE(log.Count() == 5);
    CHECK(log.Mean() == Approx(4.4));
  }

  SECTION("Test min") {
    add_values(log, {10.0, 2.0, 4.0, -2.0, 8.0});

    REQUIRE(log.Count() == 5);
    CHECK(log.Min() == Approx(-2.0));
  }

  SECTION("Test max") {
    add_values(log, {10.0, 2.0, 4.0, -2.0, 8.0});

    REQUIRE(log.Count() == 5);
    CHECK(log.Max() == Approx(10.0));
  }

  SECTION("Test medians") {
    const auto check_median = [&](std::initializer_list<double> values,
                                  double expected) {
      cse498::DataLog median_log;
      add_values(median_log, values);
      REQUIRE(median_log.Count() == values.size());
      CHECK(median_log.Median() == Approx(expected));
    };

    // odd sized median
    check_median({10.0, 2.0, 4.0, -2.0, 8.0}, 4.0);

    // even sized median, takes average of middle 2 values
    check_median({1.0, 3.0, 9.0, 5.0}, 4.0);

    // handles duplicates
    check_median({5.0, 5.0, 1.0, 5.0, 9.0}, 5.0);

    // handles already-sorted and reverse-sorted inputs
    check_median({1.0, 2.0, 3.0, 4.0, 5.0}, 3.0);
    check_median({5.0, 4.0, 3.0, 2.0, 1.0}, 3.0);

    // handles the two-value case with very large finite values
    check_median({-std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max()},
                 0.0);
  }

  SECTION("Test clear") {
    log.Add(7.0);
    log.Add(11.0);
    log.Clear();

    CHECK(log.IsEmpty());
    CHECK(log.Count() == 0);
    CHECK_THROWS_AS(log.Mean(), std::logic_error);
    CHECK_THROWS_AS(log.Median(), std::logic_error);
    CHECK_THROWS_AS(log.Min(), std::logic_error);
    CHECK_THROWS_AS(log.Max(), std::logic_error);

    log.Clear();
    CHECK(log.IsEmpty());
    CHECK(log.Count() == 0);
    CHECK_THROWS_AS(log.Median(), std::logic_error);

    log.Add(3.0);
    CHECK_FALSE(log.IsEmpty());
    REQUIRE(log.Count() == 1);
    CHECK(log.Mean() == Approx(3.0));
    CHECK(log.Median() == Approx(3.0));
    CHECK(log.Min() == Approx(3.0));
    CHECK(log.Max() == Approx(3.0));
  }
}
