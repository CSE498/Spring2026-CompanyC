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

# Simple executables 
EXECUTABLES = simple

# SaveServer needs extra .cpp sources for Database + WebSocket stack
SAVESERVER_SRCS = \
	core/Database.cpp \
	core/SyncManager.cpp \
	tools/WebSocketServer.cpp \
	tools/WebSocketConnection.cpp \
	tools/SQLiteConnection.cpp \
	tools/Serializer.cpp \
	tools/StringCompressor.cpp \
	tools/StringDiff.cpp \
	tools/DataGrid.cpp \
	tools/Datum.cpp

# Group09 demo stuff - in progress i dont think ready to run yet
GROUP09_SRCS = \
	core/Database.cpp \
	core/WorldHelpers.cpp \
	tools/SQLiteConnection.cpp \
	tools/Serializer.cpp \
	tools/StringCompressor.cpp \
	tools/StringDiff.cpp \
	tools/DataGrid.cpp \
	tools/Datum.cpp

default: simple
debug: debug-simple
opt: opt-simple
grumpy: grumpy-simple

.PHONY: default clean debug-% opt-% quick-% $(EXECUTABLES) SaveServer

# SaveServer specific stuff
SaveServer: SaveServer_main.cpp $(SAVESERVER_SRCS) $(HEADERS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) SaveServer_main.cpp $(SAVESERVER_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_all) -I$(IXWS_DIR)/.. -o $@

debug-SaveServer: $(SAVESERVER_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) SaveServer_main.cpp $(SAVESERVER_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_debug) -I$(IXWS_DIR)/.. -o SaveServer

opt-SaveServer: $(SAVESERVER_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) SaveServer_main.cpp $(SAVESERVER_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_opt) -I$(IXWS_DIR)/.. -o SaveServer

# Group09 demo executable
Group09: Group09_main.cpp $(GROUP09_SRCS) $(HEADERS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) Group09_main.cpp $(GROUP09_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_all) -I$(IXWS_DIR)/.. -o $@

debug-Group09: $(GROUP09_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS)
	$(CXX) Group09_main.cpp $(GROUP09_SRCS) $(SQLITE_OBJ) $(IXWS_OBJS) $(FLAGS_debug) -I$(IXWS_DIR)/.. -o Group09

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
FILES_generated = *.o $(SQLITE_OBJ) $(EXECUTABLES) SaveServer Group09

clean:
	rm -rf $(FILES_backup) $(FILES_generated) build/

