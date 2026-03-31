/**
 * @file SyncManager.hpp
 * @author Group-9
 * @brief Sync protocol layer connecting Database to WebSocket
 */

#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cse498 {

class Database;
class WebSocketServer;
class WebSocketConnection;

enum class SyncMessageType : uint8_t {
    FULL_STATE     = 0,
    DELTA          = 1,
    UPDATE         = 2,
    SYNC_REQUEST   = 3,
    SYNC_RESPONSE  = 4,
    SAVE           = 5,
    LOAD           = 6,
    LIST_SAVES     = 7,
    SAVE_LIST      = 8
};

enum class SyncError {
    NotStarted,
    AlreadyStarted,
    EncodeFailed,
    DecodeFailed,
    InvalidMessage,
    WebSocketError
};

struct SaveInfo {
    std::string name;
    uint64_t timestamp;  
};

class SyncManager {
public:
    /// Server mode
    SyncManager(Database& db, WebSocketServer& server);

    /// Client mode
    SyncManager(Database& db, WebSocketConnection& client);

    ~SyncManager();

    SyncManager(SyncManager&&) noexcept;
    SyncManager& operator=(SyncManager&&) noexcept;
    SyncManager(const SyncManager&) = delete;
    SyncManager& operator=(const SyncManager&) = delete;

    [[nodiscard]] std::expected<void, SyncError> Start();
    void Stop();

    void Poll();

    [[nodiscard]] bool IsRunning() const;
    [[nodiscard]] bool IsServer() const;

    /// Client only, send a local key change to the server
    [[nodiscard]] std::expected<void, SyncError> SendUpdate(const std::string& key);

    /// Encode a message frame: [msg_type:uint8][payload_length:uint64 BE][payload]
    [[nodiscard]] static std::expected<std::vector<uint8_t>, SyncError> EncodeMessage(SyncMessageType type, const std::vector<uint8_t>& payload);

    /// Decode a message frame back to (type, payload)
    [[nodiscard]] static std::expected<std::pair<SyncMessageType, std::vector<uint8_t>>, SyncError> DecodeMessage(const std::vector<uint8_t>& frame);

    /// Save protocol payload helpers (public for testability)
    [[nodiscard]] static std::vector<uint8_t> EncodeSavePayload(const std::string& name, Database& db);
    [[nodiscard]] static std::expected<std::pair<std::string, std::vector<uint8_t>>, SyncError> DecodeSavePayload(const std::vector<uint8_t>& payload);

    [[nodiscard]] static std::vector<uint8_t> EncodeLoadPayload(const std::string& name);
    [[nodiscard]] static std::expected<std::string, SyncError> DecodeLoadPayload(const std::vector<uint8_t>& payload);

    [[nodiscard]] static std::vector<uint8_t> EncodeSaveListPayload(const std::vector<SaveInfo>& saves);
    [[nodiscard]] static std::expected<std::vector<SaveInfo>, SyncError> DecodeSaveListPayload(const std::vector<uint8_t>& payload);

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace cse498
