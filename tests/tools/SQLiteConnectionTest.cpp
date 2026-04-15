/**
 * @file SQLiteConnection.cpp
 * @brief Test suite for the SQLiteConnection RAII wrapper
 * @author Group 9
 *
 * Claude AI was used to help writing test cases.
 */

#include "catch2/catch.hpp"
#include "../../source/tools/SQLiteConnection.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using namespace cse498;


static SQLiteConnection MakeTestDB() {
    SQLiteConnection db(":memory:");
    REQUIRE(db.IsOpen());
    auto rc = db.Execute(
        "CREATE TABLE kv_store ("
        "  key        TEXT PRIMARY KEY NOT NULL,"
        "  value      BLOB NOT NULL,"
        "  type_tag   TEXT NOT NULL DEFAULT '',"
        "  created_at TEXT NOT NULL DEFAULT (datetime('now')),"
        "  updated_at TEXT NOT NULL DEFAULT (datetime('now'))"
        ");"
    );
    REQUIRE(rc.has_value());
    return db;
}


static std::vector<uint8_t> ToBlob(const std::string& s) {
    return {s.begin(), s.end()};
}


static std::string FromBlob(const std::vector<uint8_t>& b) {
    return {b.begin(), b.end()};
}


// ============================================================================
// Open / Close / IsOpen
// ============================================================================

TEST_CASE("SQLiteConnection - Open in-memory", "[sqlite]") {
    SQLiteConnection db(":memory:");
    REQUIRE(db.IsOpen());
}

TEST_CASE("SQLiteConnection - IsOpen false after move", "[sqlite]") {
    SQLiteConnection db(":memory:");
    REQUIRE(db.IsOpen());

    SQLiteConnection db2(std::move(db));
    REQUIRE(db2.IsOpen());
    REQUIRE_FALSE(db.IsOpen());  // NOLINT — testing moved-from state
}

TEST_CASE("SQLiteConnection - Move assignment", "[sqlite]") {
    SQLiteConnection db1(":memory:");
    SQLiteConnection db2(":memory:");

    db2 = std::move(db1);
    REQUIRE(db2.IsOpen());
    REQUIRE_FALSE(db1.IsOpen());  // NOLINT
}

TEST_CASE("SQLiteConnection - LastErrorMessage when closed", "[sqlite]") {
    SQLiteConnection db(":memory:");
    SQLiteConnection db2(std::move(db));
    // db is now moved-from
    REQUIRE(db.LastErrorMessage() == "Connection is not open");  // NOLINT
}

// ============================================================================
// Execute
// ============================================================================

TEST_CASE("SQLiteConnection - Execute CREATE TABLE", "[sqlite]") {
    SQLiteConnection db(":memory:");
    auto result = db.Execute("CREATE TABLE t (id INTEGER PRIMARY KEY);");
    REQUIRE(result.has_value());
}

TEST_CASE("SQLiteConnection - Execute invalid SQL", "[sqlite]") {
    SQLiteConnection db(":memory:");
    auto result = db.Execute("NOT VALID SQL AT ALL;");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SQLiteError::StepFailed);
}

// ============================================================================
// Query
// ============================================================================

TEST_CASE("SQLiteConnection - Query with rows", "[sqlite]") {
    SQLiteConnection db(":memory:");
    REQUIRE(db.Execute("CREATE TABLE nums (val INTEGER);").has_value());
    REQUIRE(db.Execute("INSERT INTO nums VALUES (10);").has_value());
    REQUIRE(db.Execute("INSERT INTO nums VALUES (20);").has_value());
    REQUIRE(db.Execute("INSERT INTO nums VALUES (30);").has_value());

    std::vector<int> values;
    auto result = db.Query("SELECT val FROM nums ORDER BY val;",
        [&](sqlite3_stmt* stmt) {
            values.push_back(sqlite3_column_int(stmt, 0));
        });

    REQUIRE(result.has_value());
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 10);
    REQUIRE(values[1] == 20);
    REQUIRE(values[2] == 30);
}

TEST_CASE("SQLiteConnection - Query with no rows", "[sqlite]") {
    SQLiteConnection db(":memory:");
    REQUIRE(db.Execute("CREATE TABLE t (id INTEGER);").has_value());

    int count = 0;
    auto result = db.Query("SELECT id FROM t;",
        [&](sqlite3_stmt*) { count++; });

    REQUIRE(result.has_value());
    REQUIRE(count == 0);
}

// ============================================================================
// UpsertBlob + GetBlob
// ============================================================================

