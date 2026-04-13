/**
 * @file SyncManager.cpp
 * @author Group-9
 * @brief SyncManager implementation
 */

#ifndef __EMSCRIPTEN__

#include "SyncManager.hpp"
#include "Database.hpp"
#include "../tools/WebSocketConnection.hpp"
#include "../tools/WebSocketServer.hpp"
#include "../tools/Serializer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>

namespace cse498 {

struct SyncManager::Impl {
    enum class Mode { Server, Client };
    Mode mode;
    bool running = false;

    Database& db;

    WebSocketServer* server = nullptr;
    WebSocketConnection* client = nullptr;

    std::mutex queueMutex;
    std::queue<std::pair<uint64_t, std::vector<uint8_t>>> incomingMessages;
    std::string savesDir = "./saves/";
    std::function<void(const std::vector<SaveInfo>&)> onSaveListCallback;

    explicit Impl(Database& database) : db(database) {}
};


static constexpr size_t FRAME_HEADER_SIZE = 9;  // 1 byte type + 8 bytes length
static constexpr uint8_t MAX_MESSAGE_TYPE = static_cast<uint8_t>(SyncMessageType::SAVE_LIST);


//constructor, destructor, move funcs

SyncManager::SyncManager(Database& db, WebSocketServer& server) : mImpl(std::make_unique<Impl>(db)) {
    mImpl->mode = Impl::Mode::Server;
    mImpl->server = &server;
}

SyncManager::SyncManager(Database& db, WebSocketServer& server, std::string saves_dir) : mImpl(std::make_unique<Impl>(db)) {
    mImpl->mode = Impl::Mode::Server;
    mImpl->server = &server;
    mImpl->savesDir = std::move(saves_dir);
}

SyncManager::SyncManager(Database& db, WebSocketConnection& client) : mImpl(std::make_unique<Impl>(db)) {
    mImpl->mode = Impl::Mode::Client;
    mImpl->client = &client;
}

SyncManager::~SyncManager() {
    if (mImpl && mImpl->running) Stop();
}

SyncManager::SyncManager(SyncManager&&) noexcept = default;

SyncManager& SyncManager::operator=(SyncManager&& other) noexcept {
    if (this != &other) {
        if (mImpl && mImpl->running) Stop();
        mImpl = std::move(other.mImpl);
    }
    return *this;
}



std::expected<std::vector<uint8_t>, SyncError> SyncManager::EncodeMessage(SyncMessageType type, const std::vector<uint8_t>& payload) {
    uint64_t length = static_cast<uint64_t>(payload.size());

    std::vector<uint8_t> frame;
    frame.reserve(FRAME_HEADER_SIZE + payload.size());

    // msg_type: 1 byte
    frame.push_back(static_cast<uint8_t>(type));

    // payload_length: 8 bytes big-endian
    frame.push_back(static_cast<uint8_t>((length >> 56) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 48) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 40) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 32) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 24) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(length & 0xFF));

    // payload
    frame.insert(frame.end(), payload.begin(), payload.end());

    return frame;
}

std::expected<std::pair<SyncMessageType, std::vector<uint8_t>>, SyncError> SyncManager::DecodeMessage(const std::vector<uint8_t>& frame) {
    if (frame.size() < FRAME_HEADER_SIZE) {
        return std::unexpected(SyncError::DecodeFailed);
    }

    uint8_t raw_type = frame[0];
    if (raw_type > MAX_MESSAGE_TYPE) {
        return std::unexpected(SyncError::InvalidMessage);
    }

    auto type = static_cast<SyncMessageType>(raw_type);

    uint64_t length = (static_cast<uint64_t>(frame[1]) << 56)
                    | (static_cast<uint64_t>(frame[2]) << 48)
                    | (static_cast<uint64_t>(frame[3]) << 40)
                    | (static_cast<uint64_t>(frame[4]) << 32)
                    | (static_cast<uint64_t>(frame[5]) << 24)
                    | (static_cast<uint64_t>(frame[6]) << 16)
                    | (static_cast<uint64_t>(frame[7]) << 8)
                    | static_cast<uint64_t>(frame[8]);

    if (frame.size() < FRAME_HEADER_SIZE + length) {
        return std::unexpected(SyncError::DecodeFailed);
    }

    std::vector<uint8_t> payload(
        frame.begin() + FRAME_HEADER_SIZE,
        frame.begin() + FRAME_HEADER_SIZE + length);

    return std::pair{type, std::move(payload)};
}

