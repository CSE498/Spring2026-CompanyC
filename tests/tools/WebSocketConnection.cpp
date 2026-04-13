/**
 * @file WebSocketConnection.cpp
 * @brief Test suite for the WebSocketConnection and WebSocketServer classes
 * @author Group 9
 *
 * 
 */

#include "catch2/catch.hpp"
#include "../../source/tools/WebSocketConnection.hpp"
#include "../../source/tools/WebSocketServer.hpp"

#include <thread>
#include <chrono>
#include <atomic>

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

constexpr int TEST_PORT = 18080;

} // namespace

// ============================================================================
// Error Enum
// ============================================================================

TEST_CASE("WebSocket - Error enum values exist", "[websocket]") {
    auto a = WebSocketError::ConnectionFailed;
    auto b = WebSocketError::ConnectionClosed;
    auto c = WebSocketError::SendFailed;
    auto d = WebSocketError::ServerStartFailed;
    auto e = WebSocketError::InvalidState;
    REQUIRE(a != b);
    REQUIRE(b != c);
    REQUIRE(c != d);
    REQUIRE(d != e);
}

// ============================================================================
// WebSocketConnection -- Construction & State
// ============================================================================

TEST_CASE("WebSocket - Client default construction", "[websocket]") {
    WebSocketConnection client;
    REQUIRE_FALSE(client.IsConnected());
}

TEST_CASE("WebSocket - Client move construction", "[websocket]") {
    WebSocketConnection a;
    WebSocketConnection b(std::move(a));
    REQUIRE_FALSE(b.IsConnected());
}

TEST_CASE("WebSocket - Client move assignment", "[websocket]") {
    WebSocketConnection a;
    WebSocketConnection b;
    b = std::move(a);
    REQUIRE_FALSE(b.IsConnected());
}

// ============================================================================
// WebSocketServer -- Lifecycle
// ============================================================================

TEST_CASE("WebSocket - Server lifecycle", "[websocket]") {
    SECTION("Start and stop") {
        WebSocketServer server(TEST_PORT);
        REQUIRE_FALSE(server.IsRunning());
        REQUIRE(server.Start().has_value());
        REQUIRE(server.IsRunning());
        server.Stop();
        REQUIRE_FALSE(server.IsRunning());
    }

    SECTION("Destructor stops server") {
        {
            WebSocketServer server(TEST_PORT);
            REQUIRE(server.Start().has_value());
            REQUIRE(server.IsRunning());
        }
        WebSocketServer server2(TEST_PORT);
        REQUIRE(server2.Start().has_value());
        server2.Stop();
    }

    SECTION("Move construction") {
        WebSocketServer a(TEST_PORT);
        REQUIRE(a.Start().has_value());
        WebSocketServer b(std::move(a));
        REQUIRE(b.IsRunning());
        b.Stop();
    }

    SECTION("Move assignment") {
        WebSocketServer a(TEST_PORT);
        REQUIRE(a.Start().has_value());
        WebSocketServer b(TEST_PORT + 1);
        b = std::move(a);
        REQUIRE(b.IsRunning());
        b.Stop();
    }
}

// ============================================================================
// WebSocketConnection -- Connect / Disconnect
// ============================================================================

TEST_CASE("WebSocket - Client connect and disconnect", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    REQUIRE(server.Start().has_value());
    REQUIRE(server.IsRunning());

    WebSocketConnection client;

    SECTION("Connect fires OnConnect callback via Poll") {
        bool connected = false;
        client.OnConnect([&connected]() { connected = true; });
        auto rc = client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT));
        REQUIRE(rc.has_value());
        REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));
        REQUIRE(client.IsConnected());
    }

    SECTION("Disconnect fires OnDisconnect callback via Poll") {
        bool connected = false;
        bool disconnected = false;
        client.OnConnect([&connected]() { connected = true; });
        client.OnDisconnect([&disconnected]() { disconnected = true; });
        REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
        REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));
        REQUIRE(client.Disconnect().has_value());
        REQUIRE(WaitFor([&]() { client.Poll(); return disconnected; }));
        REQUIRE_FALSE(client.IsConnected());
    }

    SECTION("Connect while already connected returns InvalidState") {
        bool connected = false;
        client.OnConnect([&connected]() { connected = true; });
        REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
        REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));
        auto rc = client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT));
        REQUIRE_FALSE(rc.has_value());
        REQUIRE(rc.error() == WebSocketError::InvalidState);
    }

    server.Stop();
}

// ============================================================================
// Send / Receive
// ============================================================================

TEST_CASE("WebSocket - Client binary send and server echo", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    server.OnClientMessage([&server](uint64_t client_id, const std::vector<uint8_t>& data) {
        (void)server.Send(client_id, data);
    });
    REQUIRE(server.Start().has_value());

    WebSocketConnection client;
    bool connected = false;
    std::vector<uint8_t> received;
    client.OnConnect([&connected]() { connected = true; });
    client.OnMessage([&received](const std::vector<uint8_t>& data) { received = data; });

    REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
    REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));

    std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0xFF, 0x00, 0xAB};
    REQUIRE(client.Send(payload).has_value());

    REQUIRE(WaitFor([&]() { server.Poll(); client.Poll(); return !received.empty(); }));
    REQUIRE(received == payload);

    server.Stop();
}

