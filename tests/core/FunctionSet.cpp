#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/FunctionSet.hpp"

#include <string>
#include <vector>

TEST_CASE("Test FunctionSet Basic Behavior", "[tools]")
{
    FunctionSet<void()> fs;

    CHECK(fs.Empty());

    int counter = 0;

    fs.Add([&]() { counter += 1; });
    fs.Add([&]() { counter += 2; });

    CHECK(!fs.Empty());

    fs.CallAll();
    CHECK(counter == 3);

    fs.CallAll();
    CHECK(counter == 6);

    fs.Clear();
    CHECK(fs.Empty());

    fs.CallAll(); // should do nothing
    CHECK(counter == 6);
}

TEST_CASE("Test FunctionSet Call Order", "[tools]")
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

TEST_CASE("Test FunctionSet With Const Reference Argument", "[tools]")
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

TEST_CASE("Test FunctionSet Multiple Calls Preserve State", "[tools]")
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