// File-local payload helpers

namespace {

std::vector<uint8_t> EncodeFullStatePayload(Database& db) {
    Serializer s;
    auto entries = db.ExportAll();
    std::string payload_str = s.Serialize(static_cast<int>(entries.size()));

    for (const auto& [key, blob, tag] : entries) {
        payload_str += s.Serialize(key);
        payload_str += s.Serialize(tag);
        std::string blob_str(blob.begin(), blob.end());
        payload_str += s.Serialize(blob_str);
    }

    return {payload_str.begin(), payload_str.end()};
}

std::vector<uint8_t> EncodeDeltaPayload(Database& db, const std::vector<std::pair<std::string, ChangeType>>& dirty_keys) {

    Serializer s;
    std::string payload_str = s.Serialize(static_cast<int>(dirty_keys.size()));

    for (const auto& [key, change_type] : dirty_keys) {
        payload_str += s.Serialize(key);
        payload_str += s.Serialize(static_cast<int>(change_type));

        if (change_type == ChangeType::Store || change_type == ChangeType::Update) {
            auto raw = db.GetRawBytes(key);
            auto type_tag = db.GetType(key);

            std::string tag = type_tag.has_value() ? *type_tag : "";
            std::string blob_str;
            if (raw.has_value()) {
                blob_str.assign(raw->begin(), raw->end());
            }

            payload_str += s.Serialize(tag);
            payload_str += s.Serialize(blob_str);
        }
    }

    return {payload_str.begin(), payload_str.end()};
}

void ApplyFullStatePayload(Database& db, const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto count_opt = s.DeserializeAt<int>(data, pos);
    if (!count_opt) return;
    int count = *count_opt;

    std::vector<std::tuple<std::string, std::vector<uint8_t>, std::string>> entries;
    entries.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto key = s.DeserializeAt<std::string>(data, pos);
        auto tag = s.DeserializeAt<std::string>(data, pos);
        auto blob_str = s.DeserializeAt<std::string>(data, pos);
        if (!key || !tag || !blob_str) return;

        std::vector<uint8_t> blob(blob_str->begin(), blob_str->end());
        entries.emplace_back(std::move(*key), std::move(blob), std::move(*tag));
    }

    db.Clear();
    (void)db.ImportAll(entries);
}

void ApplyDeltaPayload(Database& db, const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto count_opt = s.DeserializeAt<int>(data, pos);
    if (!count_opt) return;
    int count = *count_opt;

    for (int i = 0; i < count; ++i) {
        auto key = s.DeserializeAt<std::string>(data, pos);
        auto change_int = s.DeserializeAt<int>(data, pos);
        if (!key || !change_int) return;

        auto change_type = static_cast<ChangeType>(*change_int);

        if (change_type == ChangeType::Delete) {
            db.Delete(*key);
            
        } else {
            auto tag = s.DeserializeAt<std::string>(data, pos);
            auto blob_str = s.DeserializeAt<std::string>(data, pos);
            if (!tag || !blob_str) return;

            std::vector<uint8_t> blob(blob_str->begin(), blob_str->end());
            (void)db.SetRawBytes(*key, blob, *tag, /*mark_dirty=*/false);
        }
    }
}

std::vector<uint8_t> EncodeUpdatePayload(Database& db, const std::string& key) {
    Serializer s;
    auto raw = db.GetRawBytes(key);
    auto type_tag = db.GetType(key);

    std::string tag = type_tag.has_value() ? *type_tag : "";
    std::string blob_str;
    if (raw.has_value()) {
        blob_str.assign(raw->begin(), raw->end());
    }

    std::string payload_str;
    payload_str += s.Serialize(key);
    payload_str += s.Serialize(tag);
    payload_str += s.Serialize(blob_str);

    return {payload_str.begin(), payload_str.end()};
}

