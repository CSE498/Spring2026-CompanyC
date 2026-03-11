#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../../source/core/Database.hpp"
#include <iostream>
using namespace cse498;


TEST_CASE("Test Database Constructors", "[core]"){


    SECTION("Size defaults to zero"){
        Database db;
        REQUIRE(db.Size() == 0);
    }
    SECTION("No entries exist yet"){
        Database db;
        REQUIRE_FALSE(db.Exists("Any key"));
    }
    SECTION()


}