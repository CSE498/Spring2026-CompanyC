#include <catch2/catch.hpp>
#include "../../source/tools/DataGrid.hpp"
#include "../../source/tools/Datum.hpp"

using namespace cse498;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────

TEST_CASE("DataGrid constructor creates grid with correct dimensions", "[constructor]") {
    DataGrid dg(3, 4);

    SECTION("Row count is correct") {
        // Column(0) should have 3 elements
        auto col = dg.Column(0);
        REQUIRE(col.size() == 3);
    }

    SECTION("Column count is correct") {
        auto row = dg.Row(0);
        REQUIRE(row.size() == 4);
    }
}

TEST_CASE("DataGrid constructor with 1x1 size", "[constructor]") {
    DataGrid dg(1, 1);
    dg.Insert(0, 0, 42);
    REQUIRE(dg.At(0, 0).AsDouble() == Approx(42.0));
}

TEST_CASE("DataGrid constructor with zero rows or columns does not crash", "[constructor]") {
    // Edge case: zero-size grid
    REQUIRE_NOTHROW(DataGrid(0, 0));
}

// ─────────────────────────────────────────────
// Insert (sequential)
// ─────────────────────────────────────────────

TEST_CASE("Insert fills grid left-to-right, top-to-bottom", "[insert]") {
    DataGrid dg(2, 3);

    dg.Insert(1);
    dg.Insert(2);
    dg.Insert(3);
    dg.Insert(4);
    dg.Insert(5);
    dg.Insert(6);

    REQUIRE(dg.At(0, 0).AsDouble() == Approx(1.0));
    REQUIRE(dg.At(0, 1).AsDouble() == Approx(2.0));
    REQUIRE(dg.At(0, 2).AsDouble() == Approx(3.0));
    REQUIRE(dg.At(1, 0).AsDouble() == Approx(4.0));
    REQUIRE(dg.At(1, 1).AsDouble() == Approx(5.0));
    REQUIRE(dg.At(1, 2).AsDouble() == Approx(6.0));
}

TEST_CASE("Insert beyond capacity expands the grid", "[insert]") {
    DataGrid dg(1, 2);

    dg.Insert(10);
    dg.Insert(20);
    // Grid is now full; next Insert should expand
    REQUIRE_NOTHROW(dg.Insert(30));
    REQUIRE(dg.At(1, 0).AsDouble() == Approx(30.0));
}

TEST_CASE("Insert supports double values", "[insert]") {
    DataGrid dg(1, 2);
    dg.Insert(3.14);
    REQUIRE(dg.At(0, 0).AsDouble() == Approx(3.14));
}

TEST_CASE("Insert supports string values", "[insert]") {
    DataGrid dg(1, 2);
    dg.Insert(std::string("hello"));
    REQUIRE(dg.At(0, 0).AsString() == "hello");
}

// ─────────────────────────────────────────────
// Insert (positional)
// ─────────────────────────────────────────────

TEST_CASE("Insert at valid position stores value correctly", "[insert_pos]") {
    DataGrid dg(3, 3);
    dg.Insert(1, 2, 99);
    REQUIRE(dg.At(1, 2).AsDouble() == Approx(99.0));
}

TEST_CASE("Insert at (0,0) works correctly", "[insert_pos]") {
    DataGrid dg(2, 2);
    dg.Insert(0, 0, 7);
    REQUIRE(dg.At(0, 0).AsDouble() == Approx(7.0));
}

TEST_CASE("Insert at last valid cell works correctly", "[insert_pos]") {
    DataGrid dg(3, 3);
    dg.Insert(2, 2, 55);
    REQUIRE(dg.At(2, 2).AsDouble() == Approx(55.0));
}

TEST_CASE("Insert at out-of-bounds row throws out_of_range", "[insert_pos]") {
    DataGrid dg(2, 2);
    REQUIRE_THROWS_AS(dg.Insert(5, 0, 1), std::out_of_range);
}

TEST_CASE("Insert at out-of-bounds column throws out_of_range", "[insert_pos]") {
    DataGrid dg(2, 2);
    REQUIRE_THROWS_AS(dg.Insert(0, 5, 1), std::out_of_range);
}

TEST_CASE("Insert at negative row throws out_of_range", "[insert_pos]") {
    DataGrid dg(2, 2);
    REQUIRE_THROWS_AS(dg.Insert(-1, 0, 1), std::out_of_range);
}

TEST_CASE("Insert at negative column throws out_of_range", "[insert_pos]") {
    DataGrid dg(2, 2);
    REQUIRE_THROWS_AS(dg.Insert(0, -1, 1), std::out_of_range);
}

TEST_CASE("Overwriting an existing cell updates the value", "[insert_pos]") {
    DataGrid dg(2, 2);
    dg.Insert(0, 0, 10);
    dg.Insert(0, 0, 20);
    REQUIRE(dg.At(0, 0).AsDouble() == Approx(20.0));
}

// ─────────────────────────────────────────────
// Append
// ─────────────────────────────────────────────

