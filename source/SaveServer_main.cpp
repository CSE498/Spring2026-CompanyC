/**
 * @file SaveServer_main.cpp
 * @author Group-9
 * @brief Our server to maintain the saved files
 * So the server side of our websocket listening port for saved games
 *
 * Usage: ./SaveServer [--port <N>] [--saves-dir <path>] [--help]
 *
 * Defaults: port 8080, saves directory "./saves/"
 */

#include "core/Database.hpp"
#include "core/SyncManager.hpp"
#include "tools/WebSocketServer.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

using namespace cse498;

static std::atomic<bool> running{true};

static void SignalHandler(int) {
    running.store(false);
}

static void PrintUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [--port <N>] [--saves-dir <path>] [--help]\n"
              << "  --port <N>        WebSocket listen port (1-65535, default: 8080)\n"
              << "  --saves-dir <path> Directory for save files (default: ./saves/)\n"
              << "  --help            Show this message\n";
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string saves_dir = "./saves/";

    // command line args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            PrintUsage(argv[0]);
            return 0;

        } else if (arg == "--port") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --port requires a value\n";
                PrintUsage(argv[0]);
                return 1;
            }

            ++i;
            char* end = nullptr;
            long val = std::strtol(argv[i], &end, 10);

            if (end == argv[i] || *end != '\0' || val < 1 || val > 65535) {
                std::cerr << "Error: invalid port '" << argv[i] << "' (must be 1-65535)\n";
                PrintUsage(argv[0]);
                return 1;
            }
            port = static_cast<int>(val);

        } else if (arg == "--saves-dir") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --saves-dir requires a value\n";
                PrintUsage(argv[0]);
                return 1;
            }
            ++i;
            saves_dir = argv[i];
        } else {
            std::cerr << "Error: unknown flag '" << arg << "'\n";
            PrintUsage(argv[0]);
            return 1;
        }
    }

    std::cout << "[SaveServer] Starting on port " << port << ", saves directory: " << saves_dir << "\n";


    Database db;
    WebSocketServer server(port);

    if (!server.Start().has_value()) {
        std::cerr << "[SaveServer] Failed to start server on port " << port << "\n";
        return 1;
    }

    SyncManager sync(db, server, saves_dir);

    if (!sync.Start().has_value()) {
        std::cerr << "[SaveServer] Failed to start SyncManager\n";
        server.Stop();
        return 1;
    }


    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    std::cout << "[SaveServer] Server ready, waiting for connections...\n";

    // Poll loop
    size_t last_client_count = 0;
    while (running.load()) {
        server.Poll();
        sync.Poll();

        size_t count = server.ClientCount();
        if (count != last_client_count) {
            std::cout << "[SaveServer] Active clients: " << count << "\n";
            last_client_count = count;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n[SaveServer] Shutting down...\n";
    sync.Stop();
    server.Stop();
    std::cout << "[SaveServer] Server stopped.\n";

    return 0;
}
