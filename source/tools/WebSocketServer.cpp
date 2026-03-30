/**
 * @file WebSocketServer.cpp
 * @author Group-9
 * @brief WebSocket server implementation
 *
 */

#ifndef __EMSCRIPTEN__

#include "WebSocketServer.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <mutex>
#include <queue>
#include <unordered_map>

namespace cse498 {

namespace {
struct NetSystemGuard {
    NetSystemGuard()  { ix::initNetSystem(); }
    ~NetSystemGuard() { ix::uninitNetSystem(); }
};

void EnsureNetInit() { static NetSystemGuard guard; }

} //namespace


struct ServerEvent {
    enum Type { ClientConnect, ClientDisconnect, ClientMessage };
    Type type;
    uint64_t client_id;
    std::vector<uint8_t> data;
};

struct WebSocketServer::Impl {
    int port;
    std::unique_ptr<ix::WebSocketServer> server;
    bool running{false};

    std::mutex queueMutex;
    std::queue<ServerEvent> eventQueue;

    mutable std::mutex clientsMutex;
    std::unordered_map<uint64_t, std::shared_ptr<ix::WebSocket>> clients;
    std::unordered_map<ix::WebSocket*, uint64_t> ptrToId;
    uint64_t nextClientId = 1;

    std::function<void(uint64_t)> onClientConnect;
    std::function<void(uint64_t)> onClientDisconnect;
    std::function<void(uint64_t, const std::vector<uint8_t>&)> onClientMessage;

    void HandleOpen(ix::WebSocket& ws);
    void HandleClose(ix::WebSocket& ws);
    void HandleMessage(ix::WebSocket& ws, const std::string& payload);
    std::shared_ptr<ix::WebSocket> FindClient(uint64_t id);
};

// Impl helpers 

void WebSocketServer::Impl::HandleOpen(ix::WebSocket& ws) {
    uint64_t id;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        id = nextClientId++;
        for (auto& sp : server->getClients()) {
            if (sp.get() == &ws) {
                clients[id] = sp;
                break;
            }
        }
        ptrToId[&ws] = id;
    }
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push({ServerEvent::ClientConnect, id, {}});
}

void WebSocketServer::Impl::HandleClose(ix::WebSocket& ws) {
    uint64_t id;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = ptrToId.find(&ws);
        if (it == ptrToId.end()) return;
        id = it->second;
        clients.erase(id);
        ptrToId.erase(it);
    }
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push({ServerEvent::ClientDisconnect, id, {}});
}

void WebSocketServer::Impl::HandleMessage(ix::WebSocket& ws, const std::string& payload) {
    uint64_t id;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = ptrToId.find(&ws);
        if (it == ptrToId.end()) return;
        id = it->second;
    }
    std::vector<uint8_t> data(payload.begin(), payload.end());
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push({ServerEvent::ClientMessage, id, std::move(data)});
}

std::shared_ptr<ix::WebSocket> WebSocketServer::Impl::FindClient(uint64_t id) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = clients.find(id);
    return (it != clients.end()) ? it->second : nullptr;
}





//constructors, destructors, move operation

WebSocketServer::WebSocketServer(int port) : mImpl(std::make_unique<Impl>()) {
    EnsureNetInit();
    mImpl->port = port;
}

WebSocketServer::~WebSocketServer() {
    if (mImpl && mImpl->running) Stop();
}

WebSocketServer::WebSocketServer(WebSocketServer&& other) noexcept : mImpl(std::move(other.mImpl)) {}

WebSocketServer& WebSocketServer::operator=(WebSocketServer&& other) noexcept {
    if (this != &other) {
        if (mImpl && mImpl->running) Stop();
        mImpl = std::move(other.mImpl);
    }
    return *this;
}




//the actual life cycle processes

std::expected<void, WebSocketError> WebSocketServer::Start() {
    if (!mImpl) return std::unexpected(WebSocketError::InvalidState);
    if (mImpl->running) return std::unexpected(WebSocketError::InvalidState);

    mImpl->server = std::make_unique<ix::WebSocketServer>(mImpl->port, "0.0.0.0");
    mImpl->server->disablePerMessageDeflate();

    mImpl->server->setOnClientMessageCallback([impl = mImpl.get()](std::shared_ptr<ix::ConnectionState>, ix::WebSocket& ws, const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            impl->HandleOpen(ws);

        } else if (msg->type == ix::WebSocketMessageType::Close) {
            impl->HandleClose(ws);

        } else if (msg->type == ix::WebSocketMessageType::Message) {
            impl->HandleMessage(ws, msg->str);
        }   

    });
    
    auto listenResult = mImpl->server->listen();

    if (!listenResult.first) {
        mImpl->server.reset();
        return std::unexpected(WebSocketError::ServerStartFailed);
    }

    mImpl->server->start();
    mImpl->running = true;

    return {};
}

void WebSocketServer::Stop() {
    if (!mImpl || !mImpl->running) return;
    mImpl->server->stop();
    
    {
        std::lock_guard<std::mutex> lock(mImpl->clientsMutex);
        mImpl->clients.clear();
        mImpl->ptrToId.clear();
    }

    mImpl->running = false;
    mImpl->server.reset();
}


//client callback operations

void WebSocketServer::OnClientConnect(std::function<void(uint64_t)> callback) {
    if (mImpl) mImpl->onClientConnect = std::move(callback);
}

void WebSocketServer::OnClientDisconnect(std::function<void(uint64_t)> callback) {
    if (mImpl) mImpl->onClientDisconnect = std::move(callback);
}