void ApplyUpdatePayload(Database& db, const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto key = s.DeserializeAt<std::string>(data, pos);
    auto tag = s.DeserializeAt<std::string>(data, pos);
    auto blob_str = s.DeserializeAt<std::string>(data, pos);
    if (!key || !tag || !blob_str) return;

    std::vector<uint8_t> blob(blob_str->begin(), blob_str->end());
    (void)db.SetRawBytes(*key, blob, *tag, /*mark_dirty=*/true);
}

bool ValidateSaveName(const std::string& name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}

bool WriteSaveFile(const std::string& saves_dir, const std::string& name, const std::vector<uint8_t>& data) {
    std::filesystem::path filepath = std::filesystem::path(saves_dir) / (name + ".save");
    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);

    if (!out) return false;
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));

    return out.good();
}

std::vector<uint8_t> ReadSaveFile(const std::string& saves_dir, const std::string& name) {
    std::filesystem::path filepath = std::filesystem::path(saves_dir) / (name + ".save");
    std::ifstream in(filepath, std::ios::binary | std::ios::ate);

    if (!in) return {};
    auto size = in.tellg();
    if (size <= 0) return {};

    in.seekg(0);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    in.read(reinterpret_cast<char*>(data.data()), size);
    if (!in.good()) return {};

    return data;
}

std::vector<SaveInfo> ListSaveFiles(const std::string& saves_dir) {
    std::vector<SaveInfo> result;
    std::error_code ec;

    for (const auto& entry : std::filesystem::directory_iterator(saves_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".save") continue;

        std::string name = entry.path().stem().string();
        auto ftime = entry.last_write_time();
        auto duration = ftime.time_since_epoch();
        auto sys_duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(duration);
        auto sctp = std::chrono::system_clock::time_point(sys_duration);

        uint64_t timestamp = static_cast<uint64_t>(std::chrono::system_clock::to_time_t(sctp));

        result.push_back(SaveInfo{std::move(name), timestamp});
    }
    return result;
}

} // namespace


// Save protocol payload helpers

std::vector<uint8_t> SyncManager::EncodeSavePayload(const std::string& name, Database& db) {
    Serializer s;
    std::string name_str = s.Serialize(name);
    auto state_bytes = EncodeFullStatePayload(db);

    std::vector<uint8_t> result;
    result.reserve(name_str.size() + state_bytes.size());
    result.insert(result.end(), name_str.begin(), name_str.end());
    result.insert(result.end(), state_bytes.begin(), state_bytes.end());
    return result;
}

std::expected<std::pair<std::string, std::vector<uint8_t>>, SyncError> SyncManager::DecodeSavePayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto name = s.DeserializeAt<std::string>(data, pos);
    if (!name) return std::unexpected(SyncError::DecodeFailed);

    std::vector<uint8_t> state_bytes(payload.begin() + pos, payload.end());
    return std::pair{std::move(*name), std::move(state_bytes)};
}

std::vector<uint8_t> SyncManager::EncodeLoadPayload(const std::string& name) {
    Serializer s;
    std::string payload_str = s.Serialize(name);
    return {payload_str.begin(), payload_str.end()};
}

std::expected<std::string, SyncError> SyncManager::DecodeLoadPayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto name = s.DeserializeAt<std::string>(data, pos);
    if (!name) return std::unexpected(SyncError::DecodeFailed);

    return *name;
}

std::vector<uint8_t> SyncManager::EncodeSaveListPayload(const std::vector<SaveInfo>& saves) {
    Serializer s;
    std::string payload_str = s.Serialize(static_cast<int>(saves.size()));

    for (const auto& info : saves) {
        payload_str += s.Serialize(info.name);
        payload_str += s.Serialize(static_cast<unsigned long long>(info.timestamp));
    }

    return {payload_str.begin(), payload_str.end()};
}

std::expected<std::vector<SaveInfo>, SyncError> SyncManager::DecodeSaveListPayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;

    auto count_opt = s.DeserializeAt<int>(data, pos);
    if (!count_opt || *count_opt < 0) return std::unexpected(SyncError::DecodeFailed);
    int count = *count_opt;

    std::vector<SaveInfo> result;
    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto name = s.DeserializeAt<std::string>(data, pos);
        auto ts = s.DeserializeAt<unsigned long long>(data, pos);
        
        if (!name || !ts) {
            return std::unexpected(SyncError::DecodeFailed);
        }
        result.push_back(SaveInfo{std::move(*name), static_cast<uint64_t>(*ts)});
    }

    return result;
}


// SyncManager main funcs

std::expected<void, SyncError> SyncManager::Start() {
    if (!mImpl) return std::unexpected(SyncError::NotStarted);
    if (mImpl->running) return std::unexpected(SyncError::AlreadyStarted);

    if (mImpl->mode == Impl::Mode::Server) {
        std::filesystem::create_directories(mImpl->savesDir);
        
        mImpl->server->OnClientConnect([this](uint64_t client_id) {
            auto payload = EncodeFullStatePayload(mImpl->db);
            auto frame = EncodeMessage(SyncMessageType::FULL_STATE, payload);

            if (frame.has_value()) {
                (void)mImpl->server->Send(client_id, *frame);
            }
        });

        mImpl->server->OnClientMessage([this](uint64_t client_id, const std::vector<uint8_t>& data) {
            std::lock_guard<std::mutex> lock(mImpl->queueMutex);
            mImpl->incomingMessages.push({client_id, data});
        });

    } else {
        mImpl->client->OnMessage([this](const std::vector<uint8_t>& data) {
            std::lock_guard<std::mutex> lock(mImpl->queueMutex);
            mImpl->incomingMessages.push({0, data});
        });
    }

    mImpl->running = true;
    return {};
}

void SyncManager::Stop() {
    if (!mImpl || !mImpl->running) return;
    mImpl->running = false;
}

