#pragma once

#include <cstdio>
#include <sstream>
#include <string>

namespace cse498
{
  namespace test
  {

    /** Path to a temp log file under Group-23/tests/ for file tests. */
    inline std::string TempLogPath() { return "tmp_run.log"; }

    /** Remove temp log file if present. Call from test teardown. */
    inline void RemoveTempLogFile(const std::string &path = TempLogPath())
    {
      std::remove(path.c_str());
    }

    /**
     * RAII guard that redirects std::cout to an internal ostringstream.
     * Restores the original cout buffer on destruction.
     */
    class CoutRedirect
    {
    public:
      CoutRedirect() : old_(std::cout.rdbuf())
      {
        std::cout.rdbuf(capture_.rdbuf());
      }
      ~CoutRedirect() { std::cout.rdbuf(old_); }
      std::string str() const { return capture_.str(); }
      void clear()
      {
        capture_.str("");
        capture_.clear();
      }

    private:
      std::streambuf *old_;
      std::ostringstream capture_;
    };

  } // namespace test
} // namespace cse498
