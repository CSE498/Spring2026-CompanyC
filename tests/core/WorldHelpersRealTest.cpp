#include "catch2/catch.hpp"
#include "../tools/TestHelpers.hpp"
#include "../../source/core/WorldHelpers.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/Agents/PacingAgent.hpp"
#include "../../source/core/SyncManager.hpp"
#include "../../source/tools/WebSocketConnection.hpp"
#include "../../source/tools/WebSocketServer.hpp"
#include <filesystem>

using namespace cse498;
using cse498::test::WaitFor;

namespace {

constexpr int REAL_WH_TEST_PORT = 18096;

} // namespace

TEST_CASE("WorldHelpersReal - MazeWorld + PacingAgent round-trip", "[world-helpers-real]") {
    Database db;

    // Build a MazeWorld and add a PacingAgent
    MazeWorld world;
    auto& agent = world.AddAgent<PacingAgent>("Explorer");
    agent.SetLocation(WorldPosition(3.0, 5.0));
    agent.SetSymbol('E');

    REQUIRE(SaveWorld(db, "maze", world).has_value());

    // Verify expected keys exist
    REQUIRE(db.Exists("world:maze:meta"));
    REQUIRE(db.Exists("world:maze:grid"));
    REQUIRE(db.Exists("world:maze:agent:0"));

    // Load into a fresh MazeWorld with the same structure
    MazeWorld world2;
    world2.AddAgent<PacingAgent>("placeholder");

    REQUIRE(LoadWorld(db, "maze", world2).has_value());

    // Verify base-class agent fields restored
    auto& restored = world2.GetAgent(0);
    REQUIRE(restored.GetName() == "Explorer");
    REQUIRE(restored.GetSymbol() == 'E');
    REQUIRE(restored.GetLocation().IsPosition());
    REQUIRE(restored.GetLocation().AsWorldPosition().X() == Approx(3.0));
    REQUIRE(restored.GetLocation().AsWorldPosition().Y() == Approx(5.0));

    // Verify grid dimensions match MazeWorld's 23x11 maze
    REQUIRE(world2.GetGrid().GetWidth() == 23);
    REQUIRE(world2.GetGrid().GetHeight() == 11);
}

TEST_CASE("WorldHelpersReal - MazeWorld grid cell preservation", "[world-helpers-real]") {
    Database db;

    MazeWorld world;
    const auto& grid = world.GetGrid();
    REQUIRE(grid.GetWidth() == 23);
    REQUIRE(grid.GetHeight() == 11);

    REQUIRE(SaveWorld(db, "grid_check", world).has_value());

    MazeWorld world2;
    REQUIRE(LoadWorld(db, "grid_check", world2).has_value());

    const auto& grid2 = world2.GetGrid();
    REQUIRE(grid2.GetWidth() == 23);
    REQUIRE(grid2.GetHeight() == 11);

    // Verify every cell matches (23x11 = 253 cells)
    for (size_t y = 0; y < grid.GetHeight(); ++y) {
        for (size_t x = 0; x < grid.GetWidth(); ++x) {
            REQUIRE(grid2[x, y] == grid[x, y]);
        }
    }
}

TEST_CASE("WorldHelpersReal - SaveWorld/SaveGame/LoadGame/LoadWorld full pipeline", "[world-helpers-real]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "wh_real_savegame_test";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    // -- Server side --
    Database server_db;
    WebSocketServer ws_server(REAL_WH_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    // -- Client side --
    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(REAL_WH_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return ws_server.ClientCount() > 0;
    }));

    // Drain initial FULL_STATE
    for (int i = 0; i < 5; ++i) {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // -- Build a MazeWorld with a PacingAgent --
    MazeWorld world;
    auto& agent = world.AddAgent<PacingAgent>("Maze Runner");
    agent.SetLocation(WorldPosition(5.0, 3.0));
    agent.SetSymbol('M');

    // Serialize world into client DB
    REQUIRE(SaveWorld(client_db, "real_test", world).has_value());
    (void)client_db.FlushDirty();

    // Upload to server
    REQUIRE(client_sync.SaveGame("maze_real_save").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "maze_real_save.save");
    }));

    // Clear client DB and download
    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    REQUIRE(client_sync.LoadGame("maze_real_save").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() > 0;
    }));

    // Restore world from downloaded DB
    MazeWorld world2;
    world2.AddAgent<PacingAgent>("placeholder");

    REQUIRE(LoadWorld(client_db, "real_test", world2).has_value());

    // Verify agent state survived the full pipeline
    auto& restored = world2.GetAgent(0);
    REQUIRE(restored.GetName() == "Maze Runner");
    REQUIRE(restored.GetSymbol() == 'M');
    REQUIRE(restored.GetLocation().IsPosition());
    REQUIRE(restored.GetLocation().AsWorldPosition().X() == Approx(5.0));
    REQUIRE(restored.GetLocation().AsWorldPosition().Y() == Approx(3.0));

    // Verify grid survived the full pipeline
    REQUIRE(world2.GetGrid().GetWidth() == 23);
    REQUIRE(world2.GetGrid().GetHeight() == 11);
    // Spot-check: top-left corner of MazeWorld is a wall
    REQUIRE(world2.GetGrid()[0, 0] == world.GetGrid()[0, 0]);

    // Cleanup
    server_sync.Stop();
    client_sync.Stop();
    (void)ws_client.Disconnect();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}
