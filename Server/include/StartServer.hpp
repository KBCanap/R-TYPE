/*
** EPITECH PROJECT, 2025
** StartServer.hpp
** File description:
** Lobby server managing multiple game instances via fork/CreateProcess
*/

#ifndef STARTSERVER_HPP_
#define STARTSERVER_HPP_

#include "GameSession.hpp"
#include "Protocole.hpp"
#include "TcpServer.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Platform-specific includes
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <windows.h>
using process_id_t = HANDLE;
#define INVALID_PROCESS_ID NULL
#else
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
using process_id_t = pid_t;
#define INVALID_PROCESS_ID 0
#endif

// Structure to track running game instances
struct GameInstance {
    uint16_t lobby_id;
    uint16_t server_id;
    process_id_t process_id;
    uint16_t udp_port;
    uint32_t server_ip;
    std::vector<ClientId> player_ids;

    GameInstance()
        : lobby_id(0), server_id(0), process_id(INVALID_PROCESS_ID),
          udp_port(0), server_ip(0) {}

    GameInstance(uint16_t lid, uint16_t sid, process_id_t pid, uint16_t port,
                 uint32_t ip)
        : lobby_id(lid), server_id(sid), process_id(pid), udp_port(port),
          server_ip(ip) {}
};

class StartServer {
  public:
    StartServer(int port, int base_udp_port);
    ~StartServer();

    void run();
    void stop();

    static StartServer *getInstance() { return _instance; }

  private:
    // Server configuration
    int _tcp_port;
    int _base_udp_port;
    uint16_t _next_server_id;

    // Server components
    std::unique_ptr<TCPServer> _tcp_server;
    Protocol _protocol;
    GameSession _game_session;

    // Running state
    std::atomic<bool> _running{true};

    // Game instance tracking
    std::unordered_map<uint16_t, GameInstance>
        _game_instances; // lobby_id -> GameInstance
    std::unordered_map<uint16_t, uint16_t>
        _udp_port_allocations; // udp_port -> lobby_id

    // Signal handling
    static StartServer *_instance;
#ifdef _WIN32
    static BOOL WINAPI consoleCtrlHandler(DWORD signal);
#else
    static void signalHandler(int signal);
    static void childSignalHandler(int signal);
#endif
    void setupSignalHandlers();

    // Main loop
    void lobbyLoop();
    void processMessage(const ClientMessage &message);

    // Message handlers - Connection
    void handleConnect(uint32_t client_id, const std::vector<uint8_t> &data);

    // Message handlers - Lobby Navigation
    void handleLobbyListRequest(uint32_t client_id);
    void handleLobbyInfoRequest(uint32_t client_id,
                                const std::vector<uint8_t> &data);

    // Message handlers - Lobby Management
    void handleCreateLobby(uint32_t client_id,
                           const std::vector<uint8_t> &data);
    void handleJoinLobby(uint32_t client_id, const std::vector<uint8_t> &data);
    void handleLeaveLobby(uint32_t client_id);

    // Message handlers - Game Flow
    void handleReady(uint32_t client_id);

    // Game instance management
    uint16_t allocateUdpPort();
    void freeUdpPort(uint16_t port);
    bool startGameInstance(uint16_t lobby_id);
    void cleanupFinishedGames();

    // Platform-specific process management
#ifdef _WIN32
    process_id_t createGameProcess(uint16_t server_id, uint16_t udp_port,
                                   size_t player_count);
    void terminateProcess(process_id_t process_id);
    bool isProcessRunning(process_id_t process_id);
#endif

    // Broadcasting
    void broadcastToLobby(uint16_t lobby_id, const std::string &message,
                          std::optional<ClientId> exclude = std::nullopt);

    // Client management
    void handleClientDisconnect(uint32_t client_id);

    // Utilities
    bool sendToClient(uint32_t client_id, const std::string &message);
    void sendError(uint32_t client_id, ProtocolError error);
    uint32_t getServerIp() const;
};

#endif /* !STARTSERVER_HPP_ */