TEST_CASE("WebSocket - Client string send and server echo", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    server.OnClientMessage([&server](uint64_t client_id, const std::vector<uint8_t>& data) {
        (void)server.Send(client_id, data);
    });
    REQUIRE(server.Start().has_value());

    WebSocketConnection client;
    bool connected = false;
    std::vector<uint8_t> received;
    client.OnConnect([&connected]() { connected = true; });
    client.OnMessage([&received](const std::vector<uint8_t>& data) { received = data; });

    REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
    REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));

    std::string message = "hello world";
    REQUIRE(client.Send(message).has_value());

    REQUIRE(WaitFor([&]() { server.Poll(); client.Poll(); return !received.empty(); }));
    std::string result(received.begin(), received.end());
    REQUIRE(result == message);

    server.Stop();
}

TEST_CASE("WebSocket - Send while disconnected returns ConnectionClosed", "[websocket]") {
    WebSocketConnection client;
    std::vector<uint8_t> data = {0x01};

    auto rc = client.Send(data);
    REQUIRE_FALSE(rc.has_value());
    REQUIRE(rc.error() == WebSocketError::ConnectionClosed);

    auto rc2 = client.Send("hello");
    REQUIRE_FALSE(rc2.has_value());
    REQUIRE(rc2.error() == WebSocketError::ConnectionClosed);
}

// ============================================================================
// Broadcast
// ============================================================================

TEST_CASE("WebSocket - Server broadcast to multiple clients", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    REQUIRE(server.Start().has_value());

    WebSocketConnection client1;
    WebSocketConnection client2;
    bool c1_connected = false, c2_connected = false;
    std::vector<uint8_t> c1_received, c2_received;

    client1.OnConnect([&c1_connected]() { c1_connected = true; });
    client2.OnConnect([&c2_connected]() { c2_connected = true; });
    client1.OnMessage([&c1_received](const std::vector<uint8_t>& d) { c1_received = d; });
    client2.OnMessage([&c2_received](const std::vector<uint8_t>& d) { c2_received = d; });

    REQUIRE(client1.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
    REQUIRE(client2.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() {
        client1.Poll(); client2.Poll(); server.Poll();
        return c1_connected && c2_connected;
    }));

    std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(server.Broadcast(payload).has_value());

    REQUIRE(WaitFor([&]() {
        client1.Poll(); client2.Poll();
        return !c1_received.empty() && !c2_received.empty();
    }));

    REQUIRE(c1_received == payload);
    REQUIRE(c2_received == payload);

    server.Stop();
}

// ============================================================================
// Server Client Tracking
// ============================================================================

TEST_CASE("WebSocket - Server client count tracks connections", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    uint64_t last_connected_id = 0;
    uint64_t last_disconnected_id = 0;
    server.OnClientConnect([&last_connected_id](uint64_t id) { last_connected_id = id; });
    server.OnClientDisconnect([&last_disconnected_id](uint64_t id) { last_disconnected_id = id; });

    REQUIRE(server.Start().has_value());
    REQUIRE(server.ClientCount() == 0);

    WebSocketConnection client1;
    bool c1_connected = false;
    client1.OnConnect([&c1_connected]() { c1_connected = true; });
    REQUIRE(client1.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() { client1.Poll(); server.Poll(); return c1_connected; }));
    REQUIRE(server.ClientCount() == 1);
    REQUIRE(last_connected_id != 0);

    WebSocketConnection client2;
    bool c2_connected = false;
    client2.OnConnect([&c2_connected]() { c2_connected = true; });
    REQUIRE(client2.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());

    REQUIRE(WaitFor([&]() { client2.Poll(); server.Poll(); return c2_connected; }));
    REQUIRE(server.ClientCount() == 2);

    bool c1_disconnected = false;
    client1.OnDisconnect([&c1_disconnected]() { c1_disconnected = true; });
    REQUIRE(client1.Disconnect().has_value());
    REQUIRE(WaitFor([&]() { client1.Poll(); server.Poll(); return c1_disconnected; }));

    REQUIRE(WaitFor([&]() { server.Poll(); return server.ClientCount() == 1; }));

    server.Stop();
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_CASE("WebSocket - Disconnect while not connected returns error", "[websocket]") {
    WebSocketConnection client;
    auto rc = client.Disconnect();
    REQUIRE_FALSE(rc.has_value());
    REQUIRE(rc.error() == WebSocketError::ConnectionClosed);
}

// ============================================================================
// RAII Cleanup
// ============================================================================

TEST_CASE("WebSocket - Client RAII cleanup on destruction", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    uint64_t disconnected_id = 0;
    server.OnClientDisconnect([&disconnected_id](uint64_t id) { disconnected_id = id; });
    REQUIRE(server.Start().has_value());

    {
        WebSocketConnection client;
        bool connected = false;
        client.OnConnect([&connected]() { connected = true; });
        REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
        REQUIRE(WaitFor([&]() { client.Poll(); server.Poll(); return connected; }));
        REQUIRE(server.ClientCount() == 1);
    }

    REQUIRE(WaitFor([&]() { server.Poll(); return server.ClientCount() == 0; }));

    server.Stop();
}

// ============================================================================
// Move Semantics (Connected State)
// ============================================================================

TEST_CASE("WebSocket - Client move transfers connected state", "[websocket]") {
    WebSocketServer server(TEST_PORT);
    REQUIRE(server.Start().has_value());

    WebSocketConnection client;
    bool connected = false;
    client.OnConnect([&connected]() { connected = true; });
    REQUIRE(client.Connect("ws://127.0.0.1:" + std::to_string(TEST_PORT)).has_value());
    REQUIRE(WaitFor([&]() { client.Poll(); return connected; }));

    REQUIRE(client.IsConnected());

    WebSocketConnection moved(std::move(client));
    REQUIRE(moved.IsConnected());
    REQUIRE_FALSE(client.IsConnected());

    server.Stop();
}
