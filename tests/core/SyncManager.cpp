/**
 * @file SyncManager.cpp
 * @brief Test suite for the SyncManager class
 * @author Group 9
 */

#include "catch2/catch.hpp"
#include "../../source/core/SyncManager.hpp"
#include "../../source/core/Database.hpp"
#include "../../source/tools/WebSocketConnection.hpp"
#include "../../source/tools/WebSocketServer.hpp"

#include <thread>
#include <chrono>

using namespace cse498;

namespace {

template <typename Pred>
bool WaitFor(Pred pred, int timeout_ms = 2000, int poll_interval_ms = 10) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (!pred()) {
        if (std::chrono::steady_clock::now() > deadline) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }
    return true;
}

constexpr int SYNC_TEST_PORT = 18090;

} // namespace

// ============================================================================
// EncodeMessage / DecodeMessage
// ============================================================================

TEST_CASE("SyncManager - EncodeMessage produces 9-byte header + payload", "[sync]") {
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC};
    auto frame = SyncManager::EncodeMessage(SyncMessageType::FULL_STATE, payload);
    REQUIRE(frame.has_value());
    REQUIRE(frame->size() == 9 + 3);

    // First byte is message type
    REQUIRE((*frame)[0] == static_cast<uint8_t>(SyncMessageType::FULL_STATE));

    // 8 bytes big-endian length = 3
    REQUIRE((*frame)[1] == 0x00);
    REQUIRE((*frame)[2] == 0x00);
    REQUIRE((*frame)[3] == 0x00);
    REQUIRE((*frame)[4] == 0x00);
    REQUIRE((*frame)[5] == 0x00);
    REQUIRE((*frame)[6] == 0x00);
    REQUIRE((*frame)[7] == 0x00);
    REQUIRE((*frame)[8] == 0x03);

    // Payload
    REQUIRE((*frame)[9] == 0xAA);
    REQUIRE((*frame)[10] == 0xBB);
    REQUIRE((*frame)[11] == 0xCC);
}

TEST_CASE("SyncManager - Round-trip encode/decode for each message type", "[sync]") {
    std::vector<uint8_t> payload = {0x01, 0x02};

    auto types = {
        SyncMessageType::FULL_STATE,
        SyncMessageType::DELTA,
        SyncMessageType::UPDATE,
        SyncMessageType::SYNC_REQUEST,
        SyncMessageType::SYNC_RESPONSE
    };

    for (auto type : types) {
        auto frame = SyncManager::EncodeMessage(type, payload);
        REQUIRE(frame.has_value());

        auto decoded = SyncManager::DecodeMessage(*frame);
        REQUIRE(decoded.has_value());
        REQUIRE(decoded->first == type);
        REQUIRE(decoded->second == payload);
    }
}

TEST_CASE("SyncManager - Empty payload round-trips (SYNC_REQUEST)", "[sync]") {
    std::vector<uint8_t> empty_payload;
    auto frame = SyncManager::EncodeMessage(SyncMessageType::SYNC_REQUEST, empty_payload);
    REQUIRE(frame.has_value());
    REQUIRE(frame->size() == 9);

    auto decoded = SyncManager::DecodeMessage(*frame);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->first == SyncMessageType::SYNC_REQUEST);
    REQUIRE(decoded->second.empty());
}

TEST_CASE("SyncManager - DecodeMessage rejects truncated frame", "[sync]") {
    SECTION("Less than 9 bytes") {
        std::vector<uint8_t> short_frame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        auto result = SyncManager::DecodeMessage(short_frame);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::DecodeFailed);
    }

    SECTION("Header says 10 bytes but only 3 present") {
        std::vector<uint8_t> bad_frame = {
            0x00,                                           // FULL_STATE
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, // length = 10
            0x01, 0x02, 0x03                                 // only 3 bytes
        };
        auto result = SyncManager::DecodeMessage(bad_frame);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::DecodeFailed);
    }
}

TEST_CASE("SyncManager - DecodeMessage rejects unknown message type", "[sync]") {
    std::vector<uint8_t> bad_frame = {
        0xFF,                                           // invalid type
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // length = 0
    };
    auto result = SyncManager::DecodeMessage(bad_frame);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SyncError::InvalidMessage);
}

