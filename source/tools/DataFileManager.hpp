/**
* @file DataFileManager.h
 * @author John Masterman
 *
 * Class to Manage Data Files
 */

#ifndef DATAFILEMANAGER_H
#define DATAFILEMANAGER_H

#pragma once

#include <cassert>
#include <functional>
#include <fstream>
#include <string>
#include <vector>

class DataFileManager
{
public:
    DataFileManager();
    explicit DataFileManager(const std::string& filename);

    void Open(const std::string& filename);
    void Close();

    void AddColumn(const std::string& name, std::function<std::string()> fun);

    void Update();
    void Flush();

private:
    struct Column
    {
        std::string mName;
        std::function<std::string()> mFun;
    };

    void WriteHeaderIfNeeded();

    std::ofstream mFile;
    std::vector<Column> mColumns;

    bool mHeaderWritten = false;
};

#endif // DATAFILEMANAGER_H