TEST_CASE("SQLiteConnection - UpsertBlob and GetBlob roundtrip", "[sqlite]") {
    auto db = MakeTestDB();

    std::vector<uint8_t> blob = {0xDE, 0xAD, 0xBE, 0xEF};
    auto upsert = db.UpsertBlob("kv_store", "key1", blob, "test");
    REQUIRE(upsert.has_value());

    auto loaded = db.GetBlob("kv_store", "key1");
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == blob);
}

TEST_CASE("SQLiteConnection - UpsertBlob overwrites existing key", "[sqlite]") {
    auto db = MakeTestDB();

    auto blob1 = ToBlob("first");
    auto blob2 = ToBlob("second");

    REQUIRE(db.UpsertBlob("kv_store", "k", blob1, "string").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "k", blob2, "string").has_value());

    auto loaded = db.GetBlob("kv_store", "k");
    REQUIRE(loaded.has_value());
    REQUIRE(FromBlob(*loaded) == "second");

    // Should still be one row, not two
    auto count = db.GetRowCount("kv_store");
    REQUIRE(count.has_value());
    REQUIRE(*count == 1);
}

TEST_CASE("SQLiteConnection - UpsertBlob updates type_tag on overwrite", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "k", ToBlob("v"), "int").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "k", ToBlob("v2"), "string").has_value());

    auto tag = db.GetTypeTag("kv_store", "k");
    REQUIRE(tag.has_value());
    REQUIRE(*tag == "string");
}

TEST_CASE("SQLiteConnection - UpsertBlob empty blob", "[sqlite]") {
    auto db = MakeTestDB();

    std::vector<uint8_t> empty;
    REQUIRE(db.UpsertBlob("kv_store", "empty", empty, "none").has_value());

    auto loaded = db.GetBlob("kv_store", "empty");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->empty());
}

TEST_CASE("SQLiteConnection - UpsertBlob large blob", "[sqlite]") {
    auto db = MakeTestDB();

    // 100 KB blob
    std::vector<uint8_t> big(100000, 0x42);
    REQUIRE(db.UpsertBlob("kv_store", "big", big, "blob").has_value());

    auto loaded = db.GetBlob("kv_store", "big");
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == big);
}

TEST_CASE("SQLiteConnection - GetBlob not found", "[sqlite]") {
    auto db = MakeTestDB();

    auto result = db.GetBlob("kv_store", "nonexistent");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SQLiteError::NotFound);
}

// ============================================================================
// DeleteRow
// ============================================================================

TEST_CASE("SQLiteConnection - DeleteRow removes entry", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "k", ToBlob("v"), "t").has_value());
    REQUIRE(db.RowExists("kv_store", "k").has_value());
    REQUIRE(*db.RowExists("kv_store", "k") == true);

    auto del = db.DeleteRow("kv_store", "k");
    REQUIRE(del.has_value());

    REQUIRE(*db.RowExists("kv_store", "k") == false);
}

TEST_CASE("SQLiteConnection - DeleteRow on nonexistent key succeeds", "[sqlite]") {
    auto db = MakeTestDB();

    // Should not error — just deletes zero rows
    auto del = db.DeleteRow("kv_store", "ghost");
    REQUIRE(del.has_value());
}

// ============================================================================
// RowExists
// ============================================================================

TEST_CASE("SQLiteConnection - RowExists false when empty", "[sqlite]") {
    auto db = MakeTestDB();
    auto result = db.RowExists("kv_store", "anything");
    REQUIRE(result.has_value());
    REQUIRE(*result == false);
}

TEST_CASE("SQLiteConnection - RowExists true after insert", "[sqlite]") {
    auto db = MakeTestDB();
    REQUIRE(db.UpsertBlob("kv_store", "k", ToBlob("v"), "t").has_value());

    auto result = db.RowExists("kv_store", "k");
    REQUIRE(result.has_value());
    REQUIRE(*result == true);
}

// ============================================================================
// GetAllKeys
// ============================================================================

TEST_CASE("SQLiteConnection - GetAllKeys empty table", "[sqlite]") {
    auto db = MakeTestDB();

    auto keys = db.GetAllKeys("kv_store");
    REQUIRE(keys.has_value());
    REQUIRE(keys->empty());
}

TEST_CASE("SQLiteConnection - GetAllKeys returns all keys", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "alpha", ToBlob("a"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "beta", ToBlob("b"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "gamma", ToBlob("c"), "t").has_value());

    auto keys = db.GetAllKeys("kv_store");
    REQUIRE(keys.has_value());
    REQUIRE(keys->size() == 3);

    // Order is not guaranteed, so sort before checking
    std::sort(keys->begin(), keys->end());
    REQUIRE((*keys)[0] == "alpha");
    REQUIRE((*keys)[1] == "beta");
    REQUIRE((*keys)[2] == "gamma");
}

// ============================================================================
// GetRowCount
// ============================================================================

