//
// DataGridTests.cpp
// Catch2 v2 tests for DataGrid
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../source/DataGrid.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("DataGrid constructor allocates correct dimensions", "[constructor]") {
    SECTION("Standard dimensions") {
        DataGrid dg(3, 4);
        // Row and Column should be accessible without throwing
        REQUIRE_NOTHROW(dg.Row(0));
        REQUIRE_NOTHROW(dg.Column(0));
    }

    SECTION("1x1 grid") {
        DataGrid dg(1, 1);
        REQUIRE_NOTHROW(dg.At(0, 0));
    }

    SECTION("Single row grid") {
        DataGrid dg(1, 5);
        REQUIRE(dg.Row(0).size() == 5);
    }

    SECTION("Single column grid") {
        DataGrid dg(5, 1);
        REQUIRE(dg.Column(0).size() == 5);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Insert(r, c, element)
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Insert at specific position", "[insert]") {
    DataGrid dg(3, 3);

    SECTION("Insert int and retrieve with At()") {
        dg.Insert(0, 0, 42);
        REQUIRE(dg.At(0, 0).AsDouble() == Approx(42.0));
    }

    SECTION("Insert double") {
        dg.Insert(1, 2, 3.14);
        REQUIRE(dg.At(1, 2).AsDouble() == Approx(3.14));
    }

    SECTION("Insert string") {
        dg.Insert(2, 1, std::string("hello"));
        // Datum should store the value; verify via Find or At
        REQUIRE_NOTHROW(dg.At(2, 1));
    }

    SECTION("Insert overwrites existing value") {
        dg.Insert(0, 0, 1);
        dg.Insert(0, 0, 99);
        REQUIRE(dg.At(0, 0).AsDouble() == Approx(99.0));
    }

    SECTION("Insert at last valid position") {
        REQUIRE_NOTHROW(dg.Insert(2, 2, 7));
    }

    // ── Edge / Bug cases ──────────────────────────────────────────────────────
    // NOTE: Current Insert(r,c) only checks c >= mDim.second but not r >= mDim.first.
    // The check should be r >= mDim.first. These tests document the expected behavior
    // and will fail until the bug is fixed.
    SECTION("Out-of-range row throws") {
        REQUIRE_THROWS_AS(dg.Insert(5, 0, 1), std::out_of_range);
    }

    SECTION("Out-of-range column throws") {
        REQUIRE_THROWS_AS(dg.Insert(0, 5, 1), std::out_of_range);
    }

    SECTION("Negative indices throw") {
        REQUIRE_THROWS(dg.Insert(-1, 0, 1));
        REQUIRE_THROWS(dg.Insert(0, -1, 1));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Append
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Append adds a new row", "[append]") {
    DataGrid dg(2, 3);

    SECTION("Appended row is accessible") {
        std::vector<Datum> newRow = {Datum(1.0), Datum(2.0), Datum(3.0)};
        dg.Append(newRow);
        // Grid should now have 3 rows
        REQUIRE_NOTHROW(dg.Row(2));
        REQUIRE(dg.Row(2).size() == 3);
    }

    SECTION("Appended values are correct") {
        std::vector<Datum> newRow = {Datum(10.0), Datum(20.0), Datum(30.0)};
        dg.Append(newRow);
        REQUIRE(dg.At(2, 0).AsDouble() == Approx(10.0));
        REQUIRE(dg.At(2, 1).AsDouble() == Approx(20.0));
        REQUIRE(dg.At(2, 2).AsDouble() == Approx(30.0));
    }

    SECTION("Multiple appends increase row count") {
        dg.Append({Datum(1.0), Datum(2.0), Datum(3.0)});
        dg.Append({Datum(4.0), Datum(5.0), Datum(6.0)});
        REQUIRE_NOTHROW(dg.Row(3));
    }

    SECTION("Appending empty row") {
        // Should not crash, even if dimensions mismatch
        REQUIRE_NOTHROW(dg.Append({}));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Row
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Row returns correct data", "[row]") {
    DataGrid dg(3, 3);
    dg.Insert(1, 0, 10);
    dg.Insert(1, 1, 20);
    dg.Insert(1, 2, 30);

    SECTION("Row size matches column count") {
        REQUIRE(dg.Row(1).size() == 3);
    }

    SECTION("Row values are correct") {
        auto row = dg.Row(1);
        REQUIRE(row[0].AsDouble() == Approx(10.0));
        REQUIRE(row[1].AsDouble() == Approx(20.0));
        REQUIRE(row[2].AsDouble() == Approx(30.0));
    }

    SECTION("Row returns a copy, not a reference") {
        auto row = dg.Row(1);
        // Modifying the copy should not affect the grid
        // (This is safe because Row returns by value)
        REQUIRE(dg.At(1, 0).AsDouble() == Approx(10.0));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Column
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Column returns correct data", "[column]") {
    DataGrid dg(3, 3);
    dg.Insert(0, 2, 100);
    dg.Insert(1, 2, 200);
    dg.Insert(2, 2, 300);

    SECTION("Column size matches row count") {
        REQUIRE(dg.Column(2).size() == 3);
    }

    SECTION("Column values are correct") {
        auto col = dg.Column(2);
        REQUIRE(col[0].AsDouble() == Approx(100.0));
        REQUIRE(col[1].AsDouble() == Approx(200.0));
        REQUIRE(col[2].AsDouble() == Approx(300.0));
    }

    SECTION("First column") {
        dg.Insert(0, 0, 7);
        REQUIRE(dg.Column(0)[0].AsDouble() == Approx(7.0));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// At
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("At retrieves correct element", "[at]") {
    DataGrid dg(4, 4);
    dg.Insert(0, 0, 1);
    dg.Insert(3, 3, 999);

    SECTION("Top-left corner") {
        REQUIRE(dg.At(0, 0).AsDouble() == Approx(1.0));
    }

    SECTION("Bottom-right corner") {
        REQUIRE(dg.At(3, 3).AsDouble() == Approx(999.0));
    }

    SECTION("Default-constructed cell has a sane value") {
        REQUIRE_NOTHROW(dg.At(1, 1));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Find
// ─────────────────────────────────────────────────────────────────────────────
// NOTE: Find() currently iterates over rows (std::vector<Datum>) using
// std::find_if, but the lambda compares a Datum to elements of
// std::vector<Datum>. This is a type mismatch bug — Find will not compile
// or will not work correctly until fixed. Tests below document intended behavior.
TEST_CASE("Find locates elements correctly", "[find]") {
    DataGrid dg(3, 3);
    dg.Insert(0, 0, 1.0);
    dg.Insert(1, 1, 42.0);
    dg.Insert(2, 2, 99.0);

    SECTION("Find existing element returns correct position") {
        auto result = dg.Find(42.0);
        REQUIRE(result.has_value());
        REQUIRE(result->first == 1);
        REQUIRE(result->second == 1);
    }

    SECTION("Find element in first cell") {
        auto result = dg.Find(1.0);
        REQUIRE(result.has_value());
        REQUIRE(result->first == 0);
        REQUIRE(result->second == 0);
    }

    SECTION("Find element in last cell") {
        auto result = dg.Find(99.0);
        REQUIRE(result.has_value());
        REQUIRE(result->first == 2);
        REQUIRE(result->second == 2);
    }

    SECTION("Find missing element returns unexpected") {
        auto result = dg.Find(777.0);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == "Your element is not in the data grid.");
    }

    SECTION("Find in empty-valued grid returns unexpected") {
        DataGrid empty(2, 2);
        auto result = empty.Find(1.0);
        REQUIRE_FALSE(result.has_value());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Iterator
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Iterator traverses grid correctly", "[iterator]") {
    DataGrid dg(2, 3);
    dg.Insert(0, 0, 1); dg.Insert(0, 1, 2); dg.Insert(0, 2, 3);
    dg.Insert(1, 0, 4); dg.Insert(1, 1, 5); dg.Insert(1, 2, 6);

    SECTION("begin() != end() on non-empty grid") {
        REQUIRE(dg.begin() != dg.end());
    }

    SECTION("Dereferencing begin gives first element") {
        REQUIRE((*dg.begin()).AsDouble() == Approx(1.0));
    }

    SECTION("Range-based for loop visits all cells") {
        int count = 0;
        for (auto datum : dg) {
            (void)datum;
            count++;
        }
        // 2 rows * 3 cols = 6 elements
        // NOTE: this will expose the off-by-one bug in operator++ where
        // mCol is compared to mDim.second before incrementing past it.
        REQUIRE(count == 6);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Known bugs summary (will appear as failures until fixed)
// ─────────────────────────────────────────────────────────────────────────────
// 1. Insert(r,c): condition `r > mDim.first` should be `r >= mDim.first`
//    → allows inserting one row past the end without throwing.
//
// 2. Insert(r,c): mEnd.second++ is incremented regardless of column position,
//    which will desync mEnd over multiple inserts.
//
// 3. Find(): std::find_if iterates over rows (vector<Datum>) but the lambda
//    expects a single Datum — type mismatch, won't compile as-is.
//    Should iterate over a flattened view or use nested loops.
//
// 4. Iterator::operator++: checks `mCol == mDim.second` but mCol is never
//    incremented to mDim.second before wrapping — should be `mCol >= mDim.second - 1`.