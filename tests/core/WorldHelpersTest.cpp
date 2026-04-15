#include "catch2/catch.hpp"
#include "../../source/core/WorldHelpers.hpp"
#include "../../source/core/SyncManager.hpp"
#include "../../source/tools/WebSocketConnection.hpp"
#include "../../source/tools/WebSocketServer.hpp"
#include <thread>
#include <chrono>
#include <filesystem>

using namespace cse498;

namespace {

class TestAgent : public AgentBase {
public:
    TestAgent(size_t id, const std::string& name, const WorldBase& world)
        : AgentBase(id, name, world) {}
    size_t SelectAction(WorldGrid&) override { return 0; }
};

class TestWorld : public WorldBase {
public:
    TestWorld() : WorldBase() {}

    int DoAction(AgentBase&, size_t) override { return 0; }

    ItemBase& AddItem(const std::string& name) {
        auto ptr = std::make_unique<ItemBase>(item_set.size(), name, *this);
        ItemBase& ref = *ptr;
        item_set.emplace_back(std::move(ptr));
        return ref;
    }

    void SetRunOver(bool val) { run_over = val; }
};

template <typename Pred>
bool WaitFor(Pred pred, int timeout_ms = 2000, int poll_interval_ms = 10) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (!pred()) {
        if (std::chrono::steady_clock::now() > deadline) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }
    return true;
}

constexpr int WH_TEST_PORT = 18095;

} // namespace

TEST_CASE("WorldHelpers - scaffolding compiles", "[world-helpers]") {
    Database db;
    TestWorld world;
    REQUIRE(SaveWorld(db, "test", world).has_value());
    REQUIRE(LoadWorld(db, "test", world).has_value());
}

TEST_CASE("WorldHelpers - metadata round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    // Add 2 agents and 1 item so counts are non-trivial
    world.AddAgent<TestAgent>("agent_a");
    world.AddAgent<TestAgent>("agent_b");
    world.AddItem("item_a");

    REQUIRE(SaveWorld(db, "meta_test", world).has_value());

    // Verify meta key exists and contains correct counts
    REQUIRE(db.Exists("world:meta_test:meta"));

    // Load into a world with the same structure
    TestWorld world2;
    world2.AddAgent<TestAgent>("x");
    world2.AddAgent<TestAgent>("y");
    world2.AddItem("z");

    REQUIRE(LoadWorld(db, "meta_test", world2).has_value());

    // run_over is saved but not restored (WorldBase has no public setter).
    // Default false is correct when loading a game to continue playing.
    REQUIRE(world2.IsRunOver() == false);
}

TEST_CASE("WorldHelpers - LoadWorld rejects agent count mismatch", "[world-helpers]") {
    Database db;
    TestWorld world;
    world.AddAgent<TestAgent>("agent_a");
    world.AddAgent<TestAgent>("agent_b");
    REQUIRE(SaveWorld(db, "mismatch", world).has_value());

    // World with only 1 agent — should fail
    TestWorld world2;
    world2.AddAgent<TestAgent>("x");
    auto result = LoadWorld(db, "mismatch", world2);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == DatabaseError::InvalidData);
}

TEST_CASE("WorldHelpers - LoadWorld rejects item count mismatch", "[world-helpers]") {
    Database db;
    TestWorld world;
    world.AddItem("item_a");
    REQUIRE(SaveWorld(db, "item_mm", world).has_value());

    // World with 0 items — should fail
    TestWorld world2;
    auto result = LoadWorld(db, "item_mm", world2);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == DatabaseError::InvalidData);
}

TEST_CASE("WorldHelpers - grid round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    auto& grid = world.GetGrid();
    grid.AddCellType("floor", "walkable", '.');
    grid.AddCellType("wall", "blocked", '#');
    size_t floor_id = grid.GetCellTypeID("floor");
    size_t wall_id = grid.GetCellTypeID("wall");

    grid.Resize(5, 3);
    for (size_t x = 0; x < 5; ++x) grid[x, 0] = floor_id;
    for (size_t x = 0; x < 5; ++x) grid[x, 1] = (x % 2 == 0) ? wall_id : floor_id;
    for (size_t x = 0; x < 5; ++x) grid[x, 2] = wall_id;

    REQUIRE(SaveWorld(db, "grid_test", world).has_value());
    REQUIRE(db.Exists("world:grid_test:grid"));

    TestWorld world2;
    world2.GetGrid().AddCellType("floor", "walkable", '.');
    world2.GetGrid().AddCellType("wall", "blocked", '#');

    REQUIRE(LoadWorld(db, "grid_test", world2).has_value());

    auto& grid2 = world2.GetGrid();
    REQUIRE(grid2.GetWidth() == 5);
    REQUIRE(grid2.GetHeight() == 3);

    for (size_t y = 0; y < 3; ++y) {
        for (size_t x = 0; x < 5; ++x) {
            REQUIRE(grid2[x, y] == grid[x, y]);
        }
    }
}