TEST_CASE("SQLiteConnection - GetRowCount starts at zero", "[sqlite]") {
    auto db = MakeTestDB();

    auto count = db.GetRowCount("kv_store");
    REQUIRE(count.has_value());
    REQUIRE(*count == 0);
}

TEST_CASE("SQLiteConnection - GetRowCount tracks inserts and deletes", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "a", ToBlob("1"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "b", ToBlob("2"), "t").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 2);

    REQUIRE(db.DeleteRow("kv_store", "a").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 1);
}

// ============================================================================
// GetTypeTag
// ============================================================================

TEST_CASE("SQLiteConnection - GetTypeTag roundtrip", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "score", ToBlob("i:42;"), "int").has_value());

    auto tag = db.GetTypeTag("kv_store", "score");
    REQUIRE(tag.has_value());
    REQUIRE(*tag == "int");
}

TEST_CASE("SQLiteConnection - GetTypeTag not found", "[sqlite]") {
    auto db = MakeTestDB();

    auto tag = db.GetTypeTag("kv_store", "missing");
    REQUIRE_FALSE(tag.has_value());
    REQUIRE(tag.error() == SQLiteError::NotFound);
}

// ============================================================================
// ClearTable
// ============================================================================

TEST_CASE("SQLiteConnection - ClearTable removes all rows", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "a", ToBlob("1"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "b", ToBlob("2"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "c", ToBlob("3"), "t").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 3);

    REQUIRE(db.ClearTable("kv_store").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 0);
}

TEST_CASE("SQLiteConnection - ClearTable on empty table succeeds", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.ClearTable("kv_store").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 0);
}

// ============================================================================
// GetTotalValueBytes
// ============================================================================

TEST_CASE("SQLiteConnection - GetTotalValueBytes empty table", "[sqlite]") {
    auto db = MakeTestDB();

    auto total = db.GetTotalValueBytes("kv_store");
    REQUIRE(total.has_value());
    REQUIRE(*total == 0);
}

TEST_CASE("SQLiteConnection - GetTotalValueBytes sums correctly", "[sqlite]") {
    auto db = MakeTestDB();

    // 3 bytes + 5 bytes = 8 bytes
    REQUIRE(db.UpsertBlob("kv_store", "a", ToBlob("abc"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "b", ToBlob("12345"), "t").has_value());

    auto total = db.GetTotalValueBytes("kv_store");
    REQUIRE(total.has_value());
    REQUIRE(*total == 8);
}

// ============================================================================
// Transactions
// ============================================================================

TEST_CASE("SQLiteConnection - Transaction commit persists", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.BeginTransaction().has_value());
    REQUIRE(db.UpsertBlob("kv_store", "a", ToBlob("1"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "b", ToBlob("2"), "t").has_value());
    REQUIRE(db.Commit().has_value());

    REQUIRE(*db.GetRowCount("kv_store") == 2);
    REQUIRE(FromBlob(*db.GetBlob("kv_store", "a")) == "1");
    REQUIRE(FromBlob(*db.GetBlob("kv_store", "b")) == "2");
}

TEST_CASE("SQLiteConnection - Transaction rollback reverts", "[sqlite]") {
    auto db = MakeTestDB();

    // Insert one row outside the transaction
    REQUIRE(db.UpsertBlob("kv_store", "before", ToBlob("safe"), "t").has_value());

    REQUIRE(db.BeginTransaction().has_value());
    REQUIRE(db.UpsertBlob("kv_store", "a", ToBlob("1"), "t").has_value());
    REQUIRE(db.UpsertBlob("kv_store", "b", ToBlob("2"), "t").has_value());
    REQUIRE(db.Rollback().has_value());

    // The two rows inside the transaction should be gone
    REQUIRE(*db.GetRowCount("kv_store") == 1);
    REQUIRE(*db.RowExists("kv_store", "before") == true);
    REQUIRE(*db.RowExists("kv_store", "a") == false);
    REQUIRE(*db.RowExists("kv_store", "b") == false);
}

TEST_CASE("SQLiteConnection - Transaction rollback reverts deletes", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.UpsertBlob("kv_store", "keep", ToBlob("v"), "t").has_value());
    REQUIRE(*db.GetRowCount("kv_store") == 1);

    REQUIRE(db.BeginTransaction().has_value());
    REQUIRE(db.DeleteRow("kv_store", "keep").has_value());
    REQUIRE(*db.RowExists("kv_store", "keep") == false);  // gone inside txn
    REQUIRE(db.Rollback().has_value());

    // Should be restored after rollback
    REQUIRE(*db.RowExists("kv_store", "keep") == true);
}

// ============================================================================
// EnableWAL
// ============================================================================

