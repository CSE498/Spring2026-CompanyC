#include "../../source/tools/DataLog.hpp"

#include <initializer_list>
#include <limits>
#include <string>
#include <unordered_map>

#include "../../source/core/WorldPosition.hpp"
#include "catch2/catch.hpp"

TEST_CASE("DataLog tracks named numeric stats and reset behavior", "[core]") {
  cse498::DataLog log;
  const auto add_values = [](cse498::DataLog<>& target,
                             const std::string& name,
                             std::initializer_list<double> values) {
    for (double value : values) {
      target.Add(name, value);
    }
  };

  SECTION("Test empty") {
    const auto stats = log.Summary("score");

    CHECK(log.Names().empty());
    CHECK(log.Values("score").empty());
    CHECK(stats.count == 0);
    CHECK(stats.sum == Approx(0.0));
    CHECK(stats.mean == Approx(0.0));
    CHECK(stats.median == Approx(0.0));
    CHECK(stats.min == Approx(0.0));
    CHECK(stats.max == Approx(0.0));
  }

  SECTION("Test invalid inputs") {
    log.Add("score", std::numeric_limits<double>::quiet_NaN());
    log.Add("score", std::numeric_limits<double>::infinity());
    log.Add("score", -std::numeric_limits<double>::infinity());

    const auto stats = log.Summary("score");

    CHECK(log.Names().empty());
    CHECK(log.Values("score").empty());
    CHECK(stats.count == 0);
    CHECK(stats.sum == Approx(0.0));
    CHECK(stats.mean == Approx(0.0));
    CHECK(stats.median == Approx(0.0));
    CHECK(stats.min == Approx(0.0));
    CHECK(stats.max == Approx(0.0));
  }

  SECTION("Test invalid inputs do not mutate existing data") {
    log.Add("score", 10.0);
    log.Add("score", std::numeric_limits<double>::quiet_NaN());
    log.Add("score", std::numeric_limits<double>::infinity());
    log.Add("score", -std::numeric_limits<double>::infinity());

    const auto stats = log.Summary("score");

    CHECK(log.Names().size() == 1);
    REQUIRE(log.Values("score").size() == 1);
    CHECK(stats.count == 1);
    CHECK(stats.sum == Approx(10.0));
    CHECK(stats.mean == Approx(10.0));
    CHECK(stats.median == Approx(10.0));
    CHECK(stats.min == Approx(10.0));
    CHECK(stats.max == Approx(10.0));
  }

  SECTION("Test single value") {
    log.Add("score", 42.5);

    const auto stats = log.Summary("score");

    CHECK(log.Names().size() == 1);
    REQUIRE(log.Values("score").size() == 1);

    CHECK(stats.count == 1);
    CHECK(stats.sum == Approx(42.5));
    CHECK(stats.mean == Approx(42.5));
    CHECK(stats.median == Approx(42.5));
    CHECK(stats.min == Approx(42.5));
    CHECK(stats.max == Approx(42.5));
  }

  SECTION("Test mean") {
    add_values(log, "score", {10.0, 2.0, 4.0, -2.0, 8.0});

    const auto stats = log.Summary("score");

    REQUIRE(stats.count == 5);
    CHECK(stats.sum == Approx(22.0));
    CHECK(stats.mean == Approx(4.4));
  }

  SECTION("Test min") {
    add_values(log, "score", {10.0, 2.0, 4.0, -2.0, 8.0});

    const auto stats = log.Summary("score");

    REQUIRE(stats.count == 5);
    CHECK(stats.min == Approx(-2.0));
  }

  SECTION("Test max") {
    add_values(log, "score", {10.0, 2.0, 4.0, -2.0, 8.0});

    const auto stats = log.Summary("score");

    REQUIRE(stats.count == 5);
    CHECK(stats.max == Approx(10.0));
  }

  SECTION("Test medians") {
    const auto check_median = [&](std::initializer_list<double> values,
                                  double expected) {
      cse498::DataLog median_log;
      add_values(median_log, "score", values);

      const auto stats = median_log.Summary("score");

      REQUIRE(stats.count == values.size());
      CHECK(stats.median == Approx(expected));
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

  SECTION("Test named values are independent") {
    add_values(log, "score", {10.0, 20.0});
    add_values(log, "health", {3.0, 6.0, 9.0});

    const auto names = log.Names();
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "health");
    CHECK(names[1] == "score");

    const auto score_stats = log.Summary("score");
    const auto health_stats = log.Summary("health");

    REQUIRE(score_stats.count == 2);
    CHECK(score_stats.mean == Approx(15.0));
    CHECK(score_stats.min == Approx(10.0));
    CHECK(score_stats.max == Approx(20.0));

    REQUIRE(health_stats.count == 3);
    CHECK(health_stats.mean == Approx(6.0));
    CHECK(health_stats.min == Approx(3.0));
    CHECK(health_stats.max == Approx(9.0));

    const auto& score_values = log.Values("score");
    REQUIRE(score_values.size() == 2);
    CHECK(score_values[0] == Approx(10.0));
    CHECK(score_values[1] == Approx(20.0));
  }

  SECTION("Test clear") {
    log.Add("score", 7.0);
    log.Add("health", 11.0);
    log.Clear();

    const auto stats = log.Summary("score");

    CHECK(log.Names().empty());
    CHECK(log.Values("score").empty());
    CHECK(stats.count == 0);
    CHECK(stats.sum == Approx(0.0));
    CHECK(stats.mean == Approx(0.0));
    CHECK(stats.median == Approx(0.0));
    CHECK(stats.min == Approx(0.0));
    CHECK(stats.max == Approx(0.0));

    log.Clear();
    CHECK(log.Names().empty());
    CHECK(log.Values("health").empty());
    CHECK(log.Summary("health").median == Approx(0.0));

    log.Add("score", 3.0);

    const auto new_stats = log.Summary("score");

    CHECK(log.Names().size() == 1);
    REQUIRE(new_stats.count == 1);
    CHECK(new_stats.mean == Approx(3.0));
    CHECK(new_stats.median == Approx(3.0));
    CHECK(new_stats.min == Approx(3.0));
    CHECK(new_stats.max == Approx(3.0));
  }
}

TEST_CASE("DataLog supports named snapshots", "[core]") {
  cse498::DataLog log;

  SECTION("Test unordered map snapshot") {
    std::unordered_map<std::string, double> values;
    values["score"] = 10.0;
    values["health"] = 5.0;

    log.AddSnapshot(values);
    log.AddSnapshot(values);

    const auto names = log.Names();
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "health");
    CHECK(names[1] == "score");

    const auto score_stats = log.Summary("score");
    const auto health_stats = log.Summary("health");

    REQUIRE(score_stats.count == 2);
    CHECK(score_stats.sum == Approx(20.0));
    CHECK(score_stats.mean == Approx(10.0));

    REQUIRE(health_stats.count == 2);
    CHECK(health_stats.sum == Approx(10.0));
    CHECK(health_stats.mean == Approx(5.0));
  }
}