void SyncManager::Poll() {
    if (!mImpl || !mImpl->running) return;

    if (mImpl->mode == Impl::Mode::Server) {
        mImpl->server->Poll();
    } else {
        mImpl->client->Poll();
    }

    std::queue<std::pair<uint64_t, std::vector<uint8_t>>> local;
    {
        std::lock_guard<std::mutex> lock(mImpl->queueMutex);
        std::swap(local, mImpl->incomingMessages);
    }

    while (!local.empty()) {
        auto& [client_id, raw_frame] = local.front();
        auto decoded = DecodeMessage(raw_frame);

        if (decoded.has_value()) {
            auto& [msg_type, payload] = *decoded;

            if (mImpl->mode == Impl::Mode::Server) {
                if (msg_type == SyncMessageType::UPDATE) {
                    ApplyUpdatePayload(mImpl->db, payload);
                } else if (msg_type == SyncMessageType::SYNC_REQUEST) {
                    auto full_payload = EncodeFullStatePayload(mImpl->db);
                    auto frame = EncodeMessage(SyncMessageType::SYNC_RESPONSE, full_payload);
                    if (frame.has_value()) {
                        (void)mImpl->server->Send(client_id, *frame);
                    }
                }
                else if (msg_type == SyncMessageType::SAVE) {
                    auto decoded_save = DecodeSavePayload(payload);
                    if (!decoded_save.has_value()) {
                        std::cout << "[SyncManager] Failed to decode SAVE payload\n";

                    } else {
                        auto& [name, state_bytes] = *decoded_save;
                        if (!ValidateSaveName(name)) {
                            std::cout << "[SyncManager] Rejected invalid save name '" << name << "'\n";
                        } else if (!WriteSaveFile(mImpl->savesDir, name, state_bytes)) {
                            std::cout << "[SyncManager] Failed to write save '" << name << "'\n";
                        } else {
                            std::cout << "[SyncManager] Saved '" << name << "' (" << state_bytes.size() << " bytes)\n";
                        }
                    }
                }
                else if (msg_type == SyncMessageType::LOAD) {
                    auto name_result = DecodeLoadPayload(payload);
                    if (!name_result.has_value()) {
                        std::cout << "[SyncManager] Failed to decode LOAD payload\n";

                    } else {
                        auto& name = *name_result;

                        if (!ValidateSaveName(name)) {
                            std::cout << "[SyncManager] Rejected invalid load name '" << name << "'\n";
                        } else {
                            auto file_bytes = ReadSaveFile(mImpl->savesDir, name);
                            if (file_bytes.empty()) {
                                std::cout << "[SyncManager] LOAD requested for unknown save '" << name << "'\n";
                            } else {
                                auto frame = EncodeMessage(SyncMessageType::FULL_STATE, file_bytes);
                                if (frame.has_value()) {
                                    (void)mImpl->server->Send(client_id, *frame);
                                    std::cout << "[SyncManager] Loaded '" << name << "' (" << file_bytes.size() << " bytes) for client " << client_id << "\n";
                                }
                            }
                        }
                    }
                }
                else if (msg_type == SyncMessageType::LIST_SAVES) {
                    auto saves = ListSaveFiles(mImpl->savesDir);
                    auto list_payload = EncodeSaveListPayload(saves);
                    auto frame = EncodeMessage(SyncMessageType::SAVE_LIST, list_payload);

                    if (frame.has_value()) {
                        (void)mImpl->server->Send(client_id, *frame);
                        std::cout << "[SyncManager] Sent save list (" << saves.size() << " saves) to client " << client_id << "\n";
                    }
                }

            } else {
                if (msg_type == SyncMessageType::FULL_STATE || msg_type == SyncMessageType::SYNC_RESPONSE) {
                    ApplyFullStatePayload(mImpl->db, payload);
                    
                } else if (msg_type == SyncMessageType::DELTA) {
                    ApplyDeltaPayload(mImpl->db, payload);

                } else if (msg_type == SyncMessageType::SAVE_LIST) {
                    auto list = DecodeSaveListPayload(payload);

                    if (list.has_value() && mImpl->onSaveListCallback) {
                        mImpl->onSaveListCallback(*list);
                    }
                }
            }
        }

        local.pop();
    }

    if (mImpl->mode == Impl::Mode::Server) {
        auto dirty = mImpl->db.FlushDirty();
        
        if (!dirty.empty()) {
            auto payload = EncodeDeltaPayload(mImpl->db, dirty);
            auto frame = EncodeMessage(SyncMessageType::DELTA, payload);

            if (frame.has_value()) {
                (void)mImpl->server->Broadcast(*frame);
            }
        }
    }
}

bool SyncManager::IsRunning() const {
    return mImpl && mImpl->running;
}

bool SyncManager::IsServer() const {
    return mImpl && mImpl->mode == Impl::Mode::Server;
}

std::expected<void, SyncError> SyncManager::SendUpdate(const std::string& key) {
    if (!mImpl || !mImpl->running) return std::unexpected(SyncError::NotStarted);
    if (mImpl->mode != Impl::Mode::Client) return std::unexpected(SyncError::NotStarted);

    auto raw = mImpl->db.GetRawBytes(key);
    if (!raw.has_value()) return std::unexpected(SyncError::EncodeFailed);

    auto payload = EncodeUpdatePayload(mImpl->db, key);
    auto frame = EncodeMessage(SyncMessageType::UPDATE, payload);
    if (!frame.has_value()) return std::unexpected(SyncError::EncodeFailed);

    auto result = mImpl->client->Send(*frame);
    if (!result.has_value()) return std::unexpected(SyncError::WebSocketError);

    return {};
}

std::expected<void, SyncError> SyncManager::SaveGame(const std::string& name) {
    if (!mImpl || !mImpl->running) return std::unexpected(SyncError::NotStarted);
    if (mImpl->mode != Impl::Mode::Client) return std::unexpected(SyncError::NotStarted);
    if (!ValidateSaveName(name)) return std::unexpected(SyncError::EncodeFailed);

    auto payload = EncodeSavePayload(name, mImpl->db);
    auto frame = EncodeMessage(SyncMessageType::SAVE, payload);
    if (!frame.has_value()) return std::unexpected(SyncError::EncodeFailed);

    auto result = mImpl->client->Send(*frame);
    if (!result.has_value()) return std::unexpected(SyncError::WebSocketError);

    return {};
}

