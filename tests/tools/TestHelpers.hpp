#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace cse498
{
  namespace test
  {

    /// Polling helper for async tests. Returns true if pred() becomes true
    /// before timeout_ms, false if it times out.
    template <typename Pred>
    bool WaitFor(Pred pred, int timeout_ms = 2000, int poll_interval_ms = 10)
    {
      auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
      while (!pred())
      {
        if (std::chrono::steady_clock::now() > deadline) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
      }
      return true;
    }

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
