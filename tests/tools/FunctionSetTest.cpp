#include "catch2/catch.hpp"
#include "../../source/tools/FunctionSet.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

using cse498::FunctionSet;

/*
 * DoubleValue
 *
 * Simple function pointer used to verify that FunctionSet correctly
 * accepts and invokes raw function pointers (not just lambdas).
 */
int DoubleValue(int x) {
  return x * 2;
}

/*
 * AddThree
 *
 * Functor (function object) used to verify that FunctionSet supports
 * callable objects with operator(). Ensures compatibility beyond
 * lambdas and function pointers.
 */
struct AddThree {
  int operator()(int x) const {
    return x + 3;
  }
};


TEST_CASE("FunctionSet starts empty and supports basic void behavior", "[tools]")
{
  FunctionSet<void()> fs;

  CHECK(fs.Empty());
  CHECK(fs.Size() == 0);

  int counter = 0;

  fs.Add([&]() { counter += 1; });
  fs.Add([&]() { counter += 2; });

  CHECK(!fs.Empty());
  CHECK(fs.Size() == 2);

  fs.CallAll();
  CHECK(counter == 3);

  fs.CallAll();
  CHECK(counter == 6);

  fs.Clear();
  CHECK(fs.Empty());
  CHECK(fs.Size() == 0);

  fs.CallAll();
  CHECK(counter == 6);
}

TEST_CASE("FunctionSet preserves insertion order", "[tools]")
{
  FunctionSet<void(int)> fs;
  std::vector<int> results;

  fs.Add([&](int x) { results.push_back(x + 1); });
  fs.Add([&](int x) { results.push_back(x + 2); });
  fs.Add([&](int x) { results.push_back(x + 3); });

  fs.CallAll(10);

  REQUIRE(results.size() == 3);
  CHECK(results[0] == 11);
  CHECK(results[1] == 12);
  CHECK(results[2] == 13);
}

TEST_CASE("FunctionSet supports const reference arguments", "[tools]")
{
  FunctionSet<void(const std::string&)> fs;

  std::string captured;

  fs.Add([&](const std::string& s) {
    captured = s;
  });

  std::string input = "hello world";
  fs.CallAll(input);

  CHECK(captured == "hello world");
}

TEST_CASE("FunctionSet preserves callable state across multiple calls", "[tools]")
{
  FunctionSet<void()> fs;

  int value = 0;

  fs.Add([&]() { value += 5; });

  fs.CallAll();
  CHECK(value == 5);

  fs.CallAll();
  CHECK(value == 10);

  fs.CallAll();
  CHECK(value == 15);
}

TEST_CASE("FunctionSet returns vector of results for non-void functions", "[tools]")
{
  FunctionSet<int(int)> fs;

  fs.Add([](int x) { return x + 1; });
  fs.Add([](int x) { return x + 2; });
  fs.Add([](int x) { return x + 3; });

  auto results = fs.CallAll(10);

  REQUIRE(results.size() == 3);
  CHECK(results[0] == 11);
  CHECK(results[1] == 12);
  CHECK(results[2] == 13);
}

TEST_CASE("FunctionSet supports function pointers", "[tools]")
{
  FunctionSet<int(int)> fs;

  fs.Add(DoubleValue);

  auto results = fs.CallAll(5);

  REQUIRE(results.size() == 1);
  CHECK(results[0] == 10);
}

TEST_CASE("FunctionSet supports functors", "[tools]")
{
  FunctionSet<int(int)> fs;

  fs.Add(AddThree{});

  auto results = fs.CallAll(5);

  REQUIRE(results.size() == 1);
  CHECK(results[0] == 8);
}

TEST_CASE("FunctionSet supports std bind expressions", "[tools]")
{
  FunctionSet<int(int)> fs;

  auto add = [](int a, int b) { return a + b; };
  fs.Add(std::bind(add, std::placeholders::_1, 7));

  auto results = fs.CallAll(5);

  REQUIRE(results.size() == 1);
  CHECK(results[0] == 12);
}

TEST_CASE("FunctionSet rejects null std function objects", "[tools]")
{
  FunctionSet<void()> fs;
  std::function<void()> fn;

  REQUIRE_THROWS_AS(fs.Add(fn), std::invalid_argument);
}

TEST_CASE("Calling empty non-void FunctionSet returns empty vector", "[tools]")
{
  FunctionSet<int(int)> fs;

  auto results = fs.CallAll(42);

  CHECK(results.empty());
}
