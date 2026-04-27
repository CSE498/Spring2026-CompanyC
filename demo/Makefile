FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

FLAGS_debug   = $(FLAGS_all) -g                     # Setup for running a debugger
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG           # Fastest possible executable
FLAGS_quick   = $(FLAGS_all) -DNDEBUG               # Fastest possible compilation
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++  # Some extra picky warnings

CXX = c++

# Path to shared source headers (adjust if your layout differs)
INCLUDE = -I../source

# Find all headers under ../source
HEADERS := $(shell find ../source -name '*.hpp' 2>/dev/null)

# List of all available executables
EXECUTABLES = simple Group23

default: Group23
debug: debug-Group23
opt: opt-Group23
grumpy: grumpy-Group23

.PHONY: default clean debug-% opt-% quick-% grumpy-% $(EXECUTABLES)

# Special rule for Group23 (source file is Group23_demo.cpp, not Group23_main.cpp)
Group23: Group23_demo.cpp $(HEADERS)
	$(CXX) Group23_demo.cpp \
	    ../source/tools/OutputManager.cpp \
	    ../source/tools/Timer.cpp \
	    $(FLAGS_all) $(INCLUDE) -o Group23

debug-Group23:
	$(CXX) Group23_demo.cpp $(FLAGS_debug) $(INCLUDE) -o Group23

opt-Group23:
	$(CXX) Group23_demo.cpp $(FLAGS_opt) $(INCLUDE) -o Group23

quick-Group23:
	$(CXX) Group23_demo.cpp $(FLAGS_quick) $(INCLUDE) -o Group23

grumpy-Group23:
	$(CXX) Group23_demo.cpp $(FLAGS_grumpy) $(INCLUDE) -o Group23

# Generic rule to build default version of any other executable (*_main.cpp pattern)
$(filter-out Group23, $(EXECUTABLES)): %: %_main.cpp $(HEADERS)
	$(CXX) $@_main.cpp $(FLAGS_all) $(INCLUDE) -o $@

# Generic rules for other executables
debug-%:
	$(CXX) $*_main.cpp $(FLAGS_debug) $(INCLUDE) -o $*

opt-%:
	$(CXX) $*_main.cpp $(FLAGS_opt) $(INCLUDE) -o $*

quick-%:
	$(CXX) $*_main.cpp $(FLAGS_quick) $(INCLUDE) -o $*

grumpy-%:
	$(CXX) $*_main.cpp $(FLAGS_grumpy) $(INCLUDE) -o $*

FILES_backup    = *~ *.dSYM
FILES_generated = *.o $(EXECUTABLES)

clean:
	rm -rf $(FILES_backup) $(FILES_generated)