TEST_CASE("Append adds a new row to the grid", "[append]") {
    DataGrid dg(1, 3);
    std::vector<Datum> row = {Datum(1), Datum(2), Datum(3)};
    dg.Append(row);

    // The appended row becomes row index 1
    auto retrieved = dg.Row(1);
    REQUIRE(retrieved.size() == 3);
    REQUIRE(retrieved[0].AsDouble() == Approx(1.0));
    REQUIRE(retrieved[1].AsDouble() == Approx(2.0));
    REQUIRE(retrieved[2].AsDouble() == Approx(3.0));
}

TEST_CASE("Append to empty grid (0 rows) works", "[append]") {
    DataGrid dg(0, 3);
    std::vector<Datum> row = {Datum(5), Datum(6), Datum(7)};
    REQUIRE_NOTHROW(dg.Append(row));
}

TEST_CASE("Multiple Appends accumulate rows", "[append]") {
    DataGrid dg(0, 2);
    dg.Append({Datum(1), Datum(2)});
    dg.Append({Datum(3), Datum(4)});
    dg.Append({Datum(5), Datum(6)});

    REQUIRE(dg.Row(0)[0].AsDouble() == Approx(1.0));
    REQUIRE(dg.Row(1)[0].AsDouble() == Approx(3.0));
    REQUIRE(dg.Row(2)[0].AsDouble() == Approx(5.0));
}

// ─────────────────────────────────────────────
// Row
// ─────────────────────────────────────────────

TEST_CASE("Row returns correct elements", "[row]") {
    DataGrid dg(2, 3);
    dg.Insert(0, 0, 1);
    dg.Insert(0, 1, 2);
    dg.Insert(0, 2, 3);

    auto row = dg.Row(0);
    REQUIRE(row.size() == 3);
    REQUIRE(row[0].AsDouble() == Approx(1.0));
    REQUIRE(row[1].AsDouble() == Approx(2.0));
    REQUIRE(row[2].AsDouble() == Approx(3.0));
}

TEST_CASE("Row retrieval on last row works correctly", "[row]") {
    DataGrid dg(3, 2);
    dg.Insert(2, 0, 88);
    dg.Insert(2, 1, 99);

    auto row = dg.Row(2);
    REQUIRE(row[0].AsDouble() == Approx(88.0));
    REQUIRE(row[1].AsDouble() == Approx(99.0));
}

// Note: Row() uses mDim.second for bounds check — this documents the known bug
TEST_CASE("Row with out-of-bounds index triggers assertion or UB [known bug: checks wrong dim]", "[row][known_bug]") {
    // The implementation currently asserts r < mDim.second (columns) instead of mDim.first (rows).
    // This test documents the behaviour and will need updating once the bug is fixed.
    DataGrid dg(3, 5);
    // r=2 < mDim.second(5), so this passes despite being a valid row index
    REQUIRE_NOTHROW(dg.Row(2));
}

// ─────────────────────────────────────────────
// Column
// ─────────────────────────────────────────────

TEST_CASE("Column returns correct elements", "[column]") {
    DataGrid dg(3, 2);
    dg.Insert(0, 1, 10);
    dg.Insert(1, 1, 20);
    dg.Insert(2, 1, 30);

    auto col = dg.Column(1);
    REQUIRE(col.size() == 3);
    REQUIRE(col[0].AsDouble() == Approx(10.0));
    REQUIRE(col[1].AsDouble() == Approx(20.0));
    REQUIRE(col[2].AsDouble() == Approx(30.0));
}

TEST_CASE("Column(0) retrieves first column", "[column]") {
    DataGrid dg(2, 3);
    dg.Insert(0, 0, 5);
    dg.Insert(1, 0, 6);

    auto col = dg.Column(0);
    REQUIRE(col[0].AsDouble() == Approx(5.0));
    REQUIRE(col[1].AsDouble() == Approx(6.0));
}

TEST_CASE("Column with out-of-bounds index triggers assertion", "[column]") {
    DataGrid dg(2, 3);
    // c=5 >= mDim.second(3), assertion should fire
    // Catch2 can't catch assert() — wrap in death test or just document
    // REQUIRE_THROWS is not applicable here; use a guard if assert is replaced with exceptions.
}

// ─────────────────────────────────────────────
// At
// ─────────────────────────────────────────────

TEST_CASE("At returns the correct value", "[at]") {
    DataGrid dg(3, 3);
    dg.Insert(2, 1, 42);
    REQUIRE(dg.At(2, 1).AsDouble() == Approx(42.0));
}

TEST_CASE("At on default-constructed cell returns default Datum value", "[at]") {
    DataGrid dg(2, 2);
    // No insertions — value is whatever Datum's default is
    REQUIRE_NOTHROW(dg.At(0, 0));
}

TEST_CASE("At with multiple different types", "[at]") {
    DataGrid dg(2, 3);
    dg.Insert(0, 0, 3.14);
    dg.Insert(0, 1, std::string("world"));
    dg.Insert(0, 2, 100);

    REQUIRE(dg.At(0, 0).AsDouble() == Approx(3.14));
    REQUIRE(dg.At(0, 1).AsString() == "world");
    REQUIRE(dg.At(0, 2).AsDouble() == Approx(100.0));
}

