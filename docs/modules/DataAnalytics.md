# Data Analytics Module

**Documented By:** Group 23  
**Developed By:** Collin Massmann, Lauren Phillips, Muhammad Chohan, Meghan Carter, and Ismail Abdi

---

## **0** Introduction

The Data Analytics module provides the project's instrumentation and observability layer. Its purpose is to record what happens during a simulation run, summarize important metrics, replay recorded behavior, and publish structured logs for debugging and analysis.

Unlike the World or Web Interface modules, this module does not drive the simulation directly. Instead, it measures and reports on the rest of the system so developers and future user-facing tools can better understand runtime behavior, performance, and agent decisions.

## **1** Main Structure

### **1.1** Core Classes

The module is centered around five utilities located in `source/tools`.

`ActionLog` records actions performed by agents. Each entry stores the action time, the action type, and the action duration. The class organizes this data by agent ID so analytics can inspect one agent's history or the full set of actions for a run.

`DataLog<T>` stores sampled values and exposes summary statistics over those samples. For numeric types it can compute mean and median, and for ordered types it can return minimum and maximum values. The default use case is numeric data, but the template also supports storing other ordered values for later analysis.

`Timer` measures named sections of the system, such as world updates, decision making, replay, rendering, or logging. It tracks the number of completed runs and keeps the last, minimum, maximum, total, and average duration for each named timer.

`ReplayDriver` reconstructs a previously recorded action history by loading entries from an `ActionLog`, sorting them chronologically, and replaying them back through a `WorldBase` one event at a time. This supports reproducible debugging and step-by-step inspection of behavior.

`OutputManager` is the shared logging utility for the module. It filters messages by log level, optionally adds timestamps and metadata, and routes formatted output to the console, a file, and/or an in-memory buffer.

### **1.2** Dependencies

The Data Analytics module depends on standard C++ utilities such as `vector`, `unordered_map`, `string`, `chrono`, `atomic`, `mutex`, `fstream`, and common algorithm helpers.

At the project level, it integrates with core framework types including `AgentBase`, `WorldBase`, `Database`, and `Serializer`. The replay system assumes that the world can expose agents by ID and execute actions through the common world interface.

### **1.3** APIs and Interactions

This module receives most of its useful input from the rest of the system. The World and Agent modules generate actions and execution flow that can be recorded by `ActionLog`, measured by `Timer`, and reported through `OutputManager`.

`ReplayDriver` depends on previously collected action data and feeds those actions back into the world for deterministic playback. `ActionLog` also supports database registration so action entries can be serialized and stored when persistence is required.

The Web Interface module or future reporting tools can consume analytics output in several ways: summary statistics from `DataLog`, replayed sessions from `ReplayDriver`, or structured log output from `OutputManager`. This gives the larger project a path toward post-run dashboards, debugging overlays, and developer diagnostics.

### **1.4** Build and Testing

The module is implemented under `source/tools`, with tests in `tests/tools` and an example integration driver in `demo/Group23_demo.cpp`. It is intended to compile as part of the normal project build rather than as a standalone subsystem.

## **2** Features

### **2.1** Runtime Event Capture

The module records per-agent actions, timestamps, and durations so that developers can inspect how agents behaved over time. This is useful for debugging unexpected decisions, validating execution order, and measuring how long actions take.

### **2.2** Statistical Summaries

`DataLog` supports collecting a sequence of samples and producing lightweight summaries such as mean, median, minimum, and maximum. This makes it suitable for tracking gameplay metrics, performance samples, or other values gathered during a run.

### **2.3** Performance Profiling

`Timer` supports multiple named timers so different subsystems can be measured independently. This makes it possible to compare costs across world updates, AI decisions, replay steps, rendering paths, or logging operations without mixing unrelated data together.

### **2.4** Deterministic Replay

`ReplayDriver` replays actions in chronological order and advances one event per update call. This allows a recorded session to be reproduced in a controlled way, which is useful for debugging order-dependent bugs and validating agent behavior.

### **2.5** Structured Logging

`OutputManager` provides four logging levels: `Silent`, `Normal`, `Verbose`, and `Debug`. It can also attach timestamps and metadata such as module tag, agent ID, and tick number, giving the project a single consistent place to manage runtime output.

## **3** Public Interface Highlights

The following interfaces represent the main entry points other modules are most likely to use:

```cpp
// ActionLog
void recordAction(size_t id, size_t action, std::chrono::microseconds time);
void actionEnd(size_t id, std::chrono::microseconds time);
const std::unordered_map<size_t, std::vector<ActionEntry>>& getActions() const;
const std::vector<ActionEntry>& getActionsByAgent(size_t id) const;

// DataLog<T>
void Add(const T& value);
double Mean() const;
double Median() const;
T Min() const;
T Max() const;
std::size_t Count() const;
bool IsEmpty() const;
void Clear();

// Timer
void Start(const std::string& name);
void Stop(const std::string& name);
double Average(const std::string& name) const;
std::size_t Count(const std::string& name) const;

// ReplayDriver
void startReplay(const ActionLog& log);
void update();
void pauseReplay();
void resumeReplay();

// OutputManager
void setLevel(LogLevel level);
void enableTimestamps(bool on);
void enableMetadata(bool on);
void enableTarget(OutputTarget target, bool on);
void log(LogLevel level, const std::string& message, const LogContext& ctx = {});
```

## **4** Summary

The Data Analytics module gives the project a common way to observe, measure, and replay simulation behavior. In practical terms, it supports debugging, profiling, logging, and future post-run analysis features without forcing those concerns into the gameplay or world modules themselves.
