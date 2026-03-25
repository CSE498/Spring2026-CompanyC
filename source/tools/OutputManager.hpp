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
 *   // Optional: route each formatted line to custom code (e.g. remote sink, metrics).
 *   logger.addOutputHandler([](const std::string & line) { (void)line; });
 *
 * @note Status: PROPOSAL
 **/

#pragma once

#include <fstream>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>

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

  /// Receives each fully formatted log line (same string sent to console/file/buffer).
  using OutputLineHandler = std::function<void(const std::string & line)>;

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

    /// \brief Register a callable invoked for every emitted line (after level filtering).
    /// Empty handlers are ignored. Runs under the same mutex as other sinks; keep work short.
    void addOutputHandler(OutputLineHandler handler);

    /// \brief Remove all handlers added with addOutputHandler.
    void clearOutputHandlers();

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

    /// \brief Template overload for logging arbitrary single values.
    ///
    /// The value is converted to a string using `operator<<` into an
    /// `std::ostringstream`, then routed through the same formatting and
    /// sink pipeline as the standard `log(LogLevel, std::string, LogContext)`.
    template <typename T>
    void log(LogLevel level, const T &message, const LogContext &ctx = LogContext{})
    {
      log(level, argsToString(message), ctx);
    }

    /// \brief Variadic formatted logging (concatenation).
    ///
    /// All arguments are appended (no delimiter) using `operator<<` into an
    /// `std::ostringstream`. This is useful for mixing strings and numeric
    /// values.
    ///
    /// Runs the same level filtering, timestamp/metadata formatting, and sink
    /// routing as the standard `log()` method.
    template <typename... Args>
    requires(sizeof...(Args) > 0)
    void logf(LogLevel level, Args &&...args)
    {
      log(level, argsToString(std::forward<Args>(args)...), LogContext{});
    }

    /// \brief Variadic formatted logging with LogContext.
    template <typename... Args>
    requires(sizeof...(Args) > 0)
    void logf(LogLevel level, const LogContext &ctx, Args &&...args)
    {
      log(level, argsToString(std::forward<Args>(args)...), ctx);
    }

    /// \brief Convenience variadic overload (same behavior as `logf`).
    ///
    /// This overload exists primarily so you can write:
    ///   `logger.log(LogLevel::Debug, "Value:", x, "Status:", status);`
    /// without needing to call `logf`.
    template <typename... Args>
    requires(sizeof...(Args) > 1)
    void log(LogLevel level, Args &&...args)
    {
      log(level, argsToString(std::forward<Args>(args)...), LogContext{});
    }

  private:
    LogLevel level = LogLevel::Normal;
    bool timestamps_enabled = false;
    bool metadata_enabled = false;

    // Enabled output targets stored as a bitmask on OutputTarget value.
    unsigned int targets_mask = 0u;

    std::ofstream file_stream;

    mutable std::mutex mutex;
    std::vector<std::string> buffer;
    std::vector<OutputLineHandler> output_handlers;

    // Internal helpers; all expect mutex to be held by caller.
    [[nodiscard]] bool shouldLogUnlocked(LogLevel msg_level) const;
    [[nodiscard]] std::string formatMessageUnlocked(LogLevel msg_level,
                                                    const std::string & message,
                                                    const LogContext & ctx) const;
    void writeLineUnlocked(const std::string & line);
    [[nodiscard]] static const char * levelToString(LogLevel level);
    [[nodiscard]] bool isTargetEnabledUnlocked(OutputTarget target) const;

    // Centralized conversion for template-based logging: uses stream insertion.
    template <typename... Args>
    static std::string argsToString(Args &&...args)
    {
      std::ostringstream out;
      (out << ... << std::forward<Args>(args));
      return out.str();
    }
  };

} 

