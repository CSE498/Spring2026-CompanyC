#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/ExpressionParser.hpp"

TEST_CASE("ExpressionParser basic parsing and evaluation", "[worlds]") {
  using cse498::ExpressionParser;
  ExpressionParser::VarMap vars;

  SECTION("Parses constant integers and doubles") {
    auto f = ExpressionParser::Parse("42");
    CHECK(f(vars) == 42.0);

    auto g = ExpressionParser::Parse("3.5");
    CHECK(g(vars) == 3.5);
  }

  SECTION("Parses a single variable") {
    auto f = ExpressionParser::Parse("x");
    vars = {{"x", 9.0}};
    CHECK(f(vars) == 9.0);
  }

  SECTION("Respects operator precedence and parentheses") {
    // 2 + 3*4 = 14
    auto f = ExpressionParser::Parse("2 + 3 * 4");
    vars.clear();
    CHECK(f(vars) == 14.0);

    // (2+3)*4 = 20
    auto g = ExpressionParser::Parse("(2 + 3) * 4");
    CHECK(g(vars) == 20.0);
  }

  SECTION("Handles whitespace") {
    auto f = ExpressionParser::Parse(" (  x +  2 ) * 3 ");
    vars = {{"x", 4.0}};
    CHECK(f(vars) == 18.0);
  }

  SECTION("Handles multiple variables and operators") {
    auto f = ExpressionParser::Parse("a*b + c/d");
    vars = {{"a", 2.0}, {"b", 5.0}, {"c", 9.0}, {"d", 3.0}};
    CHECK(f(vars) == 13.0);
  }
}

TEST_CASE("ExpressionParser error behavior", "[worlds]") {
  using cse498::ExpressionParser;

  SECTION("Unknown variable throws at evaluation") {
    auto f = ExpressionParser::Parse("x + 1");
    ExpressionParser::VarMap vars;  // empty
    CHECK_THROWS_AS(f(vars), std::runtime_error);
  }

  SECTION("Division by zero throws at evaluation") {
    auto f = ExpressionParser::Parse("10 / x");
    ExpressionParser::VarMap vars{{"x", 0.0}};
    CHECK_THROWS_AS(f(vars), std::runtime_error);
  }

  SECTION("Mismatched parentheses throws at parse time") {
    CHECK_THROWS_AS(ExpressionParser::Parse("(1 + 2"), std::invalid_argument);
    CHECK_THROWS_AS(ExpressionParser::Parse("1 + 2)"), std::invalid_argument);
  }

  SECTION("Invalid character throws at parse time") {
    CHECK_THROWS_AS(ExpressionParser::Parse("2 + $"), std::invalid_argument);
  }

}


