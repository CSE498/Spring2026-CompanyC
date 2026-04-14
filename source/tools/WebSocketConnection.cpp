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
#include <atomic>
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
    std::atomic<bool> connected{false};

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

#include <emscripten/websocket.h>
#include <queue>

namespace cse498 {

struct Event {
    enum Type { Connect, Disconnect, Message };
    Type type;
    std::vector<uint8_t> data;
};

struct WebSocketConnection::Impl {
    EMSCRIPTEN_WEBSOCKET_T socket = 0;
    bool connected = false;

    std::queue<Event> eventQueue;

    std::function<void(const std::vector<uint8_t>&)> onMessage;
    std::function<void()> onConnect;
    std::function<void()> onDisconnect;

    static EM_BOOL OnOpenCallback(int, const EmscriptenWebSocketOpenEvent*, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        impl->connected = true;
        impl->eventQueue.push({Event::Connect, {}});
        return EM_TRUE;
    }

    static EM_BOOL OnMessageCallback(int, const EmscriptenWebSocketMessageEvent* event, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        impl->eventQueue.push({Event::Message, {event->data, event->data + event->numBytes}});
        return EM_TRUE;
    }

    static EM_BOOL OnCloseCallback(int, const EmscriptenWebSocketCloseEvent*, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        impl->connected = false;
        impl->eventQueue.push({Event::Disconnect, {}});
        return EM_TRUE;
    }

    static EM_BOOL OnErrorCallback(int, const EmscriptenWebSocketErrorEvent*, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        impl->connected = false;
        impl->eventQueue.push({Event::Disconnect, {}});
        return EM_TRUE;
    }
};

WebSocketConnection::WebSocketConnection() : mImpl(std::make_unique<Impl>()) {}

WebSocketConnection::~WebSocketConnection() {
    if (mImpl && mImpl->socket > 0) {
        emscripten_websocket_close(mImpl->socket, 1000, "destructor");
        emscripten_websocket_delete(mImpl->socket);
    }
}

WebSocketConnection::WebSocketConnection(WebSocketConnection&& other) noexcept : mImpl(std::move(other.mImpl)) {}

WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&& other) noexcept {
    if (this != &other) {
        if (mImpl && mImpl->socket > 0) {
            emscripten_websocket_close(mImpl->socket, 1000, "move-assign");
            emscripten_websocket_delete(mImpl->socket);
        }
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
    if (mImpl->connected || mImpl->socket > 0) return std::unexpected(WebSocketError::InvalidState);

    EmscriptenWebSocketCreateAttributes attrs;
    emscripten_websocket_init_create_attributes(&attrs);
    attrs.url = url.c_str();

    mImpl->socket = emscripten_websocket_new(&attrs);
    if (mImpl->socket <= 0) return std::unexpected(WebSocketError::ConnectionFailed);

    emscripten_websocket_set_onopen_callback(mImpl->socket, mImpl.get(), Impl::OnOpenCallback);
    emscripten_websocket_set_onmessage_callback(mImpl->socket, mImpl.get(), Impl::OnMessageCallback);
    emscripten_websocket_set_onclose_callback(mImpl->socket, mImpl.get(), Impl::OnCloseCallback);
    emscripten_websocket_set_onerror_callback(mImpl->socket, mImpl.get(), Impl::OnErrorCallback);

    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Disconnect() {
    if (!mImpl) return std::unexpected(WebSocketError::InvalidState);
    if (!mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);

    mImpl->connected = false;
    emscripten_websocket_close(mImpl->socket, 1000, "client disconnect");
    emscripten_websocket_delete(mImpl->socket);
    mImpl->socket = 0;

    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::vector<uint8_t>& data) {
    if (!mImpl || !mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);

    EMSCRIPTEN_RESULT res = emscripten_websocket_send_binary(mImpl->socket, const_cast<uint8_t*>(data.data()), data.size());
    if (res != EMSCRIPTEN_RESULT_SUCCESS) return std::unexpected(WebSocketError::SendFailed);

    return {};
}

std::expected<void, WebSocketError> WebSocketConnection::Send(const std::string& data) {
    if (!mImpl || !mImpl->connected) return std::unexpected(WebSocketError::ConnectionClosed);

    EMSCRIPTEN_RESULT res = emscripten_websocket_send_utf8_text(mImpl->socket, data.c_str());
    if (res != EMSCRIPTEN_RESULT_SUCCESS) return std::unexpected(WebSocketError::SendFailed);

    return {};
}

void WebSocketConnection::Poll() {
    if (!mImpl) return;

    std::queue<Event> local;
    std::swap(local, mImpl->eventQueue);

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

#endif // __EMSCRIPTEN__
