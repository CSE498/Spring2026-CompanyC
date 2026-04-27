/**
 * @file SQLiteConnection.hpp
 * @author Group-9
 * @brief C++ Wraper for SQLite
 *
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <expected>
#include <functional>

#include "../../third-party/sqlite/sqlite3.h"

namespace cse498 {


enum class SQLiteError {
    OpenFailed,
    PrepareFailed,
    StepFailed,
    BindFailed,
    NotFound
};


class SQLiteConnection {
public:
    /// opens/create db file, like :memory:
    explicit SQLiteConnection(const std::string& db_path);

    /// Closes the connection
    ~SQLiteConnection();

    SQLiteConnection(const SQLiteConnection&) = delete;
    SQLiteConnection& operator=(const SQLiteConnection&) = delete;

    SQLiteConnection(SQLiteConnection&& other) noexcept;
    SQLiteConnection& operator=(SQLiteConnection&& other) noexcept;


    /// Execute SQL statements that return no rows
    /// (CREATE TABLE, INSERT, UPDATE, DELETE, PRAGMA, ...).
    std::expected<void, SQLiteError> Execute(const std::string& sql);

    /// Execute a SELECT
    std::expected<void, SQLiteError> Query(const std::string& sql, std::function<void(sqlite3_stmt*)> row_callback);


    /// INSERT OR REPLACE a (key, blob, type_tag) row.
    std::expected<void, SQLiteError> UpsertBlob(const std::string& table, const std::string& key, const std::vector<uint8_t>& value, const std::string& type_tag);

    /// SELECT value FROM table WHERE key = ?
    [[nodiscard]] std::expected<std::vector<uint8_t>, SQLiteError> GetBlob(const std::string& table, const std::string& key) const;

    /// DELETE FROM table WHERE key = ?
    std::expected<void, SQLiteError> DeleteRow(const std::string& table, const std::string& key);

    /// Returns true if the key exists.
    [[nodiscard]] std::expected<bool, SQLiteError> RowExists(const std::string& table, const std::string& key) const;

    /// SELECT key FROM table  (returns all keys).
    [[nodiscard]] std::expected<std::vector<std::string>, SQLiteError> GetAllKeys(const std::string& table) const;

    /// SELECT COUNT(*) FROM table.
    [[nodiscard]] std::expected<size_t, SQLiteError> GetRowCount(const std::string& table) const;

    /// SELECT type_tag FROM table WHERE key = ?
    [[nodiscard]] std::expected<std::string, SQLiteError> GetTypeTag(const std::string& table,const std::string& key) const;

    /// DELETE FROM table  (removes all rows).
    std::expected<void, SQLiteError> ClearTable(const std::string& table);

    /// SELECT SUM(LENGTH(value)) FROM table.
    [[nodiscard]] std::expected<size_t, SQLiteError> GetTotalValueBytes(const std::string& table) const;


    std::expected<void, SQLiteError> BeginTransaction();
    std::expected<void, SQLiteError> Commit();
    std::expected<void, SQLiteError> Rollback();


    std::expected<void, SQLiteError> EnableWAL();
    [[nodiscard]] bool IsOpen() const;
    [[nodiscard]] std::string LastErrorMessage() const;

private:
    sqlite3* mDb = nullptr;
};

} // namespace cse498
