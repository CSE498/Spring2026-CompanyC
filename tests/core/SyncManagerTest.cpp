/**
 * @file SyncManager.cpp
 * @brief Test suite for the SyncManager class
 * @author Group 9
 */

#include "catch2/catch.hpp"
#include "../tools/TestHelpers.hpp"
#include "../../source/core/SyncManager.hpp"
#include "../../source/core/Database.hpp"
#include "../../source/tools/WebSocketConnection.hpp"
#include "../../source/tools/WebSocketServer.hpp"
#include "../../source/tools/Serializer.hpp"

#include <filesystem>

using namespace cse498;
using cse498::test::WaitFor;

namespace {

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

TEST_CASE("SyncManager - Round-trip encode/decode for save protocol types", "[sync]") {
    std::vector<uint8_t> payload = {0x01, 0x02};

    auto types = {
        SyncMessageType::SAVE,
        SyncMessageType::LOAD,
        SyncMessageType::LIST_SAVES,
        SyncMessageType::SAVE_LIST
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
// Save Protocol Payload Helpers
// ============================================================================

TEST_CASE("SyncManager - LOAD payload round-trip", "[sync]") {
    auto encoded = SyncManager::EncodeLoadPayload("world_1");
    REQUIRE_FALSE(encoded.empty());

    auto decoded = SyncManager::DecodeLoadPayload(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(*decoded == "world_1");
}

TEST_CASE("SyncManager - SAVE_LIST payload round-trip", "[sync]") {
    std::vector<SaveInfo> saves = {
        {"world_alpha", 1711800000},
        {"world_beta",  1711900000},
        {"world_gamma", 1712000000}
    };

    auto encoded = SyncManager::EncodeSaveListPayload(saves);
    REQUIRE_FALSE(encoded.empty());

    auto decoded = SyncManager::DecodeSaveListPayload(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->size() == 3);
    REQUIRE((*decoded)[0].name == "world_alpha");
    REQUIRE((*decoded)[0].timestamp == 1711800000);
    REQUIRE((*decoded)[1].name == "world_beta");
    REQUIRE((*decoded)[1].timestamp == 1711900000);
    REQUIRE((*decoded)[2].name == "world_gamma");
    REQUIRE((*decoded)[2].timestamp == 1712000000);
}

TEST_CASE("SyncManager - SAVE_LIST empty list round-trip", "[sync]") {
    std::vector<SaveInfo> empty_saves;

    auto encoded = SyncManager::EncodeSaveListPayload(empty_saves);
    REQUIRE_FALSE(encoded.empty());

    auto decoded = SyncManager::DecodeSaveListPayload(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->empty());
}

TEST_CASE("SyncManager - SAVE payload round-trip", "[sync]") {
    Database db;
    REQUIRE(db.Store("player:1:health", 100).has_value());
    REQUIRE(db.Store("player:1:name", std::string("Alice")).has_value());
    (void)db.FlushDirty();

    auto encoded = SyncManager::EncodeSavePayload("my_save", db);
    REQUIRE_FALSE(encoded.empty());

    auto decoded = SyncManager::DecodeSavePayload(encoded);
    REQUIRE(decoded.has_value());

    auto& [name, state_bytes] = *decoded;
    REQUIRE(name == "my_save");
    REQUIRE_FALSE(state_bytes.empty());

    // Verify state bytes contain the right number of entries
    Serializer s;
    std::string state_data(state_bytes.begin(), state_bytes.end());
    size_t pos = 0;
    auto count = s.DeserializeAt<int>(state_data, pos);
    REQUIRE(count.has_value());
    REQUIRE(*count == 2);
}

TEST_CASE("SyncManager - Long save name round-trips", "[sync]") {
    std::string long_name(1500, 'x');

    SECTION("LOAD payload") {
        auto encoded = SyncManager::EncodeLoadPayload(long_name);
        auto decoded = SyncManager::DecodeLoadPayload(encoded);
        REQUIRE(decoded.has_value());
        REQUIRE(*decoded == long_name);
    }

    SECTION("SAVE payload") {
        Database db;
        REQUIRE(db.Store("key", 1).has_value());
        (void)db.FlushDirty();

        auto encoded = SyncManager::EncodeSavePayload(long_name, db);
        auto decoded = SyncManager::DecodeSavePayload(encoded);
        REQUIRE(decoded.has_value());
        REQUIRE(decoded->first == long_name);
    }
}

TEST_CASE("SyncManager - Decode payload rejects empty input", "[sync]") {
    std::vector<uint8_t> empty;

    SECTION("DecodeSavePayload") {
        auto result = SyncManager::DecodeSavePayload(empty);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::DecodeFailed);
    }

    SECTION("DecodeLoadPayload") {
        auto result = SyncManager::DecodeLoadPayload(empty);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::DecodeFailed);
    }

    SECTION("DecodeSaveListPayload") {
        auto result = SyncManager::DecodeSaveListPayload(empty);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::DecodeFailed);
    }
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

// ============================================================================
// Server Mode — Save File I/O (Phase 4c)
// ============================================================================

TEST_CASE("SyncManager - List empty saves directory", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_empty_list";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    // Raw client — no client_sync (SAVE_LIST dispatch is Phase 4d)
    WebSocketConnection ws_client;

    std::vector<SaveInfo> received_list;
    bool got_list = false;
    ws_client.OnMessage([&](const std::vector<uint8_t>& data) {
        auto decoded = SyncManager::DecodeMessage(data);
        if (decoded.has_value() && decoded->first == SyncMessageType::SAVE_LIST) {
            auto list = SyncManager::DecodeSaveListPayload(decoded->second);
            if (list.has_value()) {
                received_list = std::move(*list);
                got_list = true;
            }
        }
    });

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return ws_server.ClientCount() > 0;
    }));

    auto frame = SyncManager::EncodeMessage(SyncMessageType::LIST_SAVES, {});
    REQUIRE(frame.has_value());
    REQUIRE(ws_client.Send(*frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return got_list;
    }));

    REQUIRE(received_list.empty());

    server_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - Save then list", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_save_then_list";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    REQUIRE(client_db.Store("player:1:health", 100).has_value());
    REQUIRE(client_db.Store("player:1:name", std::string("Alice")).has_value());
    (void)client_db.FlushDirty();

    auto save_payload = SyncManager::EncodeSavePayload("my_world", client_db);
    auto save_frame = SyncManager::EncodeMessage(SyncMessageType::SAVE, save_payload);
    REQUIRE(save_frame.has_value());

    WebSocketConnection ws_client;

    std::vector<SaveInfo> received_list;
    bool got_list = false;
    ws_client.OnMessage([&](const std::vector<uint8_t>& data) {
        auto decoded = SyncManager::DecodeMessage(data);
        if (decoded.has_value() && decoded->first == SyncMessageType::SAVE_LIST) {
            auto list = SyncManager::DecodeSaveListPayload(decoded->second);
            if (list.has_value()) {
                received_list = std::move(*list);
                got_list = true;
            }
        }
    });

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return ws_server.ClientCount() > 0;
    }));

    REQUIRE(ws_client.Send(*save_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return std::filesystem::exists(tmp_dir / "my_world.save");
    }));

    auto list_frame = SyncManager::EncodeMessage(SyncMessageType::LIST_SAVES, {});
    REQUIRE(list_frame.has_value());
    REQUIRE(ws_client.Send(*list_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return got_list;
    }));

    REQUIRE(received_list.size() == 1);
    REQUIRE(received_list[0].name == "my_world");
    auto now = static_cast<uint64_t>(std::time(nullptr));
    REQUIRE(received_list[0].timestamp >= now - 60);
    REQUIRE(received_list[0].timestamp <= now + 60);

    server_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - Save then load", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_save_then_load";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    REQUIRE(client_db.Store("player:1:health", 100).has_value());
    REQUIRE(client_db.Store("player:1:name", std::string("Alice")).has_value());
    (void)client_db.FlushDirty();

    auto save_payload = SyncManager::EncodeSavePayload("load_test", client_db);
    auto save_frame = SyncManager::EncodeMessage(SyncMessageType::SAVE, save_payload);
    REQUIRE(save_frame.has_value());
    REQUIRE(ws_client.Send(*save_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "load_test.save");
    }));

    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    auto load_payload = SyncManager::EncodeLoadPayload("load_test");
    auto load_frame = SyncManager::EncodeMessage(SyncMessageType::LOAD, load_payload);
    REQUIRE(load_frame.has_value());
    REQUIRE(ws_client.Send(*load_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() == 2;
    }));

    auto health = client_db.Load<int>("player:1:health");
    REQUIRE(health.has_value());
    REQUIRE(*health == 100);

    auto name = client_db.Load<std::string>("player:1:name");
    REQUIRE(name.has_value());
    REQUIRE(*name == "Alice");

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - Overwrite existing save", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_overwrite";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    REQUIRE(client_db.Store("score", 100).has_value());
    (void)client_db.FlushDirty();

    auto save1_payload = SyncManager::EncodeSavePayload("world_a", client_db);
    auto save1_frame = SyncManager::EncodeMessage(SyncMessageType::SAVE, save1_payload);
    REQUIRE(save1_frame.has_value());
    REQUIRE(ws_client.Send(*save1_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "world_a.save");
    }));

    REQUIRE(client_db.Update("score", 999).has_value());
    (void)client_db.FlushDirty();

    auto save2_payload = SyncManager::EncodeSavePayload("world_a", client_db);
    auto save2_frame = SyncManager::EncodeMessage(SyncMessageType::SAVE, save2_payload);
    REQUIRE(save2_frame.has_value());
    REQUIRE(ws_client.Send(*save2_frame).has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int i = 0; i < 20; ++i) {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    auto load_payload = SyncManager::EncodeLoadPayload("world_a");
    auto load_frame = SyncManager::EncodeMessage(SyncMessageType::LOAD, load_payload);
    REQUIRE(load_frame.has_value());
    REQUIRE(ws_client.Send(*load_frame).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() > 0;
    }));

    auto val = client_db.Load<int>("score");
    REQUIRE(val.has_value());
    REQUIRE(*val == 999);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - Invalid save name rejected", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_invalid_name";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    REQUIRE(client_db.Store("key", 1).has_value());
    (void)client_db.FlushDirty();

    auto evil_payload = SyncManager::EncodeSavePayload("../evil", client_db);
    auto evil_frame = SyncManager::EncodeMessage(SyncMessageType::SAVE, evil_payload);
    REQUIRE(evil_frame.has_value());

    WebSocketConnection ws_client;
    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        return ws_server.ClientCount() > 0;
    }));

    REQUIRE(ws_client.Send(*evil_frame).has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int i = 0; i < 20; ++i) {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    bool dir_empty = true;
    for (const auto& entry : std::filesystem::directory_iterator(tmp_dir)) {
        (void)entry;
        dir_empty = false;
    }
    REQUIRE(dir_empty);

    REQUIRE_FALSE(std::filesystem::exists(tmp_dir.parent_path() / "evil.save"));

    server_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

// ============================================================================
// Client Mode — Save API Error Cases (Phase 4d)
// ============================================================================

TEST_CASE("SyncManager - SaveGame/LoadGame reject invalid names", "[sync]") {
    Database db;
    WebSocketConnection ws_client;
    SyncManager sync(db, ws_client);

    // Not started — all three should return NotStarted
    SECTION("SaveGame while not started") {
        auto result = sync.SaveGame("valid_name");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::NotStarted);
    }

    SECTION("LoadGame while not started") {
        auto result = sync.LoadGame("valid_name");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::NotStarted);
    }

    SECTION("RequestSaveList while not started") {
        auto result = sync.RequestSaveList();
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::NotStarted);
    }
}

