/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Demo program for Group 23 OutputManager.
 *
 * This small example enables console and buffer logging, writes a few
 * messages at different levels, enables timestamps and metadata, and
 * logs to a file named "run.log".
 **/

#include "OutputManager.hpp"

#include <iostream>

using namespace cse498;

int main()
{
  OutputManager logger;

  logger.setLevel(LogLevel::Debug);          // Show all messages.
  logger.enableTimestamps(true);             // Include timestamps.
  logger.enableMetadata(true);               // Include tag/agent/tick when provided.

  logger.enableTarget(OutputTarget::Console, true);
  logger.enableTarget(OutputTarget::Buffer, true);

  if (!logger.openLogFile("run.log")) {
    std::cout << "Warning: could not open run.log for writing; "
              << "continuing with console + buffer only.\n";
  } else {
    logger.enableTarget(OutputTarget::File, true);
  }

  LogContext ctx_normal;
  ctx_normal.tag = "OutputDemo";
  logger.logNormal("Normal level message.", ctx_normal);

  LogContext ctx_verbose;
  ctx_verbose.tag = "OutputDemo";
  ctx_verbose.agentId = 12;
  ctx_verbose.tick = 42;
  logger.logVerbose("Verbose level message with agent and tick.", ctx_verbose);

  LogContext ctx_debug;
  ctx_debug.tag = "OutputDemo";
  ctx_debug.agentId = 7;
  ctx_debug.tick = 530;
  logger.logDebug("Debug level message.", ctx_debug);

  // Show buffered content.
  const auto buffered = logger.getBufferedLogs();
  std::cout << "\nBuffered log entries (" << buffered.size() << "):\n";
  for (const auto & line : buffered) {
    std::cout << line << '\n';
  }

  return 0;
}