void WebSocketServer::OnClientMessage(
    std::function<void(uint64_t, const std::vector<uint8_t>&)> callback) {
    if (mImpl) mImpl->onClientMessage = std::move(callback);
}



//web socket send operations

std::expected<void, WebSocketError> WebSocketServer::Send(uint64_t client_id, const std::vector<uint8_t>& data) {
    if (!mImpl || !mImpl->running) return std::unexpected(WebSocketError::InvalidState);

    auto ws = mImpl->FindClient(client_id);
    if (!ws) return std::unexpected(WebSocketError::ConnectionClosed);

    std::string payload(data.begin(), data.end());
    if (!ws->sendBinary(payload).success) return std::unexpected(WebSocketError::SendFailed);
    
    return {};
}

std::expected<void, WebSocketError> WebSocketServer::Send(uint64_t client_id, const std::string& data) {
    if (!mImpl || !mImpl->running) return std::unexpected(WebSocketError::InvalidState);
    auto ws = mImpl->FindClient(client_id);

    if (!ws) return std::unexpected(WebSocketError::ConnectionClosed);
    if (!ws->send(data).success) return std::unexpected(WebSocketError::SendFailed);

    return {};
}

std::expected<void, WebSocketError> WebSocketServer::Broadcast(const std::vector<uint8_t>& data) {
    if (!mImpl || !mImpl->running) return std::unexpected(WebSocketError::InvalidState);

    std::string payload(data.begin(), data.end());
    std::lock_guard<std::mutex> lock(mImpl->clientsMutex);

    for (auto& [id, ws] : mImpl->clients) {
        ws->sendBinary(payload);
    }

    return {};
}

std::expected<void, WebSocketError> WebSocketServer::Broadcast(const std::string& data) {
    if (!mImpl || !mImpl->running) return std::unexpected(WebSocketError::InvalidState);
    std::lock_guard<std::mutex> lock(mImpl->clientsMutex);

    for (auto& [id, ws] : mImpl->clients) {
        ws->send(data);
    }

    return {};
}

//the event handling that is inbetween each action

void WebSocketServer::Poll() {
    if (!mImpl) return;

    std::queue<ServerEvent> local;
    {
        std::lock_guard<std::mutex> lock(mImpl->queueMutex);
        std::swap(local, mImpl->eventQueue);
    }

    while (!local.empty()) {
        auto& event = local.front();

        switch (event.type) {
        case ServerEvent::ClientConnect:
            if (mImpl->onClientConnect) mImpl->onClientConnect(event.client_id);
            break;

        case ServerEvent::ClientDisconnect:
            if (mImpl->onClientDisconnect) mImpl->onClientDisconnect(event.client_id);
            break;

        case ServerEvent::ClientMessage:
            if (mImpl->onClientMessage) mImpl->onClientMessage(event.client_id, event.data);
            break;
        }

        local.pop();
    }
}


//web socket query functionalities

bool WebSocketServer::IsRunning() const {
    return mImpl && mImpl->running;
}

size_t WebSocketServer::ClientCount() const {
    if (!mImpl) return 0;
    std::lock_guard<std::mutex> lock(mImpl->clientsMutex);

    return mImpl->clients.size();
}

std::vector<uint64_t> WebSocketServer::GetClientIds() const {
    if (!mImpl) return {};

    std::lock_guard<std::mutex> lock(mImpl->clientsMutex);
    std::vector<uint64_t> ids;
    ids.reserve(mImpl->clients.size());

    for (auto& [id, ws] : mImpl->clients) {
        ids.push_back(id);
    }

    return ids;
}

} // namespace cse498

#else // __EMSCRIPTEN__

#include "WebSocketServer.hpp"

namespace cse498 {

WebSocketServer::WebSocketServer(int port) : mImpl(nullptr) { (void)port; }
WebSocketServer::~WebSocketServer() = default;
WebSocketServer::WebSocketServer(WebSocketServer&&) noexcept = default;
WebSocketServer& WebSocketServer::operator=(WebSocketServer&&) noexcept = default;

std::expected<void, WebSocketError> WebSocketServer::Start() {
    return std::unexpected(WebSocketError::ServerStartFailed);
}
void WebSocketServer::Stop() {}

void WebSocketServer::OnClientConnect(std::function<void(uint64_t)>) {}
void WebSocketServer::OnClientDisconnect(std::function<void(uint64_t)>) {}
void WebSocketServer::OnClientMessage(std::function<void(uint64_t, const std::vector<uint8_t>&)>) {}

std::expected<void, WebSocketError> WebSocketServer::Send(uint64_t, const std::vector<uint8_t>&) {
    return std::unexpected(WebSocketError::InvalidState);
}
std::expected<void, WebSocketError> WebSocketServer::Send(uint64_t, const std::string&) {
    return std::unexpected(WebSocketError::InvalidState);
}
std::expected<void, WebSocketError> WebSocketServer::Broadcast(const std::vector<uint8_t>&) {
    return std::unexpected(WebSocketError::InvalidState);
}
std::expected<void, WebSocketError> WebSocketServer::Broadcast(const std::string&) {
    return std::unexpected(WebSocketError::InvalidState);
}

void WebSocketServer::Poll() {}
std::vector<uint64_t> WebSocketServer::GetClientIds() const { return {}; }
size_t WebSocketServer::ClientCount() const { return 0; }
bool WebSocketServer::IsRunning() const { return false; }

} // namespace cse498

#endif // __EMSCRIPTEN__