TEST_CASE("SyncManager - Large payload round-trips", "[sync]") {
    // 1000-byte payload to test multi-byte length encoding
    std::vector<uint8_t> payload(1000, 0x42);
    auto frame = SyncManager::EncodeMessage(SyncMessageType::DELTA, payload);
    REQUIRE(frame.has_value());

    auto decoded = SyncManager::DecodeMessage(*frame);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->first == SyncMessageType::DELTA);
    REQUIRE(decoded->second == payload);
}

// ============================================================================
// Server Mode — Construction & State
// ============================================================================

TEST_CASE("SyncManager - Server construction and state", "[sync]") {
    Database db;
    WebSocketServer ws_server(SYNC_TEST_PORT);

    SyncManager sync(db, ws_server);
    REQUIRE(sync.IsServer());
    REQUIRE_FALSE(sync.IsRunning());
}

TEST_CASE("SyncManager - Server Start and Stop", "[sync]") {
    Database db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager sync(db, ws_server);
    REQUIRE(sync.Start().has_value());
    REQUIRE(sync.IsRunning());

    sync.Stop();
    REQUIRE_FALSE(sync.IsRunning());

    ws_server.Stop();
}

TEST_CASE("SyncManager - Server Start twice returns AlreadyStarted", "[sync]") {
    Database db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager sync(db, ws_server);
    REQUIRE(sync.Start().has_value());

    auto result = sync.Start();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SyncError::AlreadyStarted);

    sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Server Mode — FULL_STATE on connect
// ============================================================================

TEST_CASE("SyncManager - Server sends FULL_STATE to new client", "[sync]") {
    Database server_db;
    REQUIRE(server_db.Store("player:1:health", 100).has_value());
    REQUIRE(server_db.Store("player:1:name", std::string("Alice")).has_value());
    (void)server_db.FlushDirty();

    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    bool connected = false;
    ws_client.OnConnect([&connected]() { connected = true; });
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return client_db.Size() == 2;
    }));

    auto health = client_db.Load<int>("player:1:health");
    REQUIRE(health.has_value());
    REQUIRE(*health == 100);

    auto name = client_db.Load<std::string>("player:1:name");
    REQUIRE(name.has_value());
    REQUIRE(*name == "Alice");

    auto dirty = client_db.FlushDirty();
    REQUIRE(dirty.empty());

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Server Mode — DELTA broadcast
// ============================================================================

TEST_CASE("SyncManager - Server broadcasts DELTA on dirty keys", "[sync]") {
    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    bool connected = false;
    ws_client.OnConnect([&connected]() { connected = true; });
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return connected;
    }));

    REQUIRE(server_db.Store("score", 42).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return client_db.Exists("score");
    }));

    auto val = client_db.Load<int>("score");
    REQUIRE(val.has_value());
    REQUIRE(*val == 42);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}

TEST_CASE("SyncManager - Server broadcasts Delete in DELTA", "[sync]") {
    Database server_db;
    REQUIRE(server_db.Store("temp_key", 99).has_value());
    (void)server_db.FlushDirty();

    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return client_db.Exists("temp_key");
    }));

    REQUIRE(server_db.Delete("temp_key"));

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return !client_db.Exists("temp_key");
    }));

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Client Mode — Construction & State
// ============================================================================

TEST_CASE("SyncManager - Client construction and state", "[sync]") {
    Database db;
    WebSocketConnection ws_client;

    SyncManager sync(db, ws_client);
    REQUIRE_FALSE(sync.IsServer());
    REQUIRE_FALSE(sync.IsRunning());
}

TEST_CASE("SyncManager - SendUpdate while not started returns NotStarted", "[sync]") {
    Database db;
    WebSocketConnection ws_client;
    SyncManager sync(db, ws_client);

    auto result = sync.SendUpdate("key");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SyncError::NotStarted);
}

// ============================================================================
// Client Mode — SendUpdate
// ============================================================================

TEST_CASE("SyncManager - Client SendUpdate reaches server", "[sync]") {
    // Server side
    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    // Client side — pre-populate a key that will be sent as UPDATE
    Database client_db;
    REQUIRE(client_db.Store("player:2:score", 500).has_value());
    (void)client_db.FlushDirty();

    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    // Connect and wait for FULL_STATE
    bool connected = false;
    ws_client.OnConnect([&connected]() { connected = true; });
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return connected;
    }));

    // Client sends UPDATE
    // First re-store the key so it exists after FULL_STATE cleared client db
    REQUIRE(client_db.Store("player:2:score", 500).has_value());
    (void)client_db.FlushDirty();

    auto send_result = client_sync.SendUpdate("player:2:score");
    REQUIRE(send_result.has_value());

    // Poll until server receives it
    REQUIRE(WaitFor([&]() {
        ws_server.Poll();
        server_sync.Poll();
        ws_client.Poll();
        client_sync.Poll();
        return server_db.Exists("player:2:score");
    }));

    auto val = server_db.Load<int>("player:2:score");
    REQUIRE(val.has_value());
    REQUIRE(*val == 500);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Integration — Two clients see each other's updates via server
// ============================================================================

TEST_CASE("SyncManager - DELTA broadcasts to multiple clients", "[sync]") {
    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    // Client 1
    Database client1_db;
    WebSocketConnection ws_client1;
    SyncManager client1_sync(client1_db, ws_client1);
    REQUIRE(client1_sync.Start().has_value());
    bool c1_connected = false;
    ws_client1.OnConnect([&c1_connected]() { c1_connected = true; });
    REQUIRE(ws_client1.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    // Client 2
    Database client2_db;
    WebSocketConnection ws_client2;
    SyncManager client2_sync(client2_db, ws_client2);
    REQUIRE(client2_sync.Start().has_value());
    bool c2_connected = false;
    ws_client2.OnConnect([&c2_connected]() { c2_connected = true; });
    REQUIRE(ws_client2.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    // Wait for both clients to connect
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client1.Poll(); client1_sync.Poll();
        ws_client2.Poll(); client2_sync.Poll();
        return c1_connected && c2_connected;
    }));

    // Server stores a new key
    REQUIRE(server_db.Store("world:tick", 100).has_value());

    // Both clients should receive it
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client1.Poll(); client1_sync.Poll();
        ws_client2.Poll(); client2_sync.Poll();
        return client1_db.Exists("world:tick") && client2_db.Exists("world:tick");
    }));

    REQUIRE(*client1_db.Load<int>("world:tick") == 100);
    REQUIRE(*client2_db.Load<int>("world:tick") == 100);

    server_sync.Stop();
    client1_sync.Stop();
    client2_sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Client Mode — SYNC_REQUEST
// ============================================================================

TEST_CASE("SyncManager - Client SYNC_REQUEST triggers SYNC_RESPONSE", "[sync]") {
    // Server with data
    Database server_db;
    REQUIRE(server_db.Store("data:a", 10).has_value());
    REQUIRE(server_db.Store("data:b", 20).has_value());
    (void)server_db.FlushDirty();

    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    // Client connects, receives FULL_STATE
    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() == 2;
    }));

    // Client clears its db and sends SYNC_REQUEST manually
    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    // Send a SYNC_REQUEST frame
    auto frame = SyncManager::EncodeMessage(SyncMessageType::SYNC_REQUEST, {});
    REQUIRE(frame.has_value());
    REQUIRE(ws_client.Send(*frame).has_value());

    // Poll until client gets re-populated via SYNC_RESPONSE
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() == 2;
    }));

    REQUIRE(*client_db.Load<int>("data:a") == 10);
    REQUIRE(*client_db.Load<int>("data:b") == 20);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}

// ============================================================================
// Vertical Slice — the Phase 3 proof
// ============================================================================

TEST_CASE("SyncManager - Vertical slice: sync counter between server and client", "[sync]") {
    // 1. Server stores initial value
    Database server_db;
    REQUIRE(server_db.Store("test:counter", 42).has_value());
    (void)server_db.FlushDirty();

    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server);
    REQUIRE(server_sync.Start().has_value());

    // 2. Client connects
    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    // 3. Client receives FULL_STATE with counter = 42
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Exists("test:counter");
    }));

    auto val1 = client_db.Load<int>("test:counter");
    REQUIRE(val1.has_value());
    REQUIRE(*val1 == 42);

    // 4. Server updates counter to 43
    REQUIRE(server_db.Update("test:counter", 43).has_value());

    // 5. Client sees 43
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        auto v = client_db.Load<int>("test:counter");
        return v.has_value() && *v == 43;
    }));

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
}