TEST_CASE("SyncManager - Client-side name validation", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_name_validation";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());
    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    REQUIRE(client_db.Store("key", 1).has_value());
    (void)client_db.FlushDirty();

    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return ws_server.ClientCount() > 0;
    }));

    SECTION("SaveGame with path traversal") {
        auto result = client_sync.SaveGame("../evil");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::EncodeFailed);
    }

    SECTION("SaveGame with empty name") {
        auto result = client_sync.SaveGame("");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::EncodeFailed);
    }

    SECTION("LoadGame with spaces") {
        auto result = client_sync.LoadGame("bad name");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::EncodeFailed);
    }

    SECTION("LoadGame with empty name") {
        auto result = client_sync.LoadGame("");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == SyncError::EncodeFailed);
    }

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

// ============================================================================
// Client Mode — Save API Integration (Phase 4d)
// ============================================================================

TEST_CASE("SyncManager - SaveGame writes file to server", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_savegame";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    // Store data and save
    REQUIRE(client_db.Store("player:1:health", 100).has_value());
    REQUIRE(client_db.Store("player:1:name", std::string("Alice")).has_value());
    (void)client_db.FlushDirty();

    auto result = client_sync.SaveGame("test_save");
    REQUIRE(result.has_value());

    // Poll until server writes the file
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "test_save.save");
    }));

    REQUIRE(std::filesystem::file_size(tmp_dir / "test_save.save") > 0);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - RequestSaveList fires callback", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_requestlist";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);

    std::vector<SaveInfo> received_list;
    bool got_list = false;
    client_sync.OnSaveListReceived([&](const std::vector<SaveInfo>& list) {
        received_list = list;
        got_list = true;
    });

    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    // Save a file first via SaveGame
    REQUIRE(client_db.Store("data", 42).has_value());
    (void)client_db.FlushDirty();
    REQUIRE(client_sync.SaveGame("world_one").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "world_one.save");
    }));

    // Request save list
    REQUIRE(client_sync.RequestSaveList().has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return got_list;
    }));

    REQUIRE(received_list.size() == 1);
    REQUIRE(received_list[0].name == "world_one");
    auto now = static_cast<uint64_t>(std::time(nullptr));
    REQUIRE(received_list[0].timestamp >= now - 60);
    REQUIRE(received_list[0].timestamp <= now + 60);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - LoadGame populates Database", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_loadgame";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    // Store data and save to server
    REQUIRE(client_db.Store("player:1:health", 100).has_value());
    REQUIRE(client_db.Store("player:1:name", std::string("Alice")).has_value());
    (void)client_db.FlushDirty();

    REQUIRE(client_sync.SaveGame("load_test_4d").has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "load_test_4d.save");
    }));

    // Clear client DB
    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    // Load from server
    REQUIRE(client_sync.LoadGame("load_test_4d").has_value());

    // Poll until client DB repopulates
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() == 2;
    }));

    auto health = client_db.Load<int>("player:1:health");
    REQUIRE(health.has_value());
    REQUIRE(*health == 100);

    auto name = client_db.Load<std::string>("player:1:name");
    REQUIRE(name.has_value());
    REQUIRE(*name == "Alice");

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}




