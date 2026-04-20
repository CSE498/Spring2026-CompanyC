# Makefile for Catch2 unit tests
#
# Assumptions:
# - Production sources live under: source/
# - Tests live under: tests/ (and mirror source/ directory structure)
#
# Usage:
#   make            # build + run tests
#   make test       # run tests
#   make build      # build test binary only
#   make clean

SRC_DIR   := ../source
TEST_DIR  := ../tests
BUILD_DIR := ../tests/build/
BIN       := $(BUILD_DIR)/run_tests

CATCH_INC := ../third-party/Catch/single_include

# SQLite - (Group 9)
SQLITE_DIR := ../third-party/sqlite
SQLITE_OBJ := $(BUILD_DIR)/third-party/sqlite3.o

# IXWebSocket - (Group 9)
IXWS_DIR := ../third-party/ixwebsocket
IXWS_SRCS := $(wildcard $(IXWS_DIR)/*.cpp)
IXWS_OBJS := $(patsubst $(IXWS_DIR)/%.cpp, $(BUILD_DIR)/third-party/ixws_%.o, $(IXWS_SRCS))

CXXFLAGS ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC) -I$(IXWS_DIR)/..
LDFLAGS  ?=
LDLIBS   ?=

# Emscripten-only source files — cannot be compiled natively
# Tests for these live in tests/web/ and are excluded below
EMSCRIPTEN_ONLY    := WebLayout WebCanvas WebButton WebImage WebTextbox WebApp
EMSCRIPTEN_EXCLUDE := $(foreach f,$(EMSCRIPTEN_ONLY),%/$(f).cpp)

# Pick up .cpp from tests/
# Exclude tests/web/ (Emscripten-only) and stray _main.cpp files
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp')
TEST_FILES := $(filter-out $(TEST_DIR)/web/%,$(TEST_FILES))
TEST_FILES := $(filter-out %_main.cpp,$(TEST_FILES))

# Pick up source .cpp files matching a .hpp, excluding Emscripten-only,
# _main entry points, and Backups/
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp'),$(SRC_FILES))
SRC_FILES  := $(filter-out $(EMSCRIPTEN_EXCLUDE),$(SRC_FILES))
SRC_FILES  := $(filter-out %_main.cpp,$(SRC_FILES))
SRC_FILES  := $(filter-out %/Backups/%,$(SRC_FILES))

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES))

DEP_FILES := $(OBJ_FILES:.o=.d)

.PHONY: all build test web clean list

all: test

build: $(BIN)

test: $(BIN)
	$(BIN)

# Link test runner
$(BIN): $(OBJ_FILES) $(SQLITE_OBJ) $(IXWS_OBJS) | $(BUILD_DIR)
	$(CXX) -std=c++2b $(CXXFLAGS) $(WARNFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile SQLite amalgamation (C, not C++)
$(SQLITE_OBJ): $(SQLITE_DIR)/sqlite3.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	cc -O2 -DSQLITE_THREADSAFE=1 -c $< -o $@

# Compile IXWebSocket sources (C++17)
$(BUILD_DIR)/third-party/ixws_%.o: $(IXWS_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++17 -O2 -I$(IXWS_DIR)/.. -MMD -MP -c $< -o $@

# Compile source/ -> build/tests/source/...
$(BUILD_DIR)/source/%.o: $(SRC_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++2b $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile tests/ -> build/tests/tests/...
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++2b $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Sanity helper
list:
	@echo "Sources:"; echo "$(SRC_FILES)" | tr ' ' '\n'
	@echo
	@echo "Tests:";   echo "$(TEST_FILES)" | tr ' ' '\n'
	@echo
	@echo "Objects:"; echo "$(OBJ_FILES)" | tr ' ' '\n'

web:
	$(MAKE) -C web

clean:
	rm -rf build

# Auto-deps
-include $(DEP_FILES)