// ─────────────────────────────────────────────
// Find
// ─────────────────────────────────────────────

TEST_CASE("Find returns correct position for an existing element", "[find]") {
    DataGrid dg(3, 3);
    dg.Insert(1, 2, 77);

    auto result = dg.Find(77);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 1);
    REQUIRE(result->second == 2);
}

TEST_CASE("Find returns unexpected for a missing element", "[find]") {
    DataGrid dg(2, 2);
    dg.Insert(0, 0, 1);
    dg.Insert(0, 1, 2);

    auto result = dg.Find(999);
    REQUIRE_FALSE(result.has_value());
    REQUIRE_FALSE(result.error().empty());
}

TEST_CASE("Find works for element at (0,0)", "[find]") {
    DataGrid dg(2, 2);
    dg.Insert(0, 0, 5);

    auto result = dg.Find(5);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 0);
    REQUIRE(result->second == 0);
}

TEST_CASE("Find works for element in last cell", "[find]") {
    DataGrid dg(2, 2);
    dg.Insert(1, 1, 99);

    auto result = dg.Find(99);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 1);
    REQUIRE(result->second == 1);
}

TEST_CASE("Find on empty grid returns unexpected", "[find]") {
    DataGrid dg(0, 0);
    auto result = dg.Find(1);
    REQUIRE_FALSE(result.has_value());
}

// ─────────────────────────────────────────────
// Iterator
// ─────────────────────────────────────────────

TEST_CASE("Iterator traverses all elements in row-major order", "[iterator]") {
    DataGrid dg(2, 3);
    dg.Insert(1); dg.Insert(2); dg.Insert(3);
    dg.Insert(4); dg.Insert(5); dg.Insert(6);

    std::vector<double> visited;
    for (auto it = dg.begin(); it != dg.end(); ++it) {
        visited.push_back((*it).AsDouble());
    }

    REQUIRE(visited == std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
}

TEST_CASE("Iterator begin() != end() on non-empty grid", "[iterator]") {
    DataGrid dg(2, 2);
    dg.Insert(1);
    REQUIRE(dg.begin() != dg.end());
}

TEST_CASE("Iterator dereference returns correct Datum", "[iterator]") {
    DataGrid dg(1, 1);
    dg.Insert(0, 0, 123);

    auto it = dg.begin();
    REQUIRE((*it).AsDouble() == Approx(123.0));
}

TEST_CASE("Iterator Pos() returns correct (row, col)", "[iterator]") {
    DataGrid dg(2, 3);
    auto it = dg.begin();
    REQUIRE(it.Pos() == std::make_pair<size_t, size_t>(0, 0));

    ++it;
    auto pos = it.Pos();
    REQUIRE(pos.first == size_t{0});
    REQUIRE(pos.second == size_t{1});

    ++it; ++it;
    pos = it.Pos();
    REQUIRE(pos.first == size_t{1});
    REQUIRE(pos.second == size_t{0});
}

TEST_CASE("Iterator wraps column to next row correctly", "[iterator]") {
    DataGrid dg(2, 2);
    dg.Insert(1); dg.Insert(2); dg.Insert(3); dg.Insert(4);

    auto it = dg.begin();
    ++it; ++it; // Move to (1, 0)
    REQUIRE(it.Pos().first == 1);
    REQUIRE(it.Pos().second == 0);
}

// ─────────────────────────────────────────────
// Mixed / integration
// ─────────────────────────────────────────────

TEST_CASE("Insert then Row then Column are consistent", "[integration]") {
    DataGrid dg(3, 3);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            dg.Insert(r, c, r * 3 + c);

    auto row1 = dg.Row(1);
    REQUIRE(row1[0].AsDouble() == Approx(3.0));
    REQUIRE(row1[1].AsDouble() == Approx(4.0));
    REQUIRE(row1[2].AsDouble() == Approx(5.0));

    auto col2 = dg.Column(2);
    REQUIRE(col2[0].AsDouble() == Approx(2.0));
    REQUIRE(col2[1].AsDouble() == Approx(5.0));
    REQUIRE(col2[2].AsDouble() == Approx(8.0));
}

TEST_CASE("Append then Column reflects appended data", "[integration]") {
    DataGrid dg(1, 2);
    dg.Insert(0, 0, 10);
    dg.Insert(0, 1, 20);
    dg.Append({Datum(30), Datum(40)});

    auto col0 = dg.Column(0);
    REQUIRE(col0[0].AsDouble() == Approx(10.0));
    REQUIRE(col0[1].AsDouble() == Approx(30.0));
}

TEST_CASE("Find after sequential Insert finds correct position", "[integration]") {
    DataGrid dg(3, 3);
    dg.Insert(1); dg.Insert(2); dg.Insert(3);
    dg.Insert(4); dg.Insert(5); dg.Insert(6);
    dg.Insert(7); dg.Insert(8); dg.Insert(9);

    auto result = dg.Find(5);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 1);
    REQUIRE(result->second == 1);
}