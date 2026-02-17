#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/FeatureVector.h"

#include <memory>
#include <stdexcept>
#include <vector>

TEST_CASE("FeatureSchema basic behavior", "[group-05][FeatureVector]") {
  const FeatureSchema schema(std::vector<std::string>{"speed", "mass", "hp"});

  REQUIRE(schema.Size() == 3);
  CHECK(schema.Name(0) == "speed");
  CHECK(schema.Name(1) == "mass");
  CHECK(schema.Name(2) == "hp");

  const auto mass_idx = schema.IndexOf("mass");
  REQUIRE(mass_idx.has_value());
  CHECK(*mass_idx == 1);

  CHECK_FALSE(schema.IndexOf("missing").has_value());
  CHECK_FALSE(schema.IndexOf("").has_value());

  CHECK_THROWS_AS(schema.Name(3), std::out_of_range);
}

TEST_CASE("FeatureSchema validation and equality", "[group-05][FeatureVector]") {
  CHECK_THROWS_AS(FeatureSchema(std::vector<std::string>{"", "x"}), std::invalid_argument);
  CHECK_THROWS_AS(FeatureSchema(std::vector<std::string>{"x", "x"}), std::invalid_argument);

  const FeatureSchema a(std::vector<std::string>{"x", "y", "z"});
  const FeatureSchema b(std::vector<std::string>{"x", "y", "z"});
  const FeatureSchema c(std::vector<std::string>{"x", "z", "y"});
  const FeatureSchema d(std::vector<std::string>{"x", "y"});

  CHECK(a == b);
  CHECK_FALSE(a == c);
  CHECK_FALSE(a == d);
}

TEST_CASE("FeatureVector index and name access", "[group-05][FeatureVector]") {
  auto schema = std::make_shared<FeatureSchema>(std::vector<std::string>{"a", "b", "c"});
  FeatureVector vec(schema);

  REQUIRE(vec.Size() == 3);
  CHECK(vec.Get(0) == Approx(0.0));
  CHECK(vec.Get(1) == Approx(0.0));
  CHECK(vec.Get(2) == Approx(0.0));

  vec.Set(0, 1.5);
  vec.Set(1, -2.25);
  vec.Set(2, 9.0);
  CHECK(vec.Get(0) == Approx(1.5));
  CHECK(vec.Get(1) == Approx(-2.25));
  CHECK(vec.Get(2) == Approx(9.0));

  CHECK_THROWS_AS(vec.Get(3), std::out_of_range);
  CHECK_THROWS_AS(vec.Set(3, 7.0), std::out_of_range);

  CHECK(vec.TrySet("b", 4.75));

  const auto b_value = vec.GetByName("b");
  REQUIRE(b_value.has_value());
  CHECK(*b_value == Approx(4.75));

  CHECK_FALSE(vec.GetByName("missing").has_value());
  CHECK_FALSE(vec.GetByName("").has_value());
  CHECK_FALSE(vec.TrySet("missing", 3.0));
  CHECK_FALSE(vec.TrySet("", 3.0));

  vec.Clear();
  CHECK(vec.Get(0) == Approx(0.0));
  CHECK(vec.Get(1) == Approx(0.0));
  CHECK(vec.Get(2) == Approx(0.0));
}

TEST_CASE("FeatureVector dot product schema checks", "[group-05][FeatureVector]") {
  auto schema = std::make_shared<FeatureSchema>(std::vector<std::string>{"x", "y", "z"});
  FeatureVector lhs(schema);
  FeatureVector rhs(schema);

  lhs.Set(0, 2.0);
  lhs.Set(1, -3.0);
  lhs.Set(2, 4.0);

  rhs.Set(0, 5.0);
  rhs.Set(1, 1.5);
  rhs.Set(2, -2.0);

  const double expected = (2.0 * 5.0) + (-3.0 * 1.5) + (4.0 * -2.0);
  CHECK(lhs.Dot(rhs) == Approx(expected));

  auto same_names = std::make_shared<FeatureSchema>(std::vector<std::string>{"x", "y", "z"});
  FeatureVector structurally_equal(same_names);
  structurally_equal.Set(0, 1.0);
  structurally_equal.Set(1, 2.0);
  structurally_equal.Set(2, 3.0);

  CHECK(lhs.Dot(structurally_equal) == Approx((2.0 * 1.0) + (-3.0 * 2.0) + (4.0 * 3.0)));

  auto mismatched_schema = std::make_shared<FeatureSchema>(std::vector<std::string>{"x", "z", "q"});
  FeatureVector mismatched(mismatched_schema);
  CHECK_THROWS_AS(lhs.Dot(mismatched), std::invalid_argument);
}

TEST_CASE("FeatureVector rejects null schema", "[group-05][FeatureVector]") {
  CHECK_THROWS_AS(FeatureVector(std::shared_ptr<const FeatureSchema>{}), std::invalid_argument);
}
