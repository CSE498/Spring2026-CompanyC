#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../OutputManager.hpp"
#include "TestHelpers.hpp"

#include <fstream>
#include <string>
#include <thread>
#include <vector>

using namespace cse498;
using namespace cse498::test;

// ----- Log level filtering ---------------------------------------------------

TEST_CASE("Log level Silent: no output to buffer", "[OutputManager][levels]")
{
  OutputManager om;
  om.setLevel(LogLevel::Silent);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, true);

  om.logNormal("n");
  om.logVerbose("v");
  om.logDebug("d");

  auto logs = om.getBufferedLogs();
  REQUIRE(logs.empty());
}

TEST_CASE("Log level Normal: only Normal in buffer", "[OutputManager][levels]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, true);

  om.logNormal("normal-msg");
  om.logVerbose("verbose-msg");
  om.logDebug("debug-msg");

  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("normal-msg") != std::string::npos);
  REQUIRE(logs[0].find("NORMAL") != std::string::npos);
}

TEST_CASE("Log level Verbose: Normal + Verbose in buffer", "[OutputManager][levels]")
{
  OutputManager om;
  om.setLevel(LogLevel::Verbose);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, true);

  om.logNormal("n");
  om.logVerbose("v");
  om.logDebug("d");

  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 2u);
  REQUIRE(logs[0].find("NORMAL") != std::string::npos);
  REQUIRE(logs[1].find("VERBOSE") != std::string::npos);
}

TEST_CASE("Log level Debug: Normal + Verbose + Debug in buffer", "[OutputManager][levels]")
{
  OutputManager om;
  om.setLevel(LogLevel::Debug);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, true);

  om.logNormal("n");
  om.logVerbose("v");
  om.logDebug("d");

  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 3u);
  REQUIRE(logs[0].find("NORMAL") != std::string::npos);
  REQUIRE(logs[1].find("VERBOSE") != std::string::npos);
  REQUIRE(logs[2].find("DEBUG") != std::string::npos);
}

// ----- Targets: buffer only --------------------------------------------------

TEST_CASE("Buffer only: logs appear in getBufferedLogs", "[OutputManager][targets]")
{
  OutputManager om;
  om.setLevel(LogLevel::Debug);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, true);

  om.logNormal("a");
  om.logDebug("b");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 2u);
  REQUIRE(logs[0].find("a") != std::string::npos);
  REQUIRE(logs[1].find("b") != std::string::npos);
}

// ----- Targets: console redirect ---------------------------------------------

TEST_CASE("Console only: output captured via cout redirect", "[OutputManager][targets]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTarget(OutputTarget::Console, true);
  om.enableTarget(OutputTarget::File, false);
  om.enableTarget(OutputTarget::Buffer, false);

  CoutRedirect redirect;
  om.logNormal("console-only-msg");
  std::string out = redirect.str();

  REQUIRE(out.find("console-only-msg") != std::string::npos);
  REQUIRE(out.find("NORMAL") != std::string::npos);
}

// ----- Targets: file only ----------------------------------------------------

TEST_CASE("File only: message written to temp file", "[OutputManager][targets]")
{
  RemoveTempLogFile();
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Buffer, false);

  REQUIRE(om.openLogFile(TempLogPath()));
  om.logNormal("file-only-msg");
  om.closeLogFile();

  std::ifstream f(TempLogPath());
  REQUIRE(f);
  std::string line;
  REQUIRE(std::getline(f, line));
  REQUIRE(line.find("file-only-msg") != std::string::npos);
  REQUIRE(line.find("NORMAL") != std::string::npos);
  RemoveTempLogFile();
}

// ----- Multiple targets -----------------------------------------------------

TEST_CASE("Multiple targets: same message in buffer and file", "[OutputManager][targets]")
{
  RemoveTempLogFile();
  OutputManager om;
  om.setLevel(LogLevel::Debug);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Buffer, true);
  REQUIRE(om.openLogFile(TempLogPath()));

  om.logNormal("multi-msg");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("multi-msg") != std::string::npos);

  om.closeLogFile();
  std::ifstream f(TempLogPath());
  REQUIRE(f);
  std::string line;
  REQUIRE(std::getline(f, line));
  REQUIRE(line.find("multi-msg") != std::string::npos);
  RemoveTempLogFile();
}

// ----- Timestamps ------------------------------------------------------------

TEST_CASE("Timestamps disabled: no bracketed time in output", "[OutputManager][format]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTimestamps(false);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  om.logNormal("no-ts");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  // Format is [NORMAL] no-ts — no "YYYY-MM-DD" or "HH:MM:SS"
  REQUIRE(logs[0].find("NORMAL") != std::string::npos);
  REQUIRE(logs[0].find("no-ts") != std::string::npos);
  // No timestamp pattern like [digits]-[digits]-[digits]
  bool has_date_like = logs[0].find("-") != std::string::npos &&
                       (logs[0].find("202") != std::string::npos || logs[0].find(":") != std::string::npos);
  REQUIRE_FALSE(has_date_like);
}

TEST_CASE("Timestamps enabled: output contains timestamp bracket and colon", "[OutputManager][format]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTimestamps(true);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  om.logNormal("with-ts");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("[") != std::string::npos);
  REQUIRE(logs[0].find(":") != std::string::npos);
  REQUIRE(logs[0].find("]") != std::string::npos);
  REQUIRE(logs[0].find("with-ts") != std::string::npos);
}

// ----- Metadata --------------------------------------------------------------

TEST_CASE("Metadata disabled: no tag in output", "[OutputManager][format]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableMetadata(false);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  LogContext ctx;
  ctx.tag = "ReplayDriver";
  om.logNormal("msg", ctx);
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("ReplayDriver") == std::string::npos);
  REQUIRE(logs[0].find("msg") != std::string::npos);
}

