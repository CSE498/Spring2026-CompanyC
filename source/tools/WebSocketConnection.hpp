/**
 * @file WebSocketConnection.hpp
 * @author Group-9
 * @brief WebSocket client wrapper
 */

#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cse498 {

enum class WebSocketError {
    ConnectionFailed,
    ConnectionClosed,
    SendFailed,
    ServerStartFailed,
    InvalidState
};

class WebSocketConnection {
public:
    WebSocketConnection();
    ~WebSocketConnection();

    WebSocketConnection(WebSocketConnection&& other) noexcept;
    WebSocketConnection& operator=(WebSocketConnection&& other) noexcept;

    WebSocketConnection(const WebSocketConnection&) = delete;
    WebSocketConnection& operator=(const WebSocketConnection&) = delete;

    [[nodiscard]] std::expected<void, WebSocketError> Connect(const std::string& url);
    [[nodiscard]] std::expected<void, WebSocketError> Disconnect();
    [[nodiscard]] std::expected<void, WebSocketError> Send(const std::vector<uint8_t>& data);
    [[nodiscard]] std::expected<void, WebSocketError> Send(const std::string& data);

    void OnMessage(std::function<void(const std::vector<uint8_t>&)> callback);
    void OnConnect(std::function<void()> callback);
    void OnDisconnect(std::function<void()> callback);

    void Poll();
    [[nodiscard]] bool IsConnected() const;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace cse498
