/*
** EPITECH PROJECT, 2025
** StartServer.cpp
** File description:
** Lobby server implementation with multi-instance game management
*/

#include "StartServer.hpp"
#include "GameServerLoop.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

StartServer *StartServer::_instance = nullptr;

StartServer::StartServer(int port, int base_udp_port)
    : _tcp_port(port), _base_udp_port(base_udp_port), 
      _next_server_id(1), _protocol(), _game_session() {
    
    _instance = this;
    setupSignalHandlers();

    try {
        _tcp_server = std::make_unique<TCPServer>(static_cast<uint16_t>(port));
        std::cout << "=== Lobby Server Started ===" << std::endl;
        std::cout << "TCP Port: " << port << std::endl;
        std::cout << "Base UDP Port: " << base_udp_port << std::endl;
        std::cout << "============================" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Failed to start TCP server: " << e.what() << std::endl;
        throw;
    }
}

StartServer::~StartServer() {
    std::cout << "\n=== Shutting down Lobby Server ===" << std::endl;
    _running = false;
    
    // Cleanup all game instances
    for (const auto &[lobby_id, instance] : _game_instances) {
        if (instance.process_id > 0) {
            std::cout << "Terminating game instance " << instance.server_id 
                      << " (PID: " << instance.process_id << ")" << std::endl;
            kill(instance.process_id, SIGTERM);
        }
    }
    
    _tcp_server.reset();
    _instance = nullptr;
}

void StartServer::setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGCHLD, childSignalHandler);
}

void StartServer::signalHandler(int signal) {
    (void)signal;
    if (_instance) {
        _instance->stop();
    }
}

void StartServer::childSignalHandler(int signal) {
    (void)signal;
    // Reap zombie processes
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        std::cout << "Game instance (PID: " << pid << ") terminated" << std::endl;
    }
}

void StartServer::stop() {
    _running = false;
}

void StartServer::run() {
    lobbyLoop();
}