std::expected<void, SyncError> SyncManager::LoadGame(const std::string& name) {
    if (!mImpl || !mImpl->running) return std::unexpected(SyncError::NotStarted);
    if (mImpl->mode != Impl::Mode::Client) return std::unexpected(SyncError::NotStarted);
    if (!ValidateSaveName(name)) return std::unexpected(SyncError::EncodeFailed);

    auto payload = EncodeLoadPayload(name);
    auto frame = EncodeMessage(SyncMessageType::LOAD, payload);
    if (!frame.has_value()) return std::unexpected(SyncError::EncodeFailed);

    auto result = mImpl->client->Send(*frame);
    if (!result.has_value()) return std::unexpected(SyncError::WebSocketError);

    return {};
}

std::expected<void, SyncError> SyncManager::RequestSaveList() {
    if (!mImpl || !mImpl->running) return std::unexpected(SyncError::NotStarted);
    if (mImpl->mode != Impl::Mode::Client) return std::unexpected(SyncError::NotStarted);

    auto frame = EncodeMessage(SyncMessageType::LIST_SAVES, {});
    if (!frame.has_value()) return std::unexpected(SyncError::EncodeFailed);

    auto result = mImpl->client->Send(*frame);
    if (!result.has_value()) return std::unexpected(SyncError::WebSocketError);

    return {};
}

void SyncManager::OnSaveListReceived(std::function<void(const std::vector<SaveInfo>&)> callback) {
    if (mImpl) {
        mImpl->onSaveListCallback = std::move(callback);
    }
}

} // namespace cse498

#else // __EMSCRIPTEN__

#include "SyncManager.hpp"
#include "Database.hpp"
#include "../tools/Serializer.hpp"

namespace cse498 {

struct SyncManager::Impl {};

SyncManager::SyncManager(Database& db, WebSocketServer&) : mImpl(std::make_unique<Impl>()) { (void)db; }
SyncManager::SyncManager(Database& db, WebSocketServer&, std::string) : mImpl(std::make_unique<Impl>()) { (void)db; }
SyncManager::SyncManager(Database& db, WebSocketConnection&) : mImpl(std::make_unique<Impl>()) { (void)db; }
SyncManager::~SyncManager() = default;
SyncManager::SyncManager(SyncManager&&) noexcept = default;
SyncManager& SyncManager::operator=(SyncManager&&) noexcept = default;

std::expected<void, SyncError> SyncManager::Start() {
    return std::unexpected(SyncError::NotStarted);
}
void SyncManager::Stop() {}
void SyncManager::Poll() {}
bool SyncManager::IsRunning() const { return false; }
bool SyncManager::IsServer() const { return false; }
std::expected<void, SyncError> SyncManager::SendUpdate(const std::string&) {
    return std::unexpected(SyncError::NotStarted);
}
std::expected<void, SyncError> SyncManager::SaveGame(const std::string&) {
    return std::unexpected(SyncError::NotStarted);
}
std::expected<void, SyncError> SyncManager::LoadGame(const std::string&) {
    return std::unexpected(SyncError::NotStarted);
}
std::expected<void, SyncError> SyncManager::RequestSaveList() {
    return std::unexpected(SyncError::NotStarted);
}
void SyncManager::OnSaveListReceived(std::function<void(const std::vector<SaveInfo>&)>) {}

std::expected<std::vector<uint8_t>, SyncError> SyncManager::EncodeMessage(SyncMessageType type, const std::vector<uint8_t>& payload) {
    uint64_t length = static_cast<uint64_t>(payload.size());
    std::vector<uint8_t> frame;
    frame.reserve(9 + payload.size());

    frame.push_back(static_cast<uint8_t>(type));
    frame.push_back(static_cast<uint8_t>((length >> 56) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 48) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 40) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 32) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 24) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(length & 0xFF));
    frame.insert(frame.end(), payload.begin(), payload.end());

    return frame;
}