TEST_CASE("SyncManager - Full round-trip: save, list, load", "[sync]") {
    auto tmp_dir = std::filesystem::temp_directory_path() / "sync_test_full_roundtrip";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directories(tmp_dir);

    // start server
    Database server_db;
    WebSocketServer ws_server(SYNC_TEST_PORT);
    REQUIRE(ws_server.Start().has_value());

    SyncManager server_sync(server_db, ws_server, tmp_dir.string());
    REQUIRE(server_sync.Start().has_value());

    // connect client side
    Database client_db;
    WebSocketConnection ws_client;
    SyncManager client_sync(client_db, ws_client);

    std::vector<SaveInfo> received_list;
    bool got_list = false;
    client_sync.OnSaveListReceived([&](const std::vector<SaveInfo>& list) {
        received_list = list;
        got_list = true;
    });

    REQUIRE(client_sync.Start().has_value());

    REQUIRE(ws_client.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());

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

    // store test data in client db
    REQUIRE(client_db.Store("player:1:health", 100).has_value());
    REQUIRE(client_db.Store("player:1:name", std::string("Hero")).has_value());
    REQUIRE(client_db.Store("world:level", 5).has_value());
    (void)client_db.FlushDirty();

    // Client saves game
    REQUIRE(client_sync.SaveGame("test_world").has_value());

    // wait for server 
    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return std::filesystem::exists(tmp_dir / "test_world.save");
    }));

    // Client clears local Database
    client_db.Clear();
    REQUIRE(client_db.Size() == 0);

    // Client requests save list, verify "test_world" appears
    REQUIRE(client_sync.RequestSaveList().has_value());

    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return got_list;
    }));

    REQUIRE(received_list.size() == 1);
    REQUIRE(received_list[0].name == "test_world");
    auto now = static_cast<uint64_t>(std::time(nullptr));
    REQUIRE(received_list[0].timestamp >= now - 60);
    REQUIRE(received_list[0].timestamp <= now + 60);

    // Client loads game
    REQUIRE(client_sync.LoadGame("test_world").has_value());


    REQUIRE(WaitFor([&]() {
        ws_server.Poll(); server_sync.Poll();
        ws_client.Poll(); client_sync.Poll();
        return client_db.Size() == 3;
    }));

    auto health = client_db.Load<int>("player:1:health");
    REQUIRE(health.has_value());
    REQUIRE(*health == 100);

    auto player_name = client_db.Load<std::string>("player:1:name");
    REQUIRE(player_name.has_value());
    REQUIRE(*player_name == "Hero");

    auto level = client_db.Load<int>("world:level");
    REQUIRE(level.has_value());
    REQUIRE(*level == 5);

    server_sync.Stop();
    client_sync.Stop();
    ws_server.Stop();
    std::filesystem::remove_all(tmp_dir);
}

