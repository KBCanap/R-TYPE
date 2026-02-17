/*
** EPITECH PROJECT, 2025
** StartServer.hpp
** File description:
** Lobby server managing multiple game instances via fork
*/

#ifndef STARTSERVER_HPP_
#define STARTSERVER_HPP_

#include "gamelogic/GameSession.hpp"
#include "network/protocol/Protocole.hpp"
#include "network/tcp/TcpServer.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

/** @brief Tracks a running game server instance (forked child process) */
struct GameInstance {
    uint16_t lobby_id;          ///< Lobby identifier
    uint16_t server_id;         ///< Unique server instance ID
    pid_t process_id;           ///< Child process PID
    uint16_t udp_port;          ///< Allocated UDP port for game traffic
    uint32_t server_ip;         ///< Server IP address
    std::vector<ClientId> player_ids; ///< Players in this game

    GameInstance()
        : lobby_id(0), server_id(0), process_id(0), udp_port(0), server_ip(0) {}

    GameInstance(uint16_t lid, uint16_t sid, pid_t pid, uint16_t port,
                 uint32_t ip)
        : lobby_id(lid), server_id(sid), process_id(pid), udp_port(port),
          server_ip(ip) {}
};

/**
 * @brief Main lobby server managing client connections and game instances
 *
 * Handles TCP connections for lobby management and forks child processes
 * to run individual game servers on allocated UDP ports.
 */
class StartServer {
  public:
    /** @brief Initializes lobby server
     * @param port TCP port for lobby connections
     * @param base_udp_port Base port for allocating UDP game servers */
    StartServer(int port, int base_udp_port);

    ~StartServer();

    /** @brief Starts the lobby server main loop */
    void run();

    /** @brief Gracefully stops the lobby server */
    void stop();

    /** @brief Returns singleton instance for signal handling */
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

    /** @brief Handles SIGINT/SIGTERM for graceful shutdown */
    static void signalHandler(int signal);

    /** @brief Handles SIGCHLD to reap finished child processes */
    static void childSignalHandler(int signal);

    /** @brief Registers signal handlers */
    void setupSignalHandlers();

    // Main loop
    /** @brief Main event loop processing client messages */
    void lobbyLoop();

    /** @brief Dispatches message to appropriate handler */
    void processMessage(const ClientMessage &message);

    // Message handlers - Connection
    /** @brief Handles initial client connection and handshake */
    void handleConnect(uint32_t client_id, const std::vector<uint8_t> &data);

    // Message handlers - Lobby Navigation
    /** @brief Sends list of all available lobbies to client */
    void handleLobbyListRequest(uint32_t client_id);

    /** @brief Sends detailed info about a specific lobby */
    void handleLobbyInfoRequest(uint32_t client_id,
                                const std::vector<uint8_t> &data);

    // Message handlers - Lobby Management
    /** @brief Creates a new lobby and adds client as host */
    void handleCreateLobby(uint32_t client_id,
                           const std::vector<uint8_t> &data);

    /** @brief Adds client to an existing lobby */
    void handleJoinLobby(uint32_t client_id, const std::vector<uint8_t> &data);

    /** @brief Removes client from their current lobby */
    void handleLeaveLobby(uint32_t client_id);

    // Message handlers - Game Flow
    /** @brief Marks client as ready and starts game if all ready */
    void handleReady(uint32_t client_id);

    // Game instance management
    /** @brief Allocates next available UDP port for game server
     * @return Allocated port number */
    uint16_t allocateUdpPort();

    /** @brief Releases UDP port back to pool */
    void freeUdpPort(uint16_t port);

    /** @brief Forks child process to run game server for lobby
     * @return True if started successfully */
    bool startGameInstance(uint16_t lobby_id);

    /** @brief Reaps finished child processes and frees resources */
    void cleanupFinishedGames();

    // Broadcasting
    /** @brief Sends message to all clients in a lobby
     * @param exclude Optional client ID to skip */
    void broadcastToLobby(uint16_t lobby_id, const std::string &message,
                          std::optional<ClientId> exclude = std::nullopt);

    // Client management
    /** @brief Handles client disconnect, removes from lobby */
    void handleClientDisconnect(uint32_t client_id);

    // Utilities
    /** @brief Sends raw message to client
     * @return True if sent successfully */
    bool sendToClient(uint32_t client_id, const std::string &message);

    /** @brief Sends error message to client */
    void sendError(uint32_t client_id, ProtocolError error);

    /** @brief Gets server's public IP address */
    uint32_t getServerIp() const;
};

#endif /* !STARTSERVER_HPP_ */
