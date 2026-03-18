/**
 * @file SQLiteConnection.cpp
 * @author Group-9
 * @brief Implementation of the SQLiteConnection wrapper.
 *
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












} // namespace cse498
