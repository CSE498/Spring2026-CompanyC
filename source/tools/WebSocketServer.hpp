/**
 * @file WebSocketServer.hpp
 * @author Group-9
 * @brief WebSocket server wrapper 
 */

#pragma once

#include "WebSocketConnection.hpp"

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cse498 {

class WebSocketServer {
public:
    explicit WebSocketServer(int port);
    ~WebSocketServer();

    WebSocketServer(WebSocketServer&& other) noexcept;
    WebSocketServer& operator=(WebSocketServer&& other) noexcept;

    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;

    [[nodiscard]] std::expected<void, WebSocketError> Start();
    void Stop();

    void OnClientConnect(std::function<void(uint64_t client_id)> callback);
    void OnClientDisconnect(std::function<void(uint64_t client_id)> callback);
    void OnClientMessage(std::function<void(uint64_t client_id, const std::vector<uint8_t>&)> callback);

    [[nodiscard]] std::expected<void, WebSocketError> Send(uint64_t client_id, const std::vector<uint8_t>& data);
    [[nodiscard]] std::expected<void, WebSocketError> Send(uint64_t client_id, const std::string& data);
    [[nodiscard]] std::expected<void, WebSocketError> Broadcast(const std::vector<uint8_t>& data);
    [[nodiscard]] std::expected<void, WebSocketError> Broadcast(const std::string& data);

    void Poll();
    [[nodiscard]] std::vector<uint64_t> GetClientIds() const;
    [[nodiscard]] size_t ClientCount() const;
    [[nodiscard]] bool IsRunning() const;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace cse498
