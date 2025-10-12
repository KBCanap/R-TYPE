/*
** EPITECH PROJECT, 2025
** StartServer.cpp
** File description:
** StartServer implementation with protocol handling
*/

#include "StartServer.hpp"
#include <chrono>
#include <iostream>
#include <thread>

StartServer *StartServer::_instance = nullptr;

StartServer::StartServer(int port, int udp_port) : _port(port), _udp_port(udp_port), _protocol(), _game_session()
{
    _instance = this;
    setupSignalHandlers();

    try {
        _tcp_server = std::make_unique<TCPServer>(static_cast<uint16_t>(port));
        std::cout << "TCP Server started on port " << port << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Failed to start TCP server: " << e.what() << std::endl;
        throw;
    }
}

StartServer::~StartServer() {
    std::cout << "\nShutting down server..." << std::endl;
    _running = false;
    _tcp_server.reset();
    _instance = nullptr;
}

void StartServer::setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

void StartServer::signalHandler(int signal) {
    (void)signal;
    if (_instance) {
        _instance->stop();
    }
}

void StartServer::stop() { _running = false; }

void StartServer::networkLoop() {
    auto last_time = std::chrono::steady_clock::now();
    const float target_dt = 1.0f / 60.0f; // 60 FPS

short StartServer::networkLoop()
{
    while (_running && !_in_game) {
        auto messages = _tcp_server->poll();

        if (!messages.empty()) {
            for (const auto &message : messages) {
                processMessage(message);
            }
        }

        // Update game logic if game has started
        if (_game_started && _game_server) {
            _game_server->update(dt);
        }

        // Sleep to maintain target framerate
        auto elapsed = std::chrono::steady_clock::now() - current_time;
        auto sleep_time = std::chrono::duration<float>(target_dt) - elapsed;
        if (sleep_time.count() > 0) {
            std::this_thread::sleep_for(sleep_time);
        }
    }
    if (_in_game) {
        return _game_session.getClientCount();
    }
    return 0;
}

void StartServer::processMessage(const ClientMessage &message) {
    std::cout << "Received raw message from client " << message.client_id
              << " (" << message.client_endpoint << ")" << std::endl;

    ParsedMessage parsed = _protocol.parseMessage(message.message);

    if (!parsed.valid) {
        std::cerr << "Invalid message format from client " << message.client_id
                  << std::endl;
        std::string error_response =
            _protocol.createError(ProtocolError::PROTOCOL_VIOLATION);
        sendToClient(message.client_id, error_response);
        return;
    }

    switch (parsed.type) {
    case MessageType::TCP_CONNECT:
        handleConnect(message.client_id);
        break;

    case MessageType::TCP_READY:
        handleReady(message.client_id);
        break;

    default:
        std::cerr << "Unexpected message type from client " << message.client_id
                  << std::endl;
        std::string error_response =
            _protocol.createError(ProtocolError::UNEXPECTED_MESSAGE);
        sendToClient(message.client_id, error_response);
        break;
    }
}

void StartServer::handleConnect(uint32_t client_id) {
    std::cout << "Processing CONNECT from client " << client_id << std::endl;

    auto result = _game_session.addClient(client_id);

    if (result.first) {
        std::cout << "Client " << client_id << " connected as player "
                  << static_cast<int>(result.second) << std::endl;
        std::string response = _protocol.createConnectAck(result.second);
        sendToClient(client_id, response);
    } else {
        ConnectError error = _game_session.isFull()
                                 ? ConnectError::GAME_FULL
                                 : ConnectError::SERVER_ERROR;

        std::cout << "Connection rejected for client " << client_id
                  << " (Error: " << static_cast<int>(error) << ")" << std::endl;
        std::string response = _protocol.createConnectNak(error);
        sendToClient(client_id, response);
    }
}

void StartServer::handleReady(uint32_t client_id) {
    std::cout << "Processing READY from client " << client_id << std::endl;

    if (!_game_session.setClientReady(client_id)) {
        std::cerr << "Client " << client_id << " not found or already ready"
                  << std::endl;
        std::string response =
            _protocol.createError(ProtocolError::UNEXPECTED_MESSAGE);
        sendToClient(client_id, response);
        return;
    }

    std::cout << "Client " << client_id << " is now ready" << std::endl;

    if (_game_session.canStartGame()) {
        startGame();
    } else {
        std::cout << "Waiting for more clients to be ready. Connected: "
                  << _game_session.getClientCount()
                  << ", Ready: " << _game_session.getReadyClients().size()
                  << std::endl;
    }
}

void StartServer::startGame() {
    std::cout << "All clients ready! Starting game..." << std::endl;

    std::string game_start_message = _protocol.createGameStart(_udp_port);

    auto connected_clients = _game_session.getConnectedClients();
    for (auto client_id : connected_clients) {
        std::cout << "Sending GAME_START to client " << client_id << std::endl;
        sendToClient(client_id, game_start_message);
    }
    
    _in_game = true;
    std::cout << "Game started with UDP port: " << _udp_port << std::endl;
}

bool StartServer::sendToClient(uint32_t client_id, const std::string &message) {
    return _tcp_server->sendToClient(client_id, message);
}

void StartServer::disconnectClient(uint32_t client_id) {
    _tcp_server->disconnectClient(client_id);
    std::cout << "Disconnected client " << client_id << std::endl;
}

std::vector<uint32_t> StartServer::getConnectedClients() {
    return _tcp_server->getConnectedClients();
}