TEST_CASE("DataLog supports arithmetic templates beyond double", "[core]") {
  cse498::DataLog<int> log;

  log.Add("integers", 2);
  log.Add("integers", 4);
  log.Add("integers", 8);
  log.Add("integers", 10);

  const auto stats = log.Summary("integers");

  REQUIRE(stats.count == 4);
  CHECK(stats.sum == Approx(24.0));
  CHECK(stats.mean == Approx(6.0));
  CHECK(stats.median == Approx(6.0));
  CHECK(stats.min == 2);
  CHECK(stats.max == 10);
}

TEST_CASE("DataLog can store generic position samples", "[core]") {
  cse498::DataLog<cse498::WorldPosition> log;

  log.Add("player", cse498::WorldPosition{0.10, 0.20});
  log.Add("player", cse498::WorldPosition{0.90, 0.80});
  log.Add("player", cse498::WorldPosition{1.10, 0.40});
  log.Add("nonplayer", cse498::WorldPosition{2.10, 2.20});

  const auto names = log.Names();
  REQUIRE(names.size() == 2);
  CHECK(names[0] == "nonplayer");
  CHECK(names[1] == "player");

  const auto& player_values = log.Values("player");
  REQUIRE(player_values.size() == 3);
  CHECK(player_values[0] == cse498::WorldPosition{0.10, 0.20});
  CHECK(player_values[1] == cse498::WorldPosition{0.90, 0.80});
  CHECK(player_values[2] == cse498::WorldPosition{1.10, 0.40});

  const auto& nonplayer_values = log.Values("nonplayer");
  REQUIRE(nonplayer_values.size() == 1);
  CHECK(nonplayer_values[0] == cse498::WorldPosition{2.10, 2.20});

  CHECK(log.Values("missing").empty());

  log.Clear();
  CHECK(log.Names().empty());
  CHECK(log.Values("player").empty());
  CHECK(log.Values("nonplayer").empty());
}