TEST_CASE("Metadata enabled: tag appears in output", "[OutputManager][format]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableMetadata(true);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  LogContext ctx;
  ctx.tag = "ReplayDriver";
  om.logNormal("msg", ctx);
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("ReplayDriver") != std::string::npos);
  REQUIRE(logs[0].find("msg") != std::string::npos);
}

TEST_CASE("Metadata: agentId and tick only when provided", "[OutputManager][format]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableMetadata(true);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  LogContext no_extra;
  no_extra.tag = "T";
  om.logNormal("m1", no_extra);
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("agent=") == std::string::npos);
  REQUIRE(logs[0].find("tick=") == std::string::npos);

  om.clearBuffer();
  LogContext with_agent_tick;
  with_agent_tick.tag = "T";
  with_agent_tick.agentId = 12;
  with_agent_tick.tick = 530;
  om.logNormal("m2", with_agent_tick);
  logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("agent=12") != std::string::npos);
  REQUIRE(logs[0].find("tick=530") != std::string::npos);
}

// ----- File handling ---------------------------------------------------------

TEST_CASE("openLogFile(validPath) returns true and writes when File enabled", "[OutputManager][file]")
{
  RemoveTempLogFile();
  OutputManager om;
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Console, false);
  om.setLevel(LogLevel::Normal);

  bool ok = om.openLogFile(TempLogPath());
  REQUIRE(ok);
  om.logNormal("written");
  om.closeLogFile();

  std::ifstream f(TempLogPath());
  REQUIRE(f);
  std::string line;
  REQUIRE(std::getline(f, line));
  REQUIRE(line.find("written") != std::string::npos);
  RemoveTempLogFile();
}

TEST_CASE("openLogFile(invalidPath) returns false and does not crash", "[OutputManager][file]")
{
  OutputManager om;
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Buffer, true);
  om.setLevel(LogLevel::Normal);

  // Path that is invalid on most systems (no write permission or invalid chars)
  bool ok = om.openLogFile("/nonexistent/unwritable/path/xyz.log");
  REQUIRE_FALSE(ok);
  om.logNormal("after-fail");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("after-fail") != std::string::npos);
}

TEST_CASE("File target enabled but open failed: buffer still works", "[OutputManager][file]")
{
  OutputManager om;
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);
  om.setLevel(LogLevel::Normal);

  om.openLogFile("/invalid/path/xyz.log");
  om.logNormal("buffer-still-works");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("buffer-still-works") != std::string::npos);
}

// ----- Buffer APIs -----------------------------------------------------------

TEST_CASE("getBufferedLogs returns expected count and content", "[OutputManager][buffer]")
{
  OutputManager om;
  om.setLevel(LogLevel::Debug);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  om.logNormal("1");
  om.logVerbose("2");
  om.logDebug("3");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 3u);
  REQUIRE(logs[0].find("1") != std::string::npos);
  REQUIRE(logs[1].find("2") != std::string::npos);
  REQUIRE(logs[2].find("3") != std::string::npos);
}

TEST_CASE("clearBuffer empties buffer", "[OutputManager][buffer]")
{
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);

  om.logNormal("x");
  REQUIRE(om.getBufferedLogs().size() == 1u);
  om.clearBuffer();
  REQUIRE(om.getBufferedLogs().empty());
  om.logNormal("y");
  REQUIRE(om.getBufferedLogs().size() == 1u);
  REQUIRE(om.getBufferedLogs()[0].find("y") != std::string::npos);
}

TEST_CASE("Buffer works with multiple targets enabled", "[OutputManager][buffer]")
{
  RemoveTempLogFile();
  OutputManager om;
  om.setLevel(LogLevel::Normal);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::File, true);
  om.enableTarget(OutputTarget::Console, false);
  REQUIRE(om.openLogFile(TempLogPath()));

  om.logNormal("both");
  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == 1u);
  REQUIRE(logs[0].find("both") != std::string::npos);
  om.closeLogFile();
  RemoveTempLogFile();
}

// ----- Thread safety (basic) -------------------------------------------------

TEST_CASE("Multiple threads logging to buffer: no crash, count matches", "[OutputManager][threads]")
{
  OutputManager om;
  om.setLevel(LogLevel::Debug);
  om.enableTarget(OutputTarget::Buffer, true);
  om.enableTarget(OutputTarget::Console, false);
  om.enableTarget(OutputTarget::File, false);

  constexpr int num_threads = 4;
  constexpr int logs_per_thread = 50;
  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t)
  {
    threads.emplace_back([&om]()
                         {
      for (int i = 0; i < logs_per_thread; ++i) {
        om.logDebug("thread-msg");
      } });
  }
  for (auto &th : threads)
    th.join();

  auto logs = om.getBufferedLogs();
  REQUIRE(logs.size() == static_cast<size_t>(num_threads * logs_per_thread));
  for (const auto &line : logs)
  {
    REQUIRE(line.find("thread-msg") != std::string::npos);
    REQUIRE(line.find("DEBUG") != std::string::npos);
  }
}
