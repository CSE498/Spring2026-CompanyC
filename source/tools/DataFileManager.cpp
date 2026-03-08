/**
 * @file DataFileManager.cpp
 * @author John Masterman
 *
 * Implementation of Class to Manage Data Files
 */

#include "DataFileManager.hpp"

#include <utility>

/**
 * Default constructor does not open a file.
 */
DataFileManager::DataFileManager() : mFile(), mColumns(), mHeaderWritten(false)
{
}

/**
 * Construct and open a file immediately.
 */
DataFileManager::DataFileManager(const std::string& filename) : DataFileManager()
{
    Open(filename);
}

/**
 * Opens (or creates) the data file and prepares it for writing.
 */
void DataFileManager::Open(const std::string& filename)
{
    Close();

    mFile.open(filename, std::ios::out | std::ios::trunc);
    assert(mFile.is_open() && "Open: failed to open file");

    mHeaderWritten = false;
}

/**
 * Closes the data file.
 */
void DataFileManager::Close()
{
    if (mFile.is_open())
    {
        mFile.flush();
        mFile.close();
    }
}

/**
 * Registers a function that will be called during Update() to produce a column value.
 */
void DataFileManager::AddColumn(const std::string& name, std::function<std::string()> fun)
{
    assert(!name.empty() && "AddColumn: column name must not be empty");
    assert(static_cast<bool>(fun) && "AddColumn: function must be valid");

    assert(!mHeaderWritten && "AddColumn: cannot add columns after writing has started");

    mColumns.push_back(Column{name, std::move(fun)});
}

/**
 * Triggers all stored functions in order and appends a new row to the data file.
 */
void DataFileManager::Update()
{
    assert(mFile.is_open() && "Update: file must be open before calling Update()");
    assert(!mColumns.empty() && "Update: no columns registered");

    WriteHeaderIfNeeded();

    for (size_t i = 0; i < mColumns.size(); i++)
    {
        mFile << mColumns[i].mFun();
        if (i + 1 < mColumns.size())
        {
            mFile << ",";
        }
    }

    mFile << "\n";
}

/**
 * Forces any buffered output to be written to disk.
 */
void DataFileManager::Flush()
{
    assert(mFile.is_open() && "Flush: file must be open before calling Flush()");
    mFile.flush();
}

/**
 * Write the header row once using registered column names.
 */
void DataFileManager::WriteHeaderIfNeeded()
{
    if (mHeaderWritten) return;

    for (size_t i = 0; i < mColumns.size(); i++)
    {
        mFile << mColumns[i].mName;
        if (i + 1 < mColumns.size())
        {
            mFile << ",";
        }
    }

    mFile << "\n";
    mHeaderWritten = true;
}
