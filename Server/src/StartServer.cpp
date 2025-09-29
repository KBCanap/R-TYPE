/*
** EPITECH PROJECT, 2025
** StartServer.cpp
** File description:
** StartServer implementation with protocol handling
*/

#include "StartServer.hpp"
#include <iostream>

StartServer* StartServer::_instance = nullptr;

StartServer::StartServer(int port) : _port(port), _protocol(), _game_session()
{
    _instance = this;
    setupSignalHandlers();

    _handle = dlopen("./libnet.so", RTLD_LAZY);
    if (!_handle) {
        std::cerr << "dlopen failed: " << dlerror() << std::endl;
        throw std::runtime_error("Failed to load network library");
    }

    void* (*start_tcp_server)(uint16_t) = reinterpret_cast<void*(*)(uint16_t)>(dlsym(_handle, "start_tcp_server"));
    _poll_tcp_messages = reinterpret_cast<std::vector<ClientMessage>(*)(void*)>(dlsym(_handle, "poll_tcp_messages"));
    _stop_tcp_server = reinterpret_cast<void(*)(void*)>(dlsym(_handle, "stop_tcp_server"));
    _send_to_client = reinterpret_cast<bool(*)(void*, uint32_t, const char*)>(dlsym(_handle, "send_to_client"));
    _disconnect_client = reinterpret_cast<void(*)(void*, uint32_t)>(dlsym(_handle, "disconnect_client"));
    _get_connected_clients = reinterpret_cast<std::vector<uint32_t>(*)(void*)>(dlsym(_handle, "get_connected_clients"));

    if (!start_tcp_server || !_poll_tcp_messages || !_stop_tcp_server || 
        !_send_to_client || !_disconnect_client || !_get_connected_clients) {
        std::cerr << "dlsym failed: " << dlerror() << std::endl;
        dlclose(_handle);
        throw std::runtime_error("Failed to load network functions");
    }

    _tcp_server = start_tcp_server(static_cast<uint16_t>(port));
    if (!_tcp_server) {
        dlclose(_handle);
        throw std::runtime_error("Failed to start TCP server");
    }
    
    std::cout << "TCP Server started on port " << port << std::endl;
}

StartServer::~StartServer()
{
    std::cout << "\nShutting down server..." << std::endl;
    _running = false;

    if (_tcp_server && _stop_tcp_server) {
        _stop_tcp_server(_tcp_server);
    }

    if (_handle) {
        dlclose(_handle);
    }

    _instance = nullptr;
}

void StartServer::setupSignalHandlers()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

void StartServer::signalHandler(int signal)
{
    (void)signal;
    if (_instance) {
        _instance->stop();
    }
}

void StartServer::stop()
{
    _running = false;
}

void StartServer::networkLoop()
{
    while (_running) {
        auto messages = _poll_tcp_messages(_tcp_server);
        
        if (!messages.empty()) {
            for (const auto& message : messages) {
                processMessage(message);
            }
        }
    }
}

void StartServer::processMessage(const ClientMessage& message)
{
    std::cout << "Received raw message from client " << message.client_id 
              << " (" << message.client_endpoint << ")" << std::endl;

    ParsedMessage parsed = _protocol.parseMessage(message.message);
    
    if (!parsed.valid) {
        std::cerr << "Invalid message format from client " << message.client_id << std::endl;
        std::string error_response = _protocol.createError(ProtocolError::PROTOCOL_VIOLATION);
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
            std::cerr << "Unexpected message type from client " << message.client_id << std::endl;
            std::string error_response = _protocol.createError(ProtocolError::UNEXPECTED_MESSAGE);
            sendToClient(message.client_id, error_response);
            break;
    }
}

void StartServer::handleConnect(uint32_t client_id)
{
    std::cout << "Processing CONNECT from client " << client_id << std::endl;

    auto result = _game_session.addClient(client_id);
    
    if (result.first) {
        std::cout << "Client " << client_id << " connected as player " 
                  << static_cast<int>(result.second) << std::endl;
        std::string response = _protocol.createConnectAck(result.second);
        sendToClient(client_id, response);
    } else {
        ConnectError error = _game_session.isFull() ? 
            ConnectError::GAME_FULL : ConnectError::SERVER_ERROR;
        
        std::cout << "Connection rejected for client " << client_id 
                  << " (Error: " << static_cast<int>(error) << ")" << std::endl;
        std::string response = _protocol.createConnectNak(error);
        sendToClient(client_id, response);
    }
}

void StartServer::handleReady(uint32_t client_id)
{
    std::cout << "Processing READY from client " << client_id << std::endl;
    
    if (!_game_session.setClientReady(client_id)) {
        std::cerr << "Client " << client_id << " not found or already ready" << std::endl;
        std::string response = _protocol.createError(ProtocolError::UNEXPECTED_MESSAGE);
        sendToClient(client_id, response);
        return;
    }
    
    std::cout << "Client " << client_id << " is now ready" << std::endl;

    if (_game_session.canStartGame()) {
        startGame();
    } else {
        std::cout << "Waiting for more clients to be ready. Connected: " 
                  << _game_session.getClientCount() << ", Ready: " 
                  << _game_session.getReadyClients().size() << std::endl;
    }
}

void StartServer::startGame()
{
    std::cout << "All clients ready! Starting game..." << std::endl;

    uint16_t udp_port = static_cast<uint16_t>(8080);
    std::string game_start_message = _protocol.createGameStart(udp_port);

    auto connected_clients = _game_session.getConnectedClients();
    for (auto client_id : connected_clients) {
        std::cout << "Sending GAME_START to client " << client_id << std::endl;
        sendToClient(client_id, game_start_message);
    }
    
    std::cout << "Game started with UDP port: " << udp_port << std::endl;
}

bool StartServer::sendToClient(uint32_t client_id, const std::string& message)
{
    if (_send_to_client && _tcp_server) {
        return _send_to_client(_tcp_server, client_id, message.c_str());
    }
    return false;
}

void StartServer::disconnectClient(uint32_t client_id)
{
    if (_disconnect_client && _tcp_server) {
        _disconnect_client(_tcp_server, client_id);
        std::cout << "Disconnected client " << client_id << std::endl;
    }
}

std::vector<uint32_t> StartServer::getConnectedClients()
{
    if (_get_connected_clients && _tcp_server) {
        return _get_connected_clients(_tcp_server);
    }
    return std::vector<uint32_t>();
}
