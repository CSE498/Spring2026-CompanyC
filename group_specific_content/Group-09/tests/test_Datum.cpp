#define CATCH_CONFIG_MAIN
#include "/catch2/catch.hpp"
#include "../source/Datum.hpp"


TEST_CASE("Test Datum Constructors", "[core]")
{
    SECTION("Default constructor creates NaN")
    {
        Datum d;
        CHECK(std::isnan(d.AsDouble()));
    }
}
