/**
 * @file DataFileManagerTest.cpp
 * @author John Masterman
 *
* This file contains unit tests for the DataFileManager class. It was written with the help of
 * ChatGPT.
 */

// #include <catch2/catch_test_macros.hpp>
#include "catch2/catch.hpp"

#include "../../source/tools/DataFileManager.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
    std::string MakeTempPath(const std::string& baseName)
    {
        namespace fs = std::filesystem;

        fs::path dir = fs::temp_directory_path();
        fs::path file = dir / (baseName + ".csv");

        // If it already exists from a previous run, remove it.
        std::error_code ec;
        fs::remove(file, ec);

        return file.string();
    }

    std::vector<std::string> ReadAllLines(const std::string& filename)
    {
        std::ifstream in(filename);
        REQUIRE(in.is_open());

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line))
        {
            lines.push_back(line);
        }
        return lines;
    }
}

TEST_CASE("DataFileManager: Open + AddColumn + Update writes header then one row", "[DataFileManager]")
{
    const std::string path = MakeTempPath("dfm_basic");

    DataFileManager dfm;
    dfm.Open(path);

    dfm.AddColumn("A", []() { return std::string("hello"); });
    dfm.AddColumn("B", []() { return std::string("42"); });
    dfm.AddColumn("C", []() { return std::string("world"); });

    dfm.Update();
    dfm.Flush();
    dfm.Close();

    auto lines = ReadAllLines(path);
    REQUIRE(lines.size() == 2);

    REQUIRE(lines[0] == "A,B,C");
    REQUIRE(lines[1] == "hello,42,world");
}

TEST_CASE("DataFileManager: Update writes header only once across multiple updates", "[DataFileManager]")
{
    const std::string path = MakeTempPath("dfm_header_once");

    DataFileManager dfm;
    dfm.Open(path);

    int counter = 0;
    dfm.AddColumn("tick", [&counter]() { return std::to_string(counter++); });
    dfm.AddColumn("fixed", []() { return std::string("X"); });

    dfm.Update();
    dfm.Update();
    dfm.Update();
    dfm.Close();

    auto lines = ReadAllLines(path);
    REQUIRE(lines.size() == 4);

    REQUIRE(lines[0] == "tick,fixed");
    REQUIRE(lines[1] == "0,X");
    REQUIRE(lines[2] == "1,X");
    REQUIRE(lines[3] == "2,X");
}

TEST_CASE("DataFileManager: Column execution order matches registration order", "[DataFileManager]")
{
    const std::string path = MakeTempPath("dfm_order");

    DataFileManager dfm;
    dfm.Open(path);

    std::vector<int> callOrder;

    dfm.AddColumn("first", [&callOrder]() {
        callOrder.push_back(1);
        return std::string("1");
    });

    dfm.AddColumn("second", [&callOrder]() {
        callOrder.push_back(2);
        return std::string("2");
    });

    dfm.AddColumn("third", [&callOrder]() {
        callOrder.push_back(3);
        return std::string("3");
    });

    dfm.Update();
    dfm.Close();

    REQUIRE(callOrder.size() == 3);
    REQUIRE(callOrder[0] == 1);
    REQUIRE(callOrder[1] == 2);
    REQUIRE(callOrder[2] == 3);

    auto lines = ReadAllLines(path);
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == "first,second,third");
    REQUIRE(lines[1] == "1,2,3");
}

TEST_CASE("DataFileManager: Re-opening truncates file (fresh schema each Open)", "[DataFileManager]")
{
    const std::string path = MakeTempPath("dfm_truncate");

    {
        DataFileManager dfm;
        dfm.Open(path);
        dfm.AddColumn("A", []() { return std::string("old"); });
        dfm.Update();
        dfm.Close();
    }

    {
        DataFileManager dfm;
        dfm.Open(path); // truncates
        dfm.AddColumn("B", []() { return std::string("new"); });
        dfm.Update();
        dfm.Close();
    }

    auto lines = ReadAllLines(path);
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == "B");
    REQUIRE(lines[1] == "new");
}

TEST_CASE("DataFileManager: Close can be called safely multiple times", "[DataFileManager]")
{
    const std::string path = MakeTempPath("dfm_close_twice");

    DataFileManager dfm;
    dfm.Open(path);

    dfm.AddColumn("A", []() { return std::string("x"); });
    dfm.Update();

    dfm.Close();
    dfm.Close(); // should be safe

    auto lines = ReadAllLines(path);
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == "A");
    REQUIRE(lines[1] == "x");
}
