# Group 23 OutputManager unit tests (Catch2). Run from this directory (Group-23/tests/).
# Usage: make          # build and run
#        make build    # build only
#        make clean

CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -Wall -Wextra -O2 -g
CATCH_INC := ../../../third-party/Catch/single_include
TOP_GROUP := ..
CPPFLAGS  := -I$(TOP_GROUP) -I$(CATCH_INC)

BIN       := run_outputmanager_tests
OBJS      := OutputManager.o OutputManagerTests.o

.PHONY: all build run clean

all: run

build: $(BIN)

run: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(BIN)

OutputManager.o: $(TOP_GROUP)/OutputManager.cpp $(TOP_GROUP)/OutputManager.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TOP_GROUP)/OutputManager.cpp -o OutputManager.o

OutputManagerTests.o: OutputManagerTests.cpp TestHelpers.hpp $(TOP_GROUP)/OutputManager.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c OutputManagerTests.cpp -o OutputManagerTests.o

clean:
	rm -f $(OBJS) $(BIN) tmp_run.log