TEST_CASE("SyncManager - OnLoadComplete fires after FULL_STATE", "[sync]") {
    WebSocketServer server(SYNC_TEST_PORT);
    REQUIRE(server.Start().has_value());

    Database server_db;
    SyncManager server_sync(server_db, server, "./test_onload_saves/");

    Database client_db;
    WebSocketConnection client_ws;

    SyncManager client_sync(client_db, client_ws);

    bool load_complete = false;
    client_sync.OnLoadComplete([&]() { load_complete = true; });

    REQUIRE(client_ws.Connect("ws://127.0.0.1:" + std::to_string(SYNC_TEST_PORT)).has_value());
    REQUIRE(server_sync.Start().has_value());
    REQUIRE(client_sync.Start().has_value());

    REQUIRE(WaitFor([&] {
        server.Poll();
        client_ws.Poll();
        return server.ClientCount() > 0;
    }));

    // Drain initial FULL_STATE from connection
    for (int i = 0; i < 5; ++i) {
        server.Poll(); server_sync.Poll();
        client_ws.Poll(); client_sync.Poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    (void)client_db.Store("key1", std::string("value1"));
    (void)client_db.FlushDirty();
    REQUIRE(client_sync.SaveGame("onload_test").has_value());

    REQUIRE(WaitFor([&] {
        server.Poll(); server_sync.Poll();
        client_ws.Poll(); client_sync.Poll();
        return std::filesystem::exists("./test_onload_saves/onload_test.save");
    }));

    client_db.Clear();
    load_complete = false;
    REQUIRE(client_sync.LoadGame("onload_test").has_value());

    REQUIRE(WaitFor([&] {
        server.Poll(); server_sync.Poll();
        client_ws.Poll(); client_sync.Poll();
        return load_complete;
    }));

    auto val = client_db.Load<std::string>("key1");
    REQUIRE(val.has_value());
    REQUIRE(*val == "value1");

    client_sync.Stop();
    server_sync.Stop();
    server.Stop();
    std::filesystem::remove_all("./test_onload_saves/");
}