TEST_CASE("WorldHelpers - agent round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    auto& a0 = world.AddAgent<TestAgent>("Alice");
    a0.SetLocation(WorldPosition(3.5, 7.2));
    a0.SetSymbol('A');

    auto& a1 = world.AddAgent<TestAgent>("Bob");
    a1.SetLocation(WorldPosition(10.0, 20.0));
    a1.SetSymbol('B');

    REQUIRE(SaveWorld(db, "agent_test", world).has_value());
    REQUIRE(db.Exists("world:agent_test:agent:0"));
    REQUIRE(db.Exists("world:agent_test:agent:1"));

    TestWorld world2;
    world2.AddAgent<TestAgent>("placeholder_0");
    world2.AddAgent<TestAgent>("placeholder_1");

    REQUIRE(LoadWorld(db, "agent_test", world2).has_value());

    REQUIRE(world2.GetAgent(0).GetName() == "Alice");
    REQUIRE(world2.GetAgent(0).GetLocation().IsPosition());
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().X() == Approx(3.5));
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().Y() == Approx(7.2));
    REQUIRE(world2.GetAgent(0).GetSymbol() == 'A');

    REQUIRE(world2.GetAgent(1).GetName() == "Bob");
    REQUIRE(world2.GetAgent(1).GetLocation().IsPosition());
    REQUIRE(world2.GetAgent(1).GetLocation().AsWorldPosition().X() == Approx(10.0));
    REQUIRE(world2.GetAgent(1).GetLocation().AsWorldPosition().Y() == Approx(20.0));
    REQUIRE(world2.GetAgent(1).GetSymbol() == 'B');
}

TEST_CASE("WorldHelpers - item round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    auto& i0 = world.AddItem("Sword");
    i0.SetLocation(WorldPosition(1.0, 2.0));

    auto& i1 = world.AddItem("Shield");
    i1.SetOwnerID(AgentID{42});

    REQUIRE(SaveWorld(db, "item_test", world).has_value());
    REQUIRE(db.Exists("world:item_test:item:0"));
    REQUIRE(db.Exists("world:item_test:item:1"));

    TestWorld world2;
    world2.AddItem("placeholder_0");
    world2.AddItem("placeholder_1");

    REQUIRE(LoadWorld(db, "item_test", world2).has_value());

    REQUIRE(world2.GetItem(0).GetName() == "Sword");
    REQUIRE(world2.GetItem(0).GetLocation().IsPosition());
    REQUIRE(world2.GetItem(0).GetLocation().AsWorldPosition().X() == Approx(1.0));
    REQUIRE(world2.GetItem(0).GetLocation().AsWorldPosition().Y() == Approx(2.0));

    REQUIRE(world2.GetItem(1).GetName() == "Shield");
    REQUIRE(world2.GetItem(1).GetLocation().IsAgentID());
    REQUIRE(world2.GetItem(1).GetLocation().AsAgentID() == 42);
}

TEST_CASE("WorldHelpers - empty world round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;
    // No agents, no items, default 0x0 grid

    REQUIRE(SaveWorld(db, "empty", world).has_value());

    TestWorld world2;
    REQUIRE(LoadWorld(db, "empty", world2).has_value());

    REQUIRE(world2.GetNumAgents() == 0);
    REQUIRE(world2.GetNumItems() == 0);
    REQUIRE(world2.GetGrid().GetWidth() == 0);
    REQUIRE(world2.GetGrid().GetHeight() == 0);
}

TEST_CASE("WorldHelpers - location variant round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    // Agent with WorldPosition
    auto& a0 = world.AddAgent<TestAgent>("pos_agent");
    a0.SetLocation(WorldPosition(99.5, 0.1));

    // Item with ItemID location
    auto& i0 = world.AddItem("inside_item");
    i0.SetLocation(ItemID{7});

    // Item with AgentID location
    auto& i1 = world.AddItem("held_item");
    i1.SetOwnerID(AgentID{3});

    // Item with WorldPosition
    auto& i2 = world.AddItem("ground_item");
    i2.SetLocation(WorldPosition(5.0, 10.0));

    REQUIRE(SaveWorld(db, "loc_test", world).has_value());

    TestWorld world2;
    world2.AddAgent<TestAgent>("x");
    world2.AddItem("x");
    world2.AddItem("x");
    world2.AddItem("x");

    REQUIRE(LoadWorld(db, "loc_test", world2).has_value());

    // Agent: WorldPosition
    REQUIRE(world2.GetAgent(0).GetLocation().IsPosition());
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().X() == Approx(99.5));
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().Y() == Approx(0.1));

    // Item 0: ItemID
    REQUIRE(world2.GetItem(0).GetLocation().IsItemID());
    REQUIRE(world2.GetItem(0).GetLocation().AsItemID() == 7);

    // Item 1: AgentID
    REQUIRE(world2.GetItem(1).GetLocation().IsAgentID());
    REQUIRE(world2.GetItem(1).GetLocation().AsAgentID() == 3);

    // Item 2: WorldPosition
    REQUIRE(world2.GetItem(2).GetLocation().IsPosition());
    REQUIRE(world2.GetItem(2).GetLocation().AsWorldPosition().X() == Approx(5.0));
    REQUIRE(world2.GetItem(2).GetLocation().AsWorldPosition().Y() == Approx(10.0));
}