TEST_CASE("SQLiteConnection - EnableWAL succeeds", "[sqlite]") {
    SQLiteConnection db(":memory:");
    auto result = db.EnableWAL();
    REQUIRE(result.has_value());
}

// ============================================================================
// File persistence (write to disk, reopen, verify)
// ============================================================================

TEST_CASE("SQLiteConnection - Persist to file and reopen", "[sqlite]") {
    const std::string path = "/tmp/cse498_sqlite_test.db";

    // Clean up any leftover file from a previous run
    std::remove(path.c_str());

    // Scope 1: create, insert, close
    {
        SQLiteConnection db(path);
        REQUIRE(db.IsOpen());
        REQUIRE(db.Execute(
            "CREATE TABLE kv_store ("
            "  key TEXT PRIMARY KEY NOT NULL,"
            "  value BLOB NOT NULL,"
            "  type_tag TEXT NOT NULL DEFAULT '',"
            "  created_at TEXT NOT NULL DEFAULT (datetime('now')),"
            "  updated_at TEXT NOT NULL DEFAULT (datetime('now'))"
            ");"
        ).has_value());

        REQUIRE(db.UpsertBlob("kv_store", "name", ToBlob("Alice"), "string").has_value());
        REQUIRE(db.UpsertBlob("kv_store", "score", ToBlob("i:42;"), "int").has_value());
    }
    // db destructor closes the connection here

    // Scope 2: reopen the same file and verify data survived
    {
        SQLiteConnection db(path);
        REQUIRE(db.IsOpen());

        REQUIRE(*db.GetRowCount("kv_store") == 2);

        auto name = db.GetBlob("kv_store", "name");
        REQUIRE(name.has_value());
        REQUIRE(FromBlob(*name) == "Alice");

        auto score = db.GetBlob("kv_store", "score");
        REQUIRE(score.has_value());
        REQUIRE(FromBlob(*score) == "i:42;");

        REQUIRE(*db.GetTypeTag("kv_store", "name") == "string");
        REQUIRE(*db.GetTypeTag("kv_store", "score") == "int");
    }

    // Clean up
    std::remove(path.c_str());
}

// ============================================================================
// Multiple keys stress test
// ============================================================================

TEST_CASE("SQLiteConnection - 500 keys insert and read", "[sqlite]") {
    auto db = MakeTestDB();

    REQUIRE(db.BeginTransaction().has_value());
    for (int i = 0; i < 500; ++i) {
        std::string key = "key:" + std::to_string(i);
        std::string val = "value_" + std::to_string(i);
        REQUIRE(db.UpsertBlob("kv_store", key, ToBlob(val), "string").has_value());
    }
    REQUIRE(db.Commit().has_value());

    REQUIRE(*db.GetRowCount("kv_store") == 500);

    // Spot check a few
    REQUIRE(FromBlob(*db.GetBlob("kv_store", "key:0")) == "value_0");
    REQUIRE(FromBlob(*db.GetBlob("kv_store", "key:499")) == "value_499");
    REQUIRE(FromBlob(*db.GetBlob("kv_store", "key:250")) == "value_250");

    auto keys = db.GetAllKeys("kv_store");
    REQUIRE(keys.has_value());
    REQUIRE(keys->size() == 500);
}

// ============================================================================
// Edge cases: special characters in keys and values
// ============================================================================

TEST_CASE("SQLiteConnection - Special characters in key", "[sqlite]") {
    auto db = MakeTestDB();

    std::string key = "player:alice:world:main:chunk:10:-5";
    REQUIRE(db.UpsertBlob("kv_store", key, ToBlob("data"), "t").has_value());

    REQUIRE(*db.RowExists("kv_store", key) == true);
    REQUIRE(FromBlob(*db.GetBlob("kv_store", key)) == "data");
}

TEST_CASE("SQLiteConnection - Binary blob with null bytes", "[sqlite]") {
    auto db = MakeTestDB();

    std::vector<uint8_t> blob = {0x00, 0xFF, 0x00, 0x42, 0x00};
    REQUIRE(db.UpsertBlob("kv_store", "binary", blob, "blob").has_value());

    auto loaded = db.GetBlob("kv_store", "binary");
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == blob);
    REQUIRE(loaded->size() == 5);
}

TEST_CASE("SQLiteConnection - Unicode key and value", "[sqlite]") {
    auto db = MakeTestDB();

    std::string key = "player:世界";
    auto blob = ToBlob("Hello 🌍");
    REQUIRE(db.UpsertBlob("kv_store", key, blob, "string").has_value());

    REQUIRE(*db.RowExists("kv_store", key) == true);
    REQUIRE(*db.GetBlob("kv_store", key) == blob);
}
