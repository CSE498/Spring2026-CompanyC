
#include <catch2/catch_test_macros.hpp>

#include "../Classes/PathGenerator.hpp"
#include <iostream>



TEST_CASE( "TEST" ) {
    WorldView world(5, 5);
    PathGenerator pg;
    pg.SetWorldView(world);

    PathRequest req;
    req.type = PathType::Shortest;
    req.start = {0, 0};
    req.goal  = {4, 4};

    // Complete Path
    WorldPath path = pg.GeneratePath(req);

    REQUIRE_FALSE(path.Empty());
    REQUIRE(path.Points().front() == req.start);
    REQUIRE(path.Points().back()  == req.goal);
    REQUIRE(path.Length() == 9);

    // Path Out of Bounds
    PathRequest req2;
    req.type = PathType::Shortest;
    req.start = {4, 4};
    req.goal = {5, 5};

    WorldPath path2 = pg.GeneratePath(req);
    REQUIRE(path2.Empty() == true);
    REQUIRE(path2.Length() == 0);

    //std::cout << world << "\n";
    //std::cout << path  << "\n";
}