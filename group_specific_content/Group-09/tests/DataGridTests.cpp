/**
 * DataGridTests.cpp
 * Comprehensive test suite for DataGrid using Catch2.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../source/DataGrid.hpp"

// ============================================================
//  Helpers
// ============================================================

static std::vector<Datum> MakeDatumRow(std::initializer_list<int> vals) {
    std::vector<Datum> row;
    for (int v : vals) row.emplace_back(v);
    return row;
}


// ============================================================
//  Construction
// ============================================================

TEST_CASE("DataGrid construction", "[construction]") {
    SECTION("begin and end both point to (0,0) on a fresh grid") {
        DataGrid dg(3, 4);
        REQUIRE(dg.begin().Pos() == std::pair<int,int>{0, 0});
        REQUIRE(dg.begin().Pos() == dg.end().Pos());
    }

    SECTION("zero rows does not crash") {
        REQUIRE_NOTHROW(DataGrid(0, 5));
    }

    SECTION("zero columns does not crash") {
        REQUIRE_NOTHROW(DataGrid(5, 0));
    }
}


// ============================================================
//  Append
// ============================================================

TEST_CASE("DataGrid::Append", "[append]") {
    SECTION("appended row has correct values") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({10, 20, 30}));

        auto r = dg.Row(0);
        REQUIRE(r.size() == 3);
        REQUIRE(r[0].AsDouble() == 10.0);
        REQUIRE(r[1].AsDouble() == 20.0);
        REQUIRE(r[2].AsDouble() == 30.0);
    }

    SECTION("multiple appends preserve insertion order") {
        DataGrid dg(0, 2);
        dg.Append(MakeDatumRow({1, 2}));
        dg.Append(MakeDatumRow({3, 4}));
        dg.Append(MakeDatumRow({5, 6}));

        REQUIRE(dg.Row(0)[0].AsDouble() == 1.0);
        REQUIRE(dg.Row(1)[0].AsDouble() == 3.0);
        REQUIRE(dg.Row(2)[0].AsDouble() == 5.0);
    }

    SECTION("append increases accessible row count") {
        DataGrid dg(1, 3);
        dg.Append(MakeDatumRow({7, 8, 9}));
        // Row 1 now exists
        REQUIRE_NOTHROW(dg.Row(1));
    }

    SECTION("appending an empty row does not crash") {
        DataGrid dg(0, 0);
        REQUIRE_NOTHROW(dg.Append({}));
    }
}


// ============================================================
//  Row
// ============================================================

TEST_CASE("DataGrid::Row", "[row]") {
    SECTION("returns a copy of the correct row") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({10, 20, 30}));
        dg.Append(MakeDatumRow({40, 50, 60}));

        auto r1 = dg.Row(0);
        REQUIRE(r1[0].AsDouble() == 10.0);
        REQUIRE(r1[2].AsDouble() == 30.0);

        auto r2 = dg.Row(1);
        REQUIRE(r2[0].AsDouble() == 40.0);
        REQUIRE(r2[2].AsDouble() == 60.0);
    }

    SECTION("out-of-bounds row index throws") {
        DataGrid dg(2, 2);
        REQUIRE_THROWS_AS(dg.Row(5), std::out_of_range);
    }
}


// ============================================================
//  Column
// ============================================================

// [bug] std::vector<Datum&> is ill-formed — references cannot be stored
// in standard containers. The return type must change to
// std::vector<std::reference_wrapper<Datum>> or std::vector<Datum>.
// These tests document intended behavior and will not compile until fixed.

TEST_CASE("DataGrid::Column", "[column][bug]") {
    SECTION("returns elements from the correct column") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({1, 2, 3}));
        dg.Append(MakeDatumRow({4, 5, 6}));

        auto col = dg.Column(1);   // should be [2, 5]
        REQUIRE(col.size() == 2);
        REQUIRE(col[0].AsDouble() == 2.0);
        REQUIRE(col[1].AsDouble() == 5.0);
    }

    SECTION("first and last columns are accessible") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({10, 20, 30}));

        REQUIRE(dg.Column(0)[0].AsDouble() == 10.0);
        REQUIRE(dg.Column(2)[0].AsDouble() == 30.0);
    }
}


// ============================================================
//  Insert — sequential
// ============================================================

// [bug] Insert(T) never advances mEnd in the normal path, so every call
// overwrites (0,0). Additionally, when the grid is full the push_back
// branch falls through and attempts an out-of-bounds write.

TEST_CASE("DataGrid::Insert sequential", "[insert][bug]") {
    SECTION("fills cells in row-major order") {
        DataGrid dg(2, 3);
        for (int i = 0; i < 6; ++i) dg.Insert(i * 10);

        REQUIRE(dg.At<int>(0, 0) == 0);
        REQUIRE(dg.At<int>(0, 1) == 10);
        REQUIRE(dg.At<int>(0, 2) == 20);
        REQUIRE(dg.At<int>(1, 0) == 30);
        REQUIRE(dg.At<int>(1, 1) == 40);
        REQUIRE(dg.At<int>(1, 2) == 50);
    }

    SECTION("grows the grid when capacity is exceeded") {
        DataGrid dg(1, 1);
        dg.Insert(99);
        dg.Insert(42);   // should append a new row

        REQUIRE(dg.Row(0)[0].AsDouble() == 99.0);
        REQUIRE(dg.Row(1)[0].AsDouble() == 42.0);
    }
}


// ============================================================
//  Insert — positional
// ============================================================

// [bug] The bounds check is inverted: `c < mDim.second` is true for
// every valid column index, so valid inserts throw and OOB ones do not.

TEST_CASE("DataGrid::Insert positional", "[insert]") {
    SECTION("valid position does not throw") {
        // [bug] currently throws due to inverted column check
        DataGrid dg(3, 3);
        REQUIRE_NOTHROW(dg.Insert(1, 1, 42));
    }

    SECTION("writes the correct value at the given position") {
        // [bug] same as above
        DataGrid dg(3, 3);
        dg.Insert(2, 2, 77);
        REQUIRE(dg.At<int>(2, 2) == 77);
    }

    SECTION("out-of-bounds row throws std::out_of_range") {
        DataGrid dg(2, 2);
        REQUIRE_THROWS_AS(dg.Insert(5, 0, 1), std::out_of_range);
    }

    SECTION("out-of-bounds column throws std::out_of_range") {
        // [bug] currently does NOT throw due to inverted condition
        DataGrid dg(2, 2);
        REQUIRE_THROWS_AS(dg.Insert(0, 5, 1), std::out_of_range);
    }

    SECTION("inserting a double value round-trips correctly") {
        // [bug] will throw until bounds check is fixed
        DataGrid dg(2, 2);
        dg.Insert(0, 0, 3.14);
        REQUIRE(dg.At<double>(0, 0) == Catch::Approx(3.14));
    }
}


// ============================================================
//  At
// ============================================================

TEST_CASE("DataGrid::At", "[at]") {
    SECTION("returns the correct int value") {
        DataGrid dg(0, 2);
        dg.Append(MakeDatumRow({42, 7}));
        REQUIRE(dg.At<int>(0, 0) == 42);
        REQUIRE(dg.At<int>(0, 1) == 7);
    }

    SECTION("returns the correct double value") {
        DataGrid dg(0, 1);
        dg.Append({Datum(2.71828)});
        REQUIRE(dg.At<double>(0, 0) == Catch::Approx(2.71828));
    }

    SECTION("out-of-bounds indices throw") {
        DataGrid dg(1, 1);
        REQUIRE_THROWS_AS(dg.At<int>(10, 10), std::out_of_range);
    }
}


// ============================================================
//  Find
// ============================================================

// [bug] Find() iterates over rows (each a std::vector<Datum>) rather than
// individual Datum cells, so the search lambda never matches anything.
// The return type also conflicts: the header declares std::pair<int,int>
// but the implementation returns std::expected<std::pair<int,int>, std::string>.

TEST_CASE("DataGrid::Find", "[find][bug]") {
    SECTION("finds an element that is present") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({10, 20, 30}));
        dg.Append(MakeDatumRow({40, 50, 60}));

        auto pos = dg.Find(50);
        REQUIRE(pos == std::pair<int,int>{1, 1});
    }

    SECTION("reports failure for an absent element") {
        DataGrid dg(0, 2);
        dg.Append(MakeDatumRow({1, 2}));

        // When using std::expected the result should be valueless
        auto pos = dg.Find(999);
        REQUIRE_FALSE(pos.has_value());
    }

    SECTION("finds an element in the first cell") {
        DataGrid dg(0, 2);
        dg.Append(MakeDatumRow({7, 8}));
        REQUIRE(dg.Find(7) == std::pair<int,int>{0, 0});
    }

    SECTION("finds an element in the last cell") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({1, 2, 3}));
        dg.Append(MakeDatumRow({4, 5, 6}));
        REQUIRE(dg.Find(6) == std::pair<int,int>{1, 2});
    }
}


// ============================================================
//  Iterator
// ============================================================

// [bug] operator++ checks whether mCol == mDim.second and resets to 0,
// but then unconditionally does ++mCol — so the iterator skips col 0 on
// every new row and also double-increments on every normal step.

TEST_CASE("DataGrid iterator", "[iterator]") {
    SECTION("begin() == end() on an empty (unfilled) grid") {
        DataGrid dg(2, 2);
        REQUIRE(dg.begin().Pos() == dg.end().Pos());
    }

    SECTION("Pos() returns correct location at begin") {
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({0, 0, 0}));
        REQUIRE(dg.begin().Pos() == std::pair<int,int>{0, 0});
    }

    SECTION("iterates over every element in row-major order") {
        // [bug] will fail until operator++ is fixed
        DataGrid dg(0, 3);
        dg.Append(MakeDatumRow({1, 2, 3}));
        dg.Append(MakeDatumRow({4, 5, 6}));

        std::vector<double> visited;
        for (auto it = dg.begin(); it != dg.end(); ++it)
            visited.push_back((*it).AsDouble());

        REQUIRE(visited.size() == 6);
        REQUIRE(visited[0] == 1.0);
        REQUIRE(visited[1] == 2.0);
        REQUIRE(visited[2] == 3.0);
        REQUIRE(visited[3] == 4.0);
        REQUIRE(visited[4] == 5.0);
        REQUIRE(visited[5] == 6.0);
    }

    SECTION("range-based for loop sums all elements correctly") {
        // [bug] will fail until operator++ is fixed
        DataGrid dg(0, 2);
        dg.Append(MakeDatumRow({10, 20}));
        dg.Append(MakeDatumRow({30, 40}));

        double sum = 0.0;
        for (auto datum : dg) sum += datum.AsDouble();
        REQUIRE(sum == 100.0);
    }

    SECTION("single-element grid: one increment reaches end") {
        // [bug] will fail until operator++ is fixed
        DataGrid dg(0, 1);
        dg.Append({Datum(99)});

        auto it = dg.begin();
        REQUIRE(it != dg.end());
        REQUIRE((*it).AsDouble() == 99.0);
        ++it;
        REQUIRE(it == dg.end());
    }
}


// ============================================================
//  Integration
// ============================================================

TEST_CASE("DataGrid integration", "[integration]") {
    SECTION("append then Row round-trips correctly") {
        DataGrid dg(0, 4);
        std::vector<Datum> original = {Datum(1), Datum(2), Datum(3), Datum(4)};
        dg.Append(original);

        auto retrieved = dg.Row(0);
        REQUIRE(retrieved.size() == original.size());
        for (size_t i = 0; i < original.size(); ++i)
            REQUIRE(retrieved[i].AsDouble() == original[i].AsDouble());
    }

    SECTION("1000-row stress test preserves all values") {
        DataGrid dg(0, 10);
        for (int i = 0; i < 1000; ++i) {
            std::vector<Datum> row;
            for (int j = 0; j < 10; ++j) row.emplace_back(i * 10 + j);
            dg.Append(row);
        }

        REQUIRE(dg.Row(0)[0].AsDouble()   ==    0.0);
        REQUIRE(dg.Row(0)[9].AsDouble()   ==    9.0);
        REQUIRE(dg.Row(500)[0].AsDouble() == 5000.0);
        REQUIRE(dg.Row(999)[9].AsDouble() == 9999.0);
    }
}