void StartServer::lobbyLoop() {
    std::cout << "Lobby server running. Waiting for clients..." << std::endl;
    
    while (_running) {
        // Poll for messages
        auto messages = _tcp_server->poll();
        
        if (!messages.empty()) {
            for (const auto &message : messages) {
                processMessage(message);
            }
        }
        
        // Cleanup finished games
        cleanupFinishedGames();
        
        // Small sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void StartServer::processMessage(const ClientMessage &message) {
    std::cout << "[Client " << message.client_id << "] Received message" << std::endl;

    ParsedMessage parsed = _protocol.parseMessage(message.message);

    if (!parsed.valid) {
        std::cerr << "[Client " << message.client_id << "] Invalid message format" << std::endl;
        sendError(message.client_id, ProtocolError::PROTOCOL_VIOLATION);
        return;
    }

    switch (parsed.type) {
    case MessageType::TCP_CONNECT:
        handleConnect(message.client_id, parsed.data);
        break;

    case MessageType::LOBBY_LIST_REQUEST:
        handleLobbyListRequest(message.client_id);
        break;

    case MessageType::LOBBY_INFO_REQUEST:
        handleLobbyInfoRequest(message.client_id, parsed.data);
        break;

    case MessageType::CREATE_LOBBY:
        handleCreateLobby(message.client_id, parsed.data);
        break;

    case MessageType::JOIN_LOBBY:
        handleJoinLobby(message.client_id, parsed.data);
        break;

    case MessageType::LEAVE_LOBBY:
        handleLeaveLobby(message.client_id);
        break;

    case MessageType::TCP_READY:
        handleReady(message.client_id);
        break;

    default:
        std::cerr << "[Client " << message.client_id << "] Unexpected message type" << std::endl;
        sendError(message.client_id, ProtocolError::UNEXPECTED_MESSAGE);
        break;
    }
}

void StartServer::handleConnect(uint32_t client_id, const std::vector<uint8_t> &data) {
    std::cout << "[Client " << client_id << "] Processing CONNECT" << std::endl;

    // Parse username from data
    std::string username = _protocol.parseUsername(data);
    
    if (username.empty()) {
        std::cerr << "[Client " << client_id << "] Invalid or empty username" << std::endl;
        sendError(client_id, ProtocolError::INVALID_USERNAME);
        return;
    }

    // Add client to session
    if (!_game_session.addClient(client_id, username)) {
        std::cerr << "[Client " << client_id << "] Failed to add client" << std::endl;
        sendError(client_id, ProtocolError::INTERNAL_SERVER_ERROR);
        return;
    }

    std::cout << "[Client " << client_id << "] Connected as '" << username << "'" << std::endl;

    std::string response = _protocol.createConnectAck(client_id);
    sendToClient(client_id, response);
}

void StartServer::handleLobbyListRequest(uint32_t client_id) {
    std::cout << "[Client " << client_id << "] Requesting lobby list" << std::endl;

    std::vector<LobbyInfo> lobbies = _game_session.getAllLobbies();
    
    std::string response = _protocol.createLobbyListResponse(lobbies);
    sendToClient(client_id, response);
    
    std::cout << "[Client " << client_id << "] Sent " << lobbies.size() << " lobbies" << std::endl;
}

void StartServer::handleLobbyInfoRequest(uint32_t client_id, const std::vector<uint8_t> &data) {
    if (data.size() < 2) {
        sendError(client_id, ProtocolError::PROTOCOL_VIOLATION);
        return;
    }

    uint16_t lobby_id = ntohs(*reinterpret_cast<const uint16_t*>(data.data()));
    
    std::cout << "[Client " << client_id << "] Requesting info for lobby " << lobby_id << std::endl;

    auto lobby_info = _game_session.getLobbyInfo(lobby_id);
    
    if (!lobby_info.has_value()) {
        sendError(client_id, ProtocolError::LOBBY_NOT_EXIST);
        return;
    }

    std::string response = _protocol.createLobbyInfoResponse(lobby_info.value());
    sendToClient(client_id, response);
}


void StartServer::handleCreateLobby(uint32_t client_id, const std::vector<uint8_t> &data) {
    std::cout << "[Client " << client_id << "] Creating lobby" << std::endl;

    if (data.size() < 3) {
        sendError(client_id, ProtocolError::PROTOCOL_VIOLATION);
        return;
    }

    // Check if already in a lobby
    if (_game_session.isClientInLobby(client_id)) {
        sendError(client_id, ProtocolError::ALREADY_IN_LOBBY);
        return;
    }

    // Parse data
    uint8_t max_players = data[0];
    uint16_t name_len = ntohs(*reinterpret_cast<const uint16_t*>(&data[1]));
    
    if (data.size() < 3 + name_len) {
        sendError(client_id, ProtocolError::PROTOCOL_VIOLATION);
        return;
    }

    std::string lobby_name(data.begin() + 3, data.begin() + 3 + name_len);

    // Create lobby
    auto lobby_id = _game_session.createLobby(client_id, lobby_name, max_players);
    
    if (!lobby_id.has_value()) {
        sendError(client_id, ProtocolError::INTERNAL_SERVER_ERROR);
        return;
    }

    std::cout << "[Client " << client_id << "] Created lobby " << lobby_id.value() 
              << " '" << lobby_name << "'" << std::endl;

    // Send acknowledgment
    std::string response = _protocol.createCreateLobbyAck(lobby_id.value());
    sendToClient(client_id, response);
}

void StartServer::handleJoinLobby(uint32_t client_id, const std::vector<uint8_t> &data) {
    if (data.size() < 2) {
        sendError(client_id, ProtocolError::PROTOCOL_VIOLATION);
        return;
    }

    uint16_t lobby_id = ntohs(*reinterpret_cast<const uint16_t*>(data.data()));
    
    std::cout << "[Client " << client_id << "] Joining lobby " << lobby_id << std::endl;

    // Check if already in a lobby
    if (_game_session.isClientInLobby(client_id)) {
        sendError(client_id, ProtocolError::ALREADY_IN_LOBBY);
        return;
    }

    // Try to join
    auto player_id = _game_session.joinLobby(client_id, lobby_id);
    
    if (!player_id.has_value()) {
        // Determine specific error
        auto lobby = _game_session.getLobby(lobby_id);
        if (!lobby.has_value()) {
            sendError(client_id, ProtocolError::LOBBY_NOT_EXIST);
        } else if (lobby->isFull()) {
            sendError(client_id, ProtocolError::LOBBY_FULL);
        } else if (lobby->status == LobbyStatus::IN_GAME) {
            sendError(client_id, ProtocolError::GAME_ALREADY_STARTED);
        } else {
            sendError(client_id, ProtocolError::INTERNAL_SERVER_ERROR);
        }
        return;
    }

    std::cout << "[Client " << client_id << "] Joined lobby " << lobby_id 
              << " as player " << static_cast<int>(player_id.value()) << std::endl;

    // Get current players
    std::vector<PlayerInfo> players = _game_session.getLobbyPlayers(lobby_id);

    // Send JOIN_LOBBY_ACK to the joining client
    std::string ack_response = _protocol.createJoinLobbyAck(lobby_id, player_id.value(), players);
    sendToClient(client_id, ack_response);

    // Broadcast PLAYER_JOINED to other players in lobby
    auto lobby = _game_session.getLobby(lobby_id);
    if (lobby.has_value()) {
        for (const auto &player : lobby->players) {
            if (player.client_id == client_id) {
                PlayerInfo new_player_info = player.toPlayerInfo();
                std::string broadcast = _protocol.createPlayerJoined(new_player_info);
                broadcastToLobby(lobby_id, broadcast, client_id);
                break;
            }
        }
    }
}

void StartServer::handleLeaveLobby(uint32_t client_id) {
    std::cout << "[Client " << client_id << "] Leaving lobby" << std::endl;

    auto lobby_id_opt = _game_session.getClientLobby(client_id);
    
    if (!lobby_id_opt.has_value()) {
        sendError(client_id, ProtocolError::PLAYER_NOT_IN_LOBBY);
        return;
    }

    uint16_t lobby_id = lobby_id_opt.value();
    
    // Get player_id before leaving
    auto lobby = _game_session.getLobby(lobby_id);
    uint8_t player_id = 0;
    if (lobby.has_value()) {
        for (const auto &player : lobby->players) {
            if (player.client_id == client_id) {
                player_id = player.player_id;
                break;
            }
        }
    }

    // Leave lobby
    if (!_game_session.leaveLobby(client_id)) {
        sendError(client_id, ProtocolError::INTERNAL_SERVER_ERROR);
        return;
    }

    std::cout << "[Client " << client_id << "] Left lobby " << lobby_id << std::endl;

    // Send acknowledgment
    std::string response = _protocol.createLeaveLobbyAck();
    sendToClient(client_id, response);

    // Broadcast PLAYER_LEFT to remaining players
    if (player_id != 0) {
        std::string broadcast = _protocol.createPlayerLeft(player_id);
        broadcastToLobby(lobby_id, broadcast);
    }
}

void StartServer::handleReady(uint32_t client_id) {
    std::cout << "[Client " << client_id << "] Setting ready" << std::endl;

    auto lobby_id_opt = _game_session.getClientLobby(client_id);
    
    if (!lobby_id_opt.has_value()) {
        sendError(client_id, ProtocolError::PLAYER_NOT_IN_LOBBY);
        return;
    }

    uint16_t lobby_id = lobby_id_opt.value();

    if (!_game_session.setPlayerReady(client_id, true)) {
        sendError(client_id, ProtocolError::INTERNAL_SERVER_ERROR);
        return;
    }

    std::cout << "[Client " << client_id << "] Ready in lobby " << lobby_id << std::endl;

    // Check if game can start
    if (_game_session.canStartGame(lobby_id)) {
        std::cout << "[Lobby " << lobby_id << "] All players ready! Starting game..." << std::endl;
        startGameInstance(lobby_id);
    }
}

bool StartServer::startGameInstance(uint16_t lobby_id) {
    auto lobby = _game_session.getLobby(lobby_id);
    if (!lobby.has_value()) {
        std::cerr << "Cannot start game: lobby " << lobby_id << " not found" << std::endl;
        return false;
    }

    // Allocate UDP port
    uint16_t udp_port = allocateUdpPort();
    if (udp_port == 0) {
        std::cerr << "Cannot start game: no UDP ports available" << std::endl;
        return false;
    }

    // Mark game as started
    if (!_game_session.startGame(lobby_id)) {
        freeUdpPort(udp_port);
        std::cerr << "Cannot start game: failed to mark as started" << std::endl;
        return false;
    }

    uint16_t server_id = _next_server_id++;
    uint32_t server_ip = getServerIp();

    std::cout << "=== Starting Game Instance ===" << std::endl;
    std::cout << "Lobby ID: " << lobby_id << std::endl;
    std::cout << "Server ID: " << server_id << std::endl;
    std::cout << "UDP Port: " << udp_port << std::endl;
    std::cout << "Players: " << lobby->players.size() << std::endl;
    std::cout << "==============================" << std::endl;

    // Send GAME_START to all players
    std::string game_start_msg = _protocol.createGameStart(udp_port, server_id, server_ip);
    broadcastToLobby(lobby_id, game_start_msg);

    // Fork process for game instance
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        freeUdpPort(udp_port);
        return false;
    }

    if (pid == 0) {
        // CHILD PROCESS - Game Instance
        std::cout << "[Game Instance " << server_id << "] Started in child process" << std::endl;
        
        try {
            GameServerLoop game_loop(udp_port, lobby->players.size());
            game_loop.start();
            
            while (game_loop.isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            game_loop.stop();
            std::cout << "[Game Instance " << server_id << "] Finished" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "[Game Instance " << server_id << "] Error: " << e.what() << std::endl;
        }
        
        exit(0); // Exit child process
    } else {
        // PARENT PROCESS - Lobby Server
        GameInstance instance(lobby_id, server_id, pid, udp_port, server_ip);
        
        for (const auto &player : lobby->players) {
            instance.player_ids.push_back(player.client_id);
        }
        
        _game_instances[lobby_id] = instance;
        
        std::cout << "[Lobby Server] Game instance " << server_id 
                  << " forked with PID " << pid << std::endl;
        
        // Lobby will be cleaned up when game finishes
        return true;
    }

    return false;
}

uint16_t StartServer::allocateUdpPort() {
    uint16_t port = _base_udp_port;
    
    while (_udp_port_allocations.find(port) != _udp_port_allocations.end()) {
        port++;
        if (port > 65535) {
            return 0; // No ports available
        }
    }
    
    return port;
}

void StartServer::freeUdpPort(uint16_t port) {
    _udp_port_allocations.erase(port);
}

void StartServer::cleanupFinishedGames() {
    std::vector<uint16_t> finished_lobbies;
    
    for (const auto &[lobby_id, instance] : _game_instances) {
        // Check if process is still running
        int status;
        pid_t result = waitpid(instance.process_id, &status, WNOHANG);
        
        if (result != 0) {
            // Process finished
            finished_lobbies.push_back(lobby_id);
        }
    }
    
    for (uint16_t lobby_id : finished_lobbies) {
        auto it = _game_instances.find(lobby_id);
        if (it != _game_instances.end()) {
            std::cout << "[Cleanup] Game instance " << it->second.server_id 
                      << " (lobby " << lobby_id << ") finished" << std::endl;
            
            freeUdpPort(it->second.udp_port);
            _game_instances.erase(it);
            
            // Delete the lobby
            _game_session.deleteLobby(lobby_id);
        }
    }
}

void StartServer::broadcastToLobby(uint16_t lobby_id, const std::string &message,
                                    std::optional<ClientId> exclude) {
    auto lobby = _game_session.getLobby(lobby_id);
    if (!lobby.has_value()) {
        return;
    }

    for (const auto &player : lobby->players) {
        if (exclude.has_value() && player.client_id == exclude.value()) {
            continue;
        }
        sendToClient(player.client_id, message);
    }
}

void StartServer::handleClientDisconnect(uint32_t client_id) {
    std::cout << "[Client " << client_id << "] Disconnected" << std::endl;
    
    // Remove from lobby if in one
    auto lobby_id_opt = _game_session.getClientLobby(client_id);
    if (lobby_id_opt.has_value()) {
        handleLeaveLobby(client_id);
    }
    
    // Remove from session
    _game_session.removeClient(client_id);
}

bool StartServer::sendToClient(uint32_t client_id, const std::string &message) {
    return _tcp_server->sendToClient(client_id, message);
}

void StartServer::sendError(uint32_t client_id, ProtocolError error) {
    std::string error_msg = _protocol.createError(error);
    sendToClient(client_id, error_msg);
}

uint32_t StartServer::getServerIp() const {
    return htonl(INADDR_LOOPBACK);
}
