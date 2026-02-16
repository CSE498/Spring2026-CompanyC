#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/DataMap.hpp"


TEST_CASE("Test DataMap Functionality", "[core]")
{
    cse498::DataMap data_map;

    // Test setting and getting data
    data_map.SetData("integer", 42);
    data_map.SetData("string", std::string("Hello, World!"));
    data_map.SetData("double", 3.14);

    REQUIRE(data_map.GetData<int>("integer") == 42);
    REQUIRE(data_map.GetData<std::string>("string") == "Hello, World!");
    REQUIRE(data_map.GetData<double>("double") == Approx(3.14));

    // Test Contains function
    REQUIRE(data_map.Contains("integer") == true);
    REQUIRE(data_map.Contains("string") == true);
    REQUIRE(data_map.Contains("double") == true);
    REQUIRE(data_map.Contains("nonexistent") == false);

    // Test RemoveData function
    data_map.RemoveData("integer");
    REQUIRE(data_map.Contains("integer") == false);

    // Test Clear function
    data_map.Clear();
    REQUIRE(data_map.Contains("string") == false);
    REQUIRE(data_map.Contains("double") == false);
}