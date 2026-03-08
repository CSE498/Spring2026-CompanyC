// PathGeneratorTests.cpp
#include "catch2/catch.hpp"

#include "../../source/tools/PathGenerator.hpp"

static cse498::PathRequest MakeReq(
    cse498::PathType type,
    cse498::Position start,
    cse498::Position goal
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

    cse498::SampleWorldPath path = pg.GeneratePath(req);

    REQUIRE_FALSE(path.Empty());
    REQUIRE(path.Points().front() == req.start);
    REQUIRE(path.Points().back()  == req.goal);
    REQUIRE(path.Length() == 9);
}

TEST_CASE("PathGenerator edge cases") {

    SECTION("start equals goal") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {2, 2}, {2, 2});

        cse498::SampleWorldPath path = pg.GeneratePath(req);

        REQUIRE_FALSE(path.Empty());
        REQUIRE(path.Points().size() == 1);
        REQUIRE(path.Points().front() == req.start);
        REQUIRE(path.Points().back()  == req.goal);
        REQUIRE(path.Length() == 1);
    }

    SECTION("start out of bounds") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {-1, 0}, {4, 4});

        cse498::SampleWorldPath path = pg.GeneratePath(req);

        REQUIRE(path.Empty());
        REQUIRE(path.Length() == 0);
    }

    SECTION("goal out of bounds") {
        cse498::WorldView world(5, 5);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {5, 5});

        cse498::SampleWorldPath path = pg.GeneratePath(req);

        REQUIRE(path.Empty());
        REQUIRE(path.Length() == 0);
    }

    SECTION("world view not set") {
        cse498::PathGenerator pg;

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {1, 1});

        cse498::SampleWorldPath path = pg.GeneratePath(req);

        REQUIRE(path.Empty());
        REQUIRE(path.Length() == 0);
    }

    SECTION("empty world") {
        cse498::WorldView world(0, 0);
        cse498::PathGenerator pg;
        pg.SetWorldView(world);

        const cse498::PathRequest req =
            MakeReq(cse498::PathType::Shortest, {0, 0}, {0, 0});

        cse498::SampleWorldPath path = pg.GeneratePath(req);

        REQUIRE(path.Empty());
        REQUIRE(path.Length() == 0);
    }
}