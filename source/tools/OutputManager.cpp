#include "OutputManager.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cse498
{

  namespace
  {
    inline unsigned int TargetBit(OutputTarget target)
    {
      return 1u << static_cast<unsigned int>(target);
    }
  }

  OutputManager::OutputManager()
  {
    targets_mask |= TargetBit(OutputTarget::Console);
  }

  OutputManager::~OutputManager()
  {
    std::lock_guard<std::mutex> guard(mutex);
    if (file_stream.is_open())
    {
      file_stream.close();
    }
  }

  void OutputManager::setLevel(LogLevel in_level)
  {
    // Relaxed ordering is sufficient: log() only needs a consistent value,
    // not a synchronization point with other state protected by mutex.
    level.store(in_level, std::memory_order_relaxed);
  }

  LogLevel OutputManager::getLevel() const
  {
    return level.load(std::memory_order_relaxed);
  }

  void OutputManager::enableTimestamps(bool on)
  {
    std::lock_guard<std::mutex> guard(mutex);
    timestamps_enabled = on;
  }

  void OutputManager::enableMetadata(bool on)
  {
    std::lock_guard<std::mutex> guard(mutex);
    metadata_enabled = on;
  }

  void OutputManager::enableTarget(OutputTarget target, bool on)
  {
    std::lock_guard<std::mutex> guard(mutex);
    const unsigned int bit = TargetBit(target);
    if (on)
    {
      targets_mask |= bit;
    }
    else
    {
      targets_mask &= ~bit;
    }
  }

  bool OutputManager::openLogFile(const std::string &path)
  {
    std::lock_guard<std::mutex> guard(mutex);

    if (file_stream.is_open())
    {
      file_stream.close();
    }

    file_stream.clear();
    file_stream.open(path.c_str(), std::ios::out | std::ios::trunc);

    const bool success = static_cast<bool>(file_stream);

    if (!success)
    {
      targets_mask |= TargetBit(OutputTarget::Console);
      return false;
    }

    return true;
  }

  void OutputManager::closeLogFile()
  {
    std::lock_guard<std::mutex> guard(mutex);
    if (file_stream.is_open())
    {
      file_stream.close();
    }
  }

  std::vector<std::string> OutputManager::getBufferedLogs() const
  {
    std::lock_guard<std::mutex> guard(mutex);
    return buffer;
  }

  void OutputManager::clearBuffer()
  {
    std::lock_guard<std::mutex> guard(mutex);
    buffer.clear();
  }

  void OutputManager::logNormal(const std::string &message,
                                const LogContext &ctx)
  {
    log(LogLevel::Normal, message, ctx);
  }

  void OutputManager::logVerbose(const std::string &message,
                                 const LogContext &ctx)
  {
    log(LogLevel::Verbose, message, ctx);
  }

  void OutputManager::logDebug(const std::string &message,
                               const LogContext &ctx)
  {
    log(LogLevel::Debug, message, ctx);
  }

  void OutputManager::log(LogLevel msg_level,
                          const std::string &message,
                          const LogContext &ctx)
  {
    // Fast-path: avoid taking the mutex at all if this message would be
    // filtered out at the current log level.
    if (!ShouldLogUnlocked(msg_level))
    {
      return;
    }

    std::lock_guard<std::mutex> guard(mutex);

    // Re-check under the mutex in case the level changed concurrently
    // between the fast-path check and acquiring the lock.
    if (!ShouldLogUnlocked(msg_level))
    {
      return;
    }

    const std::string line = FormatMessageUnlocked(msg_level, message, ctx);
    WriteLineUnlocked(line);
  }

  bool OutputManager::ShouldLogUnlocked(LogLevel msg_level) const
  {
    const LogLevel current_level = level.load(std::memory_order_relaxed);

    switch (current_level)
    {
    case LogLevel::Silent:
      return false;
    case LogLevel::Normal:
      return (msg_level == LogLevel::Normal);
    case LogLevel::Verbose:
      return (msg_level == LogLevel::Normal ||
              msg_level == LogLevel::Verbose);
    case LogLevel::Debug:
      return true;
    }
    return false;
  }

  std::string OutputManager::FormatMessageUnlocked(LogLevel msg_level,
                                                   const std::string &message,
                                                   const LogContext &ctx) const
  {
    std::ostringstream out;

    if (timestamps_enabled)
    {
      using clock = std::chrono::system_clock;
      const auto now = clock::now();
      const std::time_t now_time = clock::to_time_t(now);

      std::tm time_info{};
#if defined(_WIN32)
      localtime_s(&time_info, &now_time);
#else
      localtime_r(&now_time, &time_info);
#endif

      out << '[' << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
    }

    out << '[' << LevelToString(msg_level) << "] ";

    if (metadata_enabled)
    {
      if (!ctx.tag.empty())
      {
        out << '[' << ctx.tag << "] ";
      }

      const bool have_agent = (ctx.agentId >= 0);
      const bool have_tick = (ctx.tick >= 0);

      if (have_agent || have_tick)
      {
        out << '(';
        bool first = true;
        if (have_agent)
        {
          out << "agent=" << ctx.agentId;
          first = false;
        }
        if (have_tick)
        {
          if (!first)
            out << ' ';
          out << "tick=" << ctx.tick;
        }
        out << ") ";
      }
    }

    out << message;
    return out.str();
  }

  void OutputManager::WriteLineUnlocked(const std::string &line)
  {
    if (IsTargetEnabledUnlocked(OutputTarget::Console))
    {
      std::cout << line << '\n';
    }

    if (IsTargetEnabledUnlocked(OutputTarget::File))
    {
      if (file_stream.is_open())
      {
        file_stream << line << '\n';
      }
      else
      {
        std::cout << line << '\n';
      }
    }

    if (IsTargetEnabledUnlocked(OutputTarget::Buffer))
    {
      buffer.push_back(line);
    }
  }

  const char *OutputManager::LevelToString(LogLevel in_level)
  {
    switch (in_level)
    {
    case LogLevel::Silent:
      return "SILENT";
    case LogLevel::Normal:
      return "NORMAL";
    case LogLevel::Verbose:
      return "VERBOSE";
    case LogLevel::Debug:
      return "DEBUG";
    }
    return "UNKNOWN";
  }

  bool OutputManager::IsTargetEnabledUnlocked(OutputTarget target) const
  {
    const unsigned int bit = TargetBit(target);
    return (targets_mask & bit) != 0u;
  }

}