std::expected<std::pair<SyncMessageType, std::vector<uint8_t>>, SyncError> SyncManager::DecodeMessage(const std::vector<uint8_t>& frame) {
    if (frame.size() < 9) return std::unexpected(SyncError::DecodeFailed);
    uint8_t raw_type = frame[0];
    if (raw_type > 8) return std::unexpected(SyncError::InvalidMessage);
    auto type = static_cast<SyncMessageType>(raw_type);
    
    uint64_t length = (static_cast<uint64_t>(frame[1]) << 56)
                    | (static_cast<uint64_t>(frame[2]) << 48)
                    | (static_cast<uint64_t>(frame[3]) << 40)
                    | (static_cast<uint64_t>(frame[4]) << 32)
                    | (static_cast<uint64_t>(frame[5]) << 24)
                    | (static_cast<uint64_t>(frame[6]) << 16)
                    | (static_cast<uint64_t>(frame[7]) << 8)
                    | static_cast<uint64_t>(frame[8]);
    if (frame.size() < 9 + length) return std::unexpected(SyncError::DecodeFailed);
    std::vector<uint8_t> payload(frame.begin() + 9, frame.begin() + 9 + length);
    return std::pair{type, std::move(payload)};
}

std::vector<uint8_t> SyncManager::EncodeSavePayload(const std::string& name, Database& db) {
    Serializer s;
    std::string name_str = s.Serialize(name);

    auto entries = db.ExportAll();
    std::string state_str = s.Serialize(static_cast<int>(entries.size()));
    for (const auto& [key, blob, tag] : entries) {
        state_str += s.Serialize(key);
        state_str += s.Serialize(tag);
        std::string blob_str(blob.begin(), blob.end());
        state_str += s.Serialize(blob_str);
    }

    std::vector<uint8_t> result;
    result.reserve(name_str.size() + state_str.size());
    result.insert(result.end(), name_str.begin(), name_str.end());
    result.insert(result.end(), state_str.begin(), state_str.end());
    return result;
}

std::expected<std::pair<std::string, std::vector<uint8_t>>, SyncError> SyncManager::DecodeSavePayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;
    auto name = s.DeserializeAt<std::string>(data, pos);
    if (!name) return std::unexpected(SyncError::DecodeFailed);
    std::vector<uint8_t> state_bytes(payload.begin() + pos, payload.end());
    return std::pair{std::move(*name), std::move(state_bytes)};
}

std::vector<uint8_t> SyncManager::EncodeLoadPayload(const std::string& name) {
    Serializer s;
    std::string payload_str = s.Serialize(name);
    return {payload_str.begin(), payload_str.end()};
}

std::expected<std::string, SyncError> SyncManager::DecodeLoadPayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;
    auto name = s.DeserializeAt<std::string>(data, pos);
    if (!name) return std::unexpected(SyncError::DecodeFailed);
    return *name;
}

std::vector<uint8_t> SyncManager::EncodeSaveListPayload(const std::vector<SaveInfo>& saves) {
    Serializer s;
    std::string payload_str = s.Serialize(static_cast<int>(saves.size()));
    for (const auto& info : saves) {
        payload_str += s.Serialize(info.name);
        payload_str += s.Serialize(static_cast<unsigned long long>(info.timestamp));
    }
    return {payload_str.begin(), payload_str.end()};
}

std::expected<std::vector<SaveInfo>, SyncError> SyncManager::DecodeSaveListPayload(const std::vector<uint8_t>& payload) {
    Serializer s;
    std::string data(payload.begin(), payload.end());
    size_t pos = 0;
    auto count_opt = s.DeserializeAt<int>(data, pos);
    if (!count_opt || *count_opt < 0) return std::unexpected(SyncError::DecodeFailed);
    int count = *count_opt;
    std::vector<SaveInfo> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        auto name = s.DeserializeAt<std::string>(data, pos);
        auto ts = s.DeserializeAt<unsigned long long>(data, pos);
        if (!name || !ts) return std::unexpected(SyncError::DecodeFailed);
        result.push_back(SaveInfo{std::move(*name), static_cast<uint64_t>(*ts)});
    }
    return result;
}

} // namespace cse498

#endif // __EMSCRIPTEN__