TEST_CASE("WorldHelpers - full world round-trip", "[world-helpers]") {
    Database db;
    TestWorld world;

    // Set up grid
    auto& grid = world.GetGrid();
    grid.AddCellType("floor", "", '.');
    grid.AddCellType("wall", "", '#');
    grid.Resize(3, 3);
    size_t wall_id = grid.GetCellTypeID("wall");
    grid[0, 0] = wall_id; grid[1, 0] = wall_id; grid[2, 0] = wall_id;
    grid[0, 2] = wall_id; grid[1, 2] = wall_id; grid[2, 2] = wall_id;

    // Add agents
    auto& a0 = world.AddAgent<TestAgent>("Hero");
    a0.SetLocation(WorldPosition(1.0, 1.0));
    a0.SetSymbol('@');

    auto& a1 = world.AddAgent<TestAgent>("Companion");
    a1.SetLocation(WorldPosition(2.0, 1.0));
    a1.SetSymbol('C');

    // Add items
    auto& i0 = world.AddItem("Key");
    i0.SetLocation(WorldPosition(1.0, 1.0));

    auto& i1 = world.AddItem("Potion");
    i1.SetOwnerID(AgentID{0});

    REQUIRE(SaveWorld(db, "full", world).has_value());

    // Build matching world structure
    TestWorld world2;
    world2.GetGrid().AddCellType("floor", "", '.');
    world2.GetGrid().AddCellType("wall", "", '#');
    world2.AddAgent<TestAgent>("_");
    world2.AddAgent<TestAgent>("_");
    world2.AddItem("_");
    world2.AddItem("_");

    REQUIRE(LoadWorld(db, "full", world2).has_value());

    // Verify grid
    auto& g2 = world2.GetGrid();
    REQUIRE(g2.GetWidth() == 3);
    REQUIRE(g2.GetHeight() == 3);
    REQUIRE(g2[0, 0] == wall_id);
    REQUIRE(g2[1, 1] == 0);

    // Verify agents
    REQUIRE(world2.GetAgent(0).GetName() == "Hero");
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().X() == Approx(1.0));
    REQUIRE(world2.GetAgent(0).GetSymbol() == '@');

    REQUIRE(world2.GetAgent(1).GetName() == "Companion");
    REQUIRE(world2.GetAgent(1).GetLocation().AsWorldPosition().X() == Approx(2.0));
    REQUIRE(world2.GetAgent(1).GetSymbol() == 'C');

    // Verify items
    REQUIRE(world2.GetItem(0).GetName() == "Key");
    REQUIRE(world2.GetItem(0).GetLocation().IsPosition());

    REQUIRE(world2.GetItem(1).GetName() == "Potion");
    REQUIRE(world2.GetItem(1).GetLocation().IsAgentID());
    REQUIRE(world2.GetItem(1).GetLocation().AsAgentID() == 0);
}

TEST_CASE("WorldHelpers - SaveWorld/SaveGame/LoadGame/LoadWorld round-trip", "[world-helpers]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "wh_savegame_test";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    // -- Server side --
    Database server_db;
    WebSocketServer ws_server(WH_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    // -- Client side --
    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(WH_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return ws_server.ClientCount() > 0;
    }));

    for (int i = 0; i < 5; ++i) {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // -- Build a world and save it into client_db --
    TestWorld world;
    world.GetGrid().AddCellType("floor", "", '.');
    world.GetGrid().Resize(2, 2);
    auto& agent = world.AddAgent<TestAgent>("Hero");
    agent.SetLocation(WorldPosition(1.0, 1.0));
    agent.SetSymbol('@');
    auto& item = world.AddItem("Gem");
    item.SetLocation(WorldPosition(0.0, 0.0));

    REQUIRE(SaveWorld(client_db, "nettest", world).has_value());
    (void)client_db.FlushDirty();

    // -- Send to server via SaveGame --
    REQUIRE(client_sync.SaveGame("nettest_save").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "nettest_save.save");
    }));

    // -- Clear client DB and load back --
    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    REQUIRE(client_sync.LoadGame("nettest_save").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() > 0;
    }));

    // -- Reconstruct world from loaded DB --
    TestWorld world2;
    world2.GetGrid().AddCellType("floor", "", '.');
    world2.AddAgent<TestAgent>("_");
    world2.AddItem("_");

    REQUIRE(LoadWorld(client_db, "nettest", world2).has_value());

    REQUIRE(world2.GetGrid().GetWidth() == 2);
    REQUIRE(world2.GetGrid().GetHeight() == 2);
    REQUIRE(world2.GetAgent(0).GetName() == "Hero");
    REQUIRE(world2.GetAgent(0).GetLocation().AsWorldPosition().X() == Approx(1.0));
    REQUIRE(world2.GetAgent(0).GetSymbol() == '@');
    REQUIRE(world2.GetItem(0).GetName() == "Gem");
    REQUIRE(world2.GetItem(0).GetLocation().AsWorldPosition().X() == Approx(0.0));

    server_sync.Stop();
    client_sync.Stop();
    (void)ws_client.Disconnect();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}
