FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

FLAGS_debug   = $(FLAGS_all) -g                     # Setup for running a debugger
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG           # Fastest possible executable
FLAGS_quick   = $(FLAGS_all) -DNDEBUG               # Fastest possible compilation
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++  # Some extra picky warnings

CXX = c++

# --- SQLite (Group 9 - Database persistence) ---
SQLITE_DIR = ../third-party/sqlite
SQLITE_OBJ = sqlite3.o

$(SQLITE_OBJ): $(SQLITE_DIR)/sqlite3.c
	cc -O2 -DSQLITE_THREADSAFE=1 -c $< -o $@

# --- IXWebSocket (Group 9 - WebSocket) ---
IXWS_DIR = ../third-party/ixwebsocket
IXWS_SRCS = $(wildcard $(IXWS_DIR)/*.cpp)
IXWS_OBJS = $(patsubst $(IXWS_DIR)/%.cpp, build/ixws/%.o, $(IXWS_SRCS))

$(IXWS_OBJS): build/ixws/%.o: $(IXWS_DIR)/%.cpp
	@mkdir -p build/ixws
	$(CXX) -std=c++17 -O2 -I$(IXWS_DIR)/.. -c $< -o $@

# Find all headers
HEADERS := $(shell find Agents core Interfaces tools Worlds -name '*.hpp')

# List of all available executables
EXECUTABLES = simple

default: simple
debug: debug-simple
opt: opt-simple
grumpy: grumpy-simple

.PHONY: default clean debug-% opt-% quick-% $(EXECUTABLES)

# Generic rule to build default version of any executable
$(EXECUTABLES): %: %_main.cpp $(HEADERS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) $@_main.cpp $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_all) -o $@

# Generic rule to build debug version of any executable
debug-%: $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) $*_main.cpp $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_debug) -o $*

# Generic rule to build opt version of any executable
opt-%: $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) $*_main.cpp $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_opt) -o $*

# Generic rule to build quick version of any executable
quick-%: $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) $*_main.cpp $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_quick) -o $*

# Generic rule to build quick version of any executable
grumpy-%: $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) $*_main.cpp $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_grumpy) -o $*

FILES_backup	= *~ *.dSYM
FILES_generated = *.o $(SQLITE_OBJ) $(EXECUTABLES)

clean:
	rm -rf $(FILES_backup) $(FILES_generated) build/

