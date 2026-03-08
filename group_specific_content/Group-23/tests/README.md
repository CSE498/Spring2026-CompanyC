# Group 23 OutputManager unit tests

Uses **Catch2** from the repo: `third-party/Catch/single_include/`.

## Build and run (Unix / MinGW / MSYS2)

From this directory (`group_specific_content/Group-23/tests/`):

```bash
make          # build and run
make build    # build only
make clean
```

Or compile manually (from repo root):

```bash
cd group_specific_content/Group-23/tests
g++ -std=c++17 -I.. -I../../../third-party/Catch/single_include -c ../OutputManager.cpp -o OutputManager.o
g++ -std=c++17 -I.. -I../../../third-party/Catch/single_include -c OutputManagerTests.cpp -o OutputManagerTests.o
g++ -o run_outputmanager_tests OutputManager.o OutputManagerTests.o
./run_outputmanager_tests
```

C++17 is used for portability; the repo main line uses C++23.

## Test coverage

- **Log levels**: Silent, Normal, Verbose, Debug filtering (buffer only).
- **Targets**: Buffer only, console (via cout redirect), file only, multiple targets.
- **Format**: Timestamps and metadata on/off; tag, agentId, tick when provided.
- **File**: Valid path, invalid path (no crash), fallback when file open fails.
- **Buffer**: `getBufferedLogs` count/content, `clearBuffer`, buffer with multiple targets.
- **Threads**: Multiple threads logging to buffer; total count and no crash.
