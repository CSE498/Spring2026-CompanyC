.DEFAULT_GOAL := default

FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

FLAGS_debug   = $(FLAGS_all) -g                     # Setup for running a debugger
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG           # Fastest possible executable
FLAGS_quick   = $(FLAGS_all) -DNDEBUG               # Fastest possible compilation
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++  # Some extra picky warnings

CXX  = c++
EMCC = em++

# --- SQLite (Group 9 - Database persistence) ---
SQLITE_DIR = ../third-party/sqlite
SQLITE_OBJ = sqlite3.o

$(SQLITE_OBJ): $(SQLITE_DIR)/sqlite3.c
	cc -O2 -DSQLITE_THREADSAFE=1 -c $< -o $@

# SQLite compiled for Emscripten (web build)
SQLITE_OBJ_WEB = build/sqlite3_web.o

$(SQLITE_OBJ_WEB): $(SQLITE_DIR)/sqlite3.c
	@mkdir -p build
	emcc -O2 -DSQLITE_THREADSAFE=0 -c $< -o $@

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
EXECUTABLES = agent

default: agent
debug: debug-agent
opt: opt-agent
grumpy: grumpy-agent

# Sources shared by all build targets (native and web)
BASE_SOURCES = Agents/ClassicAgent.cpp   \
               Agents/GoblinAgent.cpp    \
               Agents/TendencyAgent.cpp  \
			   Agents/HunterAgent.cpp  \
               tools/BehaviorTree.cpp    \
               tools/CompositeNodes.cpp  \
               tools/LeafNodes.cpp       \
               tools/PathGenerator.cpp   \
               tools/Timer.cpp           \
               tools/WorldPath.cpp       \
               Worlds/DynamicWorld.cpp	 \
			   Worlds/InteractionHeavyWorld.cpp

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

# ── Web / Emscripten build ────────────────────────────────────────────────────
WEB_SOURCES = web_main.cpp            \
              Interfaces/WebApp.cpp   \
               tools/WebLayout.cpp    \
               tools/WebCanvas.cpp    \
               tools/WebTextbox.cpp   \
               tools/WebInterface.cpp \
               tools/WebSocketConnection.cpp \
               tools/SQLiteConnection.cpp \
               tools/Serializer.cpp   \
               tools/StringCompressor.cpp \
               tools/StringDiff.cpp   \
               tools/DataGrid.cpp     \
               tools/Datum.cpp        \
               core/Database.cpp      \
               core/SyncManager.cpp   \
               core/WorldHelpers.cpp  \
               Worlds/StubWorld.cpp   \
              $(BASE_SOURCES)

FLAGS_web_base = -std=c++23 -I. \
                 -sWASM=1 \
                 -sEXPORTED_FUNCTIONS=_main,_HandleAction \
                 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
                 -lembind \
				 --preload-file ../source/Worlds/interaction_world_maps \
                 -lwebsocket

FLAGS_web_opt   = $(FLAGS_web_base) -O3 -DNDEBUG
FLAGS_web_debug = $(FLAGS_web_base) -O0 -g

WEB_OUT_DIR = ../web

web web-opt: $(WEB_SOURCES) $(SQLITE_OBJ_WEB)
	@mkdir -p $(WEB_OUT_DIR)
	$(EMCC) $(WEB_SOURCES) $(SQLITE_OBJ_WEB) $(FLAGS_web_opt) -o $(WEB_OUT_DIR)/demo.html

web-debug: $(WEB_SOURCES) $(SQLITE_OBJ_WEB)
	@mkdir -p $(WEB_OUT_DIR)
	$(EMCC) $(WEB_SOURCES) $(SQLITE_OBJ_WEB) $(FLAGS_web_debug) -o $(WEB_OUT_DIR)/demo.html
# ─────────────────────────────────────────────────────────────────────────────

server:
	cd $(WEB_OUT_DIR) && python3 -m http.server 8080

.PHONY: default clean web web-opt web-debug server debug-% opt-% quick-% $(EXECUTABLES) SaveServer Group09

# SaveServer specific targets
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
$(EXECUTABLES): %: %_main.cpp $(HEADERS) $(SQLITE_OBJ)
	$(CXX) $@_main.cpp $(BASE_SOURCES) $(SQLITE_OBJ) $(FLAGS_all) -I. -o $@

# Generic rule to build debug version of any executable
debug-%: $(SQLITE_OBJ)
	$(CXX) $*_main.cpp $(BASE_SOURCES) $(SQLITE_OBJ) $(FLAGS_debug) -I. -o $*

# Generic rule to build opt version of any executable
opt-%: $(SQLITE_OBJ)
	$(CXX) $*_main.cpp $(BASE_SOURCES) $(SQLITE_OBJ) $(FLAGS_opt) -I. -o $*

# Generic rule to build quick version of any executable
quick-%: $(SQLITE_OBJ)
	$(CXX) $*_main.cpp $(BASE_SOURCES) $(SQLITE_OBJ) $(FLAGS_quick) -I. -o $*

# Generic rule to build grumpy version of any executable
grumpy-%: $(SQLITE_OBJ)
	$(CXX) $*_main.cpp $(BASE_SOURCES) $(SQLITE_OBJ) $(FLAGS_grumpy) -I. -o $*

FILES_backup	= *~ *.dSYM
FILES_generated = *.o $(SQLITE_OBJ) $(SQLITE_OBJ_WEB) $(EXECUTABLES) SaveServer Group09

clean:
	rm -rf $(FILES_backup) $(FILES_generated) build/
