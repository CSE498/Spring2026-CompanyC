FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

FLAGS_debug   = $(FLAGS_all) -g                     # Setup for running a debugger
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG           # Fastest possible executable
FLAGS_quick   = $(FLAGS_all) -DNDEBUG               # Fastest possible compilation
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++  # Some extra picky warnings

CXX = c++

# Find all headers
HEADERS := $(shell find Agents core Interfaces tools Worlds -name '*.hpp')

# List of all available executables
EXECUTABLES = agent

default: agent
debug: debug-agent
opt: opt-agentj
grumpy: grumpy-agent

.PHONY: default clean debug-% opt-% quick-% $(EXECUTABLES)

# Generic rule to build default version of any executable
$(EXECUTABLES): %: %_main.cpp $(HEADERS)
	$(CXX) $@_main.cpp $(FLAGS_all) -o $@

# Generic rule to build debug version of any executable
debug-%: 
	$(CXX) $*_main.cpp $(FLAGS_debug) -o $*

# Generic rule to build opt version of any executable
opt-%: 
	$(CXX) $*_main.cpp $(FLAGS_opt) -o $*

# Generic rule to build quick version of any executable
quick-%: 
	$(CXX) $*_main.cpp $(FLAGS_quick) -o $*

# Generic rule to build quick version of any executable
grumpy-%: 
	$(CXX) $*_main.cpp $(FLAGS_grumpy) -o $*

FILES_backup	= *~ *.dSYM
FILES_generated = *.o $(EXECUTABLES)

clean:
	rm -rf $(FILES_backup) $(FILES_generated)

FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

FLAGS_debug   = $(FLAGS_all) -g
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG
FLAGS_quick   = $(FLAGS_all) -DNDEBUG
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++

CXX = c++

# Find all headers and source files
HEADERS := $(shell find Agents core Interfaces tools Worlds web -name '*.hpp')
SOURCES := $(filter-out tools/OutputManagerDemo.cpp, \
           $(shell find Agents core Interfaces tools Worlds web -name '*.cpp'))

# List of all available executables
EXECUTABLES = agent

default: agent
debug: debug-agent
opt: opt-agent
grumpy: grumpy-agent

.PHONY: default clean debug-% opt-% quick-% grumpy-% $(EXECUTABLES)

# Default build
$(EXECUTABLES): %: %_main.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(FLAGS_all) $@_main.cpp $(SOURCES) -I. -o $@

# Debug build
debug-%:
	$(CXX) $(FLAGS_debug) $*_main.cpp $(SOURCES) -I. -o $*

# Optimized build
opt-%:
	$(CXX) $(FLAGS_opt) $*_main.cpp $(SOURCES) -I. -o $*

# Quick build
quick-%:
	$(CXX) $(FLAGS_quick) $*_main.cpp $(SOURCES) -I. -o $*

# Grumpy build
grumpy-%:
	$(CXX) $(FLAGS_grumpy) $*_main.cpp $(SOURCES) -I. -o $*

FILES_backup    = *~ *.dSYM
FILES_generated = *.o $(EXECUTABLES)

clean:
	rm -rf $(FILES_backup) $(FILES_generated)