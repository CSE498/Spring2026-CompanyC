#include "catch2/catch.hpp"

#include "../../source/tools/PathGenerator.hpp"

static cse498::PathRequest MakeReq(
    cse498::PathType type,
    cse498::StateGridPosition start,
    cse498::StateGridPosition goal
) {
    cse498::PathRequest r;
    r.type  = type;
    r.start = start;
    r.goal  = goal;
    return r;
}

TEST_CASE("PathGenerator basic path") {

    cse498::WorldView world(5, 5);
    cse498::PathGenerator pg;
    pg.SetWorldView(world);

    const cse498::PathRequest req =
        MakeReq(cse498::PathType::Shortest, {0, 0}, {4, 4});

    cse498::WorldPath path = pg.GeneratePath(req);

    REQUIRE(path.length() > 0);

    REQUIRE(path.start().x == req.start.GetX());
    REQUIRE(path.start().y == req.start.GetY());

    REQUIRE(path.end().x == req.goal.GetX());
    REQUIRE(path.end().y == req.goal.GetY());
}

TEST_CASE("PathGenerator edge cases") {

    SECTION("start equals goal") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {2, 2}, {2, 2});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);

        REQUIRE(path.start().x == 2);
        REQUIRE(path.start().y == 2);

        REQUIRE(path.end().x == 2);
        REQUIRE(path.end().y == 2);
    }

    SECTION("start out of bounds") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {5, 0}, {4, 4});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);
    }

    SECTION("goal out of bounds") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {5, 5});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);
    }

    SECTION("world view not set") {
        cse498::PathGenerator pg;

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {1, 1});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);
    }

    SECTION("empty world") {
        cse498::WorldView world(0, 0);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {0, 0});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);
    }

    SECTION("unreachable goal") {
        cse498::WorldView world(5, 5);

        world.SetBlocked({1, 0});
        world.SetBlocked({0, 1});

        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const auto req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {4, 4});

        cse498::WorldPath path = pg.GeneratePath(req);

        REQUIRE(path.length() == 0);
    }
}