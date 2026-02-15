#include <catch2/catch_test_macros.hpp>

#include "../Classes/WorldPath.hpp"


static void RequirePointEq(const Point& p, double x, double y) {
    REQUIRE(p.x == x);
    REQUIRE(p.y == y);
}

TEST_CASE("WorldPath basic operations", "[WorldPath]") {

    SECTION("Empty path has length 0 and is not self-intersecting") {
        WorldPath p;
        REQUIRE(p.length() == 0.0);
        REQUIRE_FALSE(p.self_intersect());
    }

    SECTION("Single point: start=end and length is 0") {
        WorldPath p;
        p.Add({1.0, 2.0});

        REQUIRE(p.length() == 0.0);
        REQUIRE_FALSE(p.self_intersect());

        RequirePointEq(p.start(), 1.0, 2.0);
        RequirePointEq(p.end(),   1.0, 2.0);
    }

    SECTION("Two points: length is Euclidean distance and start/end are correct") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({3.0, 4.0});

        REQUIRE(p.length() == 5.0);
        REQUIRE_FALSE(p.self_intersect());

        RequirePointEq(p.start(), 0.0, 0.0);
        RequirePointEq(p.end(),   3.0, 4.0);
    }

    SECTION("Multiple segments: length is sum of segment distances") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({3.0, 4.0});
        p.Add({6.0, 8.0});

        REQUIRE(p.length() == 10.0);
        REQUIRE_FALSE(p.self_intersect());
    }

    SECTION("clear resets path length to 0") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({1.0, 1.0});
        REQUIRE(p.length() > 0.0);

        p.clear();
        REQUIRE(p.length() == 0.0);
        REQUIRE_FALSE(p.self_intersect());
    }
}

TEST_CASE("WorldPath self-intersection", "[WorldPath]") {

    SECTION("Fewer than 4 points cannot self-intersect") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({2.0, 2.0});
        p.Add({3.0, 3.0});

        REQUIRE_FALSE(p.self_intersect());
    }

    SECTION("Simple crossing polyline should be detected") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({2.0, 2.0});
        p.Add({0.0, 2.0});
        p.Add({2.0, 0.0});
        REQUIRE(p.self_intersect() == true);
    }

    SECTION("Non-intersecting polyline returns false") {
        WorldPath p;
        p.Add({0.0, 0.0});
        p.Add({1.0, 0.0});
        p.Add({2.0, 0.0});
        p.Add({3.0, 0.0});

        REQUIRE_FALSE(p.self_intersect());
    }
}