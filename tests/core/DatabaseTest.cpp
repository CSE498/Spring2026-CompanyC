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
   


}
TEST_CASE("Load and Store", "[core]"){

    SECTION("Store a string and load it back"){
        Database db;
        std::string s{"Hello World"};
        bool stored = db.Store("test", s);
        REQUIRE(stored);

        auto result = db.Load<std::string>("test");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "Hello World");
    }

    SECTION("Load non-existent key returns nullopt"){
        Database db;
        auto result = db.Load<std::string>("missing");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Store increments size"){
        Database db;
        db.Store("key", std::string{"value"});
        REQUIRE(db.Size() == 1);
    }


}