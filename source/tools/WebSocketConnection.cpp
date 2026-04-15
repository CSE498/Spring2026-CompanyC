/**
 * @file WebSocketConnection.cpp
 * @author Group-9
 * @brief WebSocket client implementation 
 *
 */

#ifndef __EMSCRIPTEN__

#include "WebSocketConnection.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <queue>

namespace cse498 {

namespace {
struct NetSystemGuard {
    NetSystemGuard() {ix::initNetSystem();}
    ~NetSystemGuard() {ix::uninitNetSystem();}
};

void EnsureNetInit() {static NetSystemGuard guard;}

} // namespace

struct Event {
    enum Type {Connect, Disconnect, Message};
    Type type;
    std::vector<uint8_t> data;
};

struct WebSocketConnection::Impl {
    ix::WebSocket ws;
    bool connected{false};

    std::mutex queueMutex;
    std::queue<Event> eventQueue;

    std::function<void(const std::vector<uint8_t>&)> onMessage;
    std::function<void()> onConnect;
    std::function<void()> onDisconnect;
};

WebSocketConnection::WebSocketConnection() : mImpl(std::make_unique<Impl>()) {
    EnsureNetInit();
}

WebSocketConnection::~WebSocketConnection() {
    if (mImpl && mImpl->connected) mImpl->ws.stop();
}

WebSocketConnection::WebSocketConnection(WebSocketConnection&& other) noexcept : mImpl(std::move(other.mImpl)) {}

WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&& other) noexcept {
    if (this != &other) {
        if (mImpl && mImpl->connected) mImpl->ws.stop();
        mImpl = std::move(other.mImpl);
    }
    return *this;
}

bool WebSocketConnection::IsConnected() const {
    return mImpl && mImpl->connected;
}

void WebSocketConnection::OnMessage(std::function<void(const std::vector<uint8_t>&)> callback) {
    if (mImpl) mImpl->onMessage = std::move(callback);
}

void WebSocketConnection::OnConnect(std::function<void()> callback) {
    if (mImpl) mImpl->onConnect = std::move(callback);
}

void WebSocketConnection::OnDisconnect(std::function<void()> callback) {
    if (mImpl) mImpl->onDisconnect = std::move(callback);
}

std::expected<void, WebSocketError> WebSocketConnection::Connect(const std::string& url) {
    if (!mImpl) return std::unexpected(WebSocketError::InvalidState);
    if (mImpl->connected) return std::unexpected(WebSocketError::InvalidState);

    mImpl->ws.setUrl(url);
    mImpl->ws.disableAutomaticReconnection();

    mImpl->ws.setOnMessageCallback([impl = mImpl.get()](const ix::WebSocketMessagePtr& msg) {
        std::lock_guard<std::mutex> lock(impl->queueMutex);

        if (msg->type == ix::WebSocketMessageType::Open) {
            impl->connected = true;
            impl->eventQueue.push({Event::Connect, {}});

        } else if (msg->type == ix::WebSocketMessageType::Close) {
            impl->connected = false;
            impl->eventQueue.push({Event::Disconnect, {}});

        } else if (msg->type == ix::WebSocketMessageType::Message) {
            impl->eventQueue.push({Event::Message, {msg->str.begin(), msg->str.end()}});

        } else if (msg->type == ix::WebSocketMessageType::Error) {
            impl->connected = false;
            impl->eventQueue.push({Event::Disconnect, {}});
        }
    });

    mImpl->ws.start();
    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Disconnect() {
    if (!mImpl) return std::unexpected(WebSocketError::InvalidState);
    if (!mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);

    mImpl->connected = false;
    mImpl->ws.stop();

    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::vector<uint8_t>& data) {
    if (!mImpl || !mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);

    std::string payload(data.begin(), data.end());
    if (!mImpl->ws.sendBinary(payload).success) return std::unexpected(WebSocketError::SendFailed);

    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::string& data) {
    if (!mImpl || !mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);
    if (!mImpl->ws.send(data).success) return std::unexpected(WebSocketError::SendFailed);

    return {};
}

void WebSocketConnection::Poll() {
    if (!mImpl) return;

    std::queue<Event> local;
    {
        std::lock_guard<std::mutex> lock(mImpl->queueMutex);
        std::swap(local, mImpl->eventQueue);
    }

    while (!local.empty()) {
        auto& event = local.front();

        switch (event.type) {
        case Event::Connect:
            if (mImpl->onConnect) mImpl->onConnect();
            break;

        case Event::Disconnect:
            if (mImpl->onDisconnect) mImpl->onDisconnect();
            break;
            
        case Event::Message:
            if (mImpl->onMessage) mImpl->onMessage(event.data);
            break;
        }

        local.pop();
    }
}

} // namespace cse498

#else // __EMSCRIPTEN__

#include "WebSocketConnection.hpp"

namespace cse498 {

WebSocketConnection::WebSocketConnection() : mImpl(nullptr) {}
WebSocketConnection::~WebSocketConnection() = default;
WebSocketConnection::WebSocketConnection(WebSocketConnection&&) noexcept = default;
WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&&) noexcept = default;

std::expected<void, WebSocketError> WebSocketConnection::Connect(const std::string&) {
    return std::unexpected(WebSocketError::InvalidState);
}

std::expected<void, WebSocketError> WebSocketConnection::Disconnect() {
    return std::unexpected(WebSocketError::InvalidState);
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::vector<uint8_t>&) {
    return std::unexpected(WebSocketError::ConnectionClosed);
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::string&) {
    return std::unexpected(WebSocketError::ConnectionClosed);
}

void WebSocketConnection::OnMessage(std::function<void(const std::vector<uint8_t>&)>) {}
void WebSocketConnection::OnConnect(std::function<void()>) {}
void WebSocketConnection::OnDisconnect(std::function<void()>) {}

void WebSocketConnection::Poll() {}
bool WebSocketConnection::IsConnected() const { return false; }

} // namespace cse498

#endif // __EMSCRIPTEN__
