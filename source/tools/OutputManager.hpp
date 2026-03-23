/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simple, unified logging utility for Group 23 (Data Analytics).
 *
 * OutputManager provides a lightweight, configurable logging interface used
 * across the World/Agent simulation. It supports multiple log levels,
 * optional timestamps and metadata, and routing messages to console, file,
 * and/or an in-memory buffer.
 *
 * Log levels (enum class LogLevel):
 *   - Silent  : no output at all
 *   - Normal  : important system messages
 *   - Verbose : additional runtime information
 *   - Debug   : fine-grained debugging details
 *
 * Filtering rules (based on the current level set by setLevel()):
 *   - Silent : print nothing
 *   - Normal : print only Normal messages
 *   - Verbose: print Normal and Verbose messages
 *   - Debug  : print all messages (Normal, Verbose, Debug)
 *
 * Example usage:
 *
 *   using namespace cse498;
 *
 *   OutputManager logger;
 *   logger.setLevel(LogLevel::Debug);
 *   logger.enableTimestamps(true);
 *   logger.enableMetadata(true);
 *   logger.enableTarget(OutputTarget::Console, true);
 *   logger.enableTarget(OutputTarget::Buffer, true);
 *   logger.openLogFile("run.log");
 *   logger.enableTarget(OutputTarget::File, true);
 *
 *   LogContext ctx;
 *   ctx.tag = "ReplayDriver";
 *   ctx.agentId = 12;
 *   ctx.tick = 530;
 *
 *   logger.logDebug("Loaded 152 actions", ctx);
 *
 *   const auto buffered = logger.getBufferedLogs();
 *
 * @note Status: PROPOSAL
 **/

#pragma once

#include <atomic>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace cse498 {

  /// \brief Logging levels used by OutputManager.
  enum class LogLevel {
    Silent = 0,
    Normal,
    Verbose,
    Debug
  };

  /// \brief Available output targets for log messages.
  enum class OutputTarget {
    Console = 0,
    File,
    Buffer
  };

  /// \brief Optional metadata that can be attached to each log message.
  struct LogContext {
    std::string tag;       ///< Optional module/system tag (e.g., "ReplayDriver")
    int         agentId = -1;   ///< Optional agent identifier (-1 = unused)
    long long   tick    = -1;   ///< Optional tick/frame (-1 = unused)
  };

  /// \brief Unified logging utility for Group 23.
  class OutputManager {
  public:
    OutputManager();
    ~OutputManager();

    /// \brief Set the current log filtering level.
    void setLevel(LogLevel level);

    /// \brief Get the current log filtering level.
    [[nodiscard]] LogLevel getLevel() const;

    /// \brief Enable or disable timestamps in log output.
    void enableTimestamps(bool on);

    /// \brief Enable or disable metadata (tag/agent/tick) in log output.
    void enableMetadata(bool on);

    /// \brief Enable or disable a specific output target.
    void enableTarget(OutputTarget target, bool on);

    /// \brief Open a log file for OutputTarget::File.
    /// \return true on success, false on failure (no exceptions thrown).
    [[nodiscard]] bool openLogFile(const std::string & path);

    /// \brief Close the currently open log file, if any.
    void closeLogFile();

    /// \brief Retrieve a copy of all buffered log lines.
    [[nodiscard]] std::vector<std::string> getBufferedLogs() const;

    /// \brief Clear all buffered log lines.
    void clearBuffer();

    // Convenience wrappers for common log levels.

    void logNormal(const std::string & message,
                   const LogContext & ctx = LogContext{});

    void logVerbose(const std::string & message,
                    const LogContext & ctx = LogContext{});

    void logDebug(const std::string & message,
                  const LogContext & ctx = LogContext{});

    /// \brief Core logging function used by all helper methods.
    void log(LogLevel level,
             const std::string & message,
             const LogContext & ctx = LogContext{});

  private:
    // Log level is atomic so that it can be read without taking the mutex
    // in fast paths (e.g., early filtering in log()).
    std::atomic<LogLevel> level{LogLevel::Normal};
    bool timestamps_enabled = false;
    bool metadata_enabled = false;

    // Enabled output targets stored as a bitmask on OutputTarget value.
    unsigned int targets_mask = 0u;

    std::ofstream file_stream;

    mutable std::mutex mutex;
    std::vector<std::string> buffer;

    // Internal helpers; all expect mutex to be held by caller.
    [[nodiscard]] bool ShouldLogUnlocked(LogLevel msg_level) const;
    [[nodiscard]] std::string FormatMessageUnlocked(LogLevel msg_level,
                                                    const std::string & message,
                                                    const LogContext & ctx) const;
    void WriteLineUnlocked(const std::string & line);
    [[nodiscard]] static const char * LevelToString(LogLevel level);
    [[nodiscard]] bool IsTargetEnabledUnlocked(OutputTarget target) const;
  };

} 

