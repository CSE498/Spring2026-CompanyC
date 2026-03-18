/**
 * @file SQLiteConnection.cpp
 * @author Group-9
 * @brief Implementation of the SQLiteConnection wrapper.
 *
 * claude assistance
 */

#include "SQLiteConnection.hpp"

#include <cstring>

namespace cse498 {

//Constructors, destructors, etc.

SQLiteConnection::SQLiteConnection(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &mDb);

    if (rc != SQLITE_OK) {
        if (mDb) {
            sqlite3_close(mDb);
            mDb = nullptr;
        }
    }
}


SQLiteConnection::~SQLiteConnection() {
    if (mDb) {
        sqlite3_close(mDb);
        mDb = nullptr;
    }
}

SQLiteConnection::SQLiteConnection(SQLiteConnection&& other) noexcept : mDb(other.mDb) {
    other.mDb = nullptr;
}

SQLiteConnection& SQLiteConnection::operator=(SQLiteConnection&& other) noexcept {
    if (this != &other) {
        if (mDb) {
            sqlite3_close(mDb);
        }
        mDb = other.mDb;
        other.mDb = nullptr;
    }
    return *this;
}


// state info
bool SQLiteConnection::IsOpen() const {
    return mDb != nullptr;
}

std::string SQLiteConnection::LastErrorMessage() const {
    if (!mDb) {
        return "Connection is not open";
    }
    return sqlite3_errmsg(mDb);
}



//executions and querys
std::expected<void, SQLiteError> SQLiteConnection::Execute(const std::string& sql) {
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    char* err_msg = nullptr;
    int rc = sqlite3_exec(mDb, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        if (err_msg) sqlite3_free(err_msg);
        return std::unexpected(SQLiteError::StepFailed);
    }

    return {};
}


std::expected<void, SQLiteError> SQLiteConnection::Query(const std::string& sql, std::function<void(sqlite3_stmt*)> row_callback) {
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }


    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        row_callback(stmt);
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::unexpected(SQLiteError::StepFailed);
    }

    return {};
}






//sqlite key value funcs
std::expected<void, SQLiteError> SQLiteConnection::UpsertBlob(const std::string& table, const std::string& key, const std::vector<uint8_t>& value, const std::string& type_tag) {
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql =
        "INSERT INTO " + table + " (key, value, type_tag, created_at, updated_at) "
        "VALUES (?1, ?2, ?3, datetime('now'), datetime('now')) "
        "ON CONFLICT(key) DO UPDATE SET "
        "  value = ?2, type_tag = ?3, updated_at = datetime('now');";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }



    if (value.empty()) {
        rc = sqlite3_bind_zeroblob(stmt, 2, 0);

    } else {
        rc = sqlite3_bind_blob(stmt, 2, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    }
    
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return std::unexpected(SQLiteError::BindFailed);
    }


    rc = sqlite3_bind_text(stmt, 3, type_tag.c_str(), static_cast<int>(type_tag.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::unexpected(SQLiteError::StepFailed);
    }

    return {};
}



std::expected<std::vector<uint8_t>, SQLiteError> SQLiteConnection::GetBlob(const std::string& table, const std::string& key) const{
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT value FROM " + table + " WHERE key = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const void* blob = sqlite3_column_blob(stmt, 0);
        int blob_size = sqlite3_column_bytes(stmt, 0);

        std::vector<uint8_t> result;
        if (blob && blob_size > 0) {
            const auto* bytes = static_cast<const uint8_t*>(blob);
            result.assign(bytes, bytes + blob_size);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return std::unexpected(SQLiteError::NotFound);
    }

    return std::unexpected(SQLiteError::StepFailed);
}



std::expected<void, SQLiteError> SQLiteConnection::DeleteRow(const std::string& table, const std::string& key){
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "DELETE FROM " + table + " WHERE key = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::unexpected(SQLiteError::StepFailed);
    }

    return {};
}


std::expected<bool, SQLiteError> SQLiteConnection::RowExists(const std::string& table, const std::string& key) const{
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT 1 FROM " + table + " WHERE key = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;

    return std::unexpected(SQLiteError::StepFailed);
}

std::expected<std::vector<std::string>, SQLiteError> SQLiteConnection::GetAllKeys(const std::string& table) const{
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT key FROM " + table + ";";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    std::vector<std::string> keys;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (text) {
            keys.emplace_back(text);
        }
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::unexpected(SQLiteError::StepFailed);
    }

    return keys;
}

std::expected<size_t, SQLiteError> SQLiteConnection::GetRowCount(const std::string& table) const{
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT COUNT(*) FROM " + table + ";";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int64_t count = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return static_cast<size_t>(count);
    }

    sqlite3_finalize(stmt);
    return std::unexpected(SQLiteError::StepFailed);
}

std::expected<std::string, SQLiteError> SQLiteConnection::GetTypeTag(const std::string& table, const std::string& key) const{
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT type_tag FROM " + table + " WHERE key = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) { 
        sqlite3_finalize(stmt); 
        return std::unexpected(SQLiteError::BindFailed); 
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string result = text ? text : "";
        sqlite3_finalize(stmt);
        return result;
    }

    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return std::unexpected(SQLiteError::NotFound);
    }

    return std::unexpected(SQLiteError::StepFailed);
}

std::expected<void, SQLiteError> SQLiteConnection::ClearTable(const std::string& table) {
    return Execute("DELETE FROM " + table + ";");
}

std::expected<size_t, SQLiteError> SQLiteConnection::GetTotalValueBytes(const std::string& table) const {
    if (!mDb) return std::unexpected(SQLiteError::OpenFailed);

    const std::string sql = "SELECT COALESCE(SUM(LENGTH(value)), 0) FROM " + table + ";";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(mDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        return std::unexpected(SQLiteError::PrepareFailed);
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int64_t total = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return static_cast<size_t>(total);
    }

    sqlite3_finalize(stmt);
    return std::unexpected(SQLiteError::StepFailed);
}


// atomic transaction stuff

std::expected<void, SQLiteError> SQLiteConnection::BeginTransaction() {
    return Execute("BEGIN TRANSACTION;");
}

std::expected<void, SQLiteError> SQLiteConnection::Commit() {
    return Execute("COMMIT;");
}

std::expected<void, SQLiteError> SQLiteConnection::Rollback() {
    return Execute("ROLLBACK;");
}


std::expected<void, SQLiteError> SQLiteConnection::EnableWAL() {
    return Execute("PRAGMA journal_mode=WAL;");
}

} // namespace cse498
