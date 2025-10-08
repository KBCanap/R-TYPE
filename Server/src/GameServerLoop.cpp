/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameServerLoop.cpp
*/

#include "GameServerLoop.hpp"
#include <iostream>
#include <chrono>

GameServerLoop* GameServerLoop::instance = nullptr;

GameServerLoop::GameServerLoop(uint16_t port, uint32_t max_clients)
    : _port(port),
      _max_clients(max_clients),
      _in_game(false),
      _running(false),
      _udp_server(nullptr),
      _loop_thread(nullptr),
      _protocol()
{
    instance = this;
    setupSignalHandlers();
    std::cout << "GameServerLoop initialized on port " << _port 
              << " (max clients: " << _max_clients << ")" << std::endl;
}

GameServerLoop::~GameServerLoop()
{
    stop();
    instance = nullptr;
}

void GameServerLoop::setupSignalHandlers()
{
    std::signal(SIGINT, GameServerLoop::signalHandler);
    std::signal(SIGTERM, GameServerLoop::signalHandler);
}

void GameServerLoop::signalHandler(int signal)
{
    std::cout << "\n[SIGNAL] Received signal " << signal << " (";
    if (signal == SIGINT) {
        std::cout << "SIGINT - Ctrl+C";
    } else if (signal == SIGTERM) {
        std::cout << "SIGTERM";
    }
    std::cout << ")" << std::endl;
    
    if (instance) {
        std::cout << "[SIGNAL] Stopping server gracefully..." << std::endl;
        instance->stop();
    }
    
    std::exit(0);
}

void GameServerLoop::start()
{
    if (_running) {
        std::cout << "GameServerLoop is already running!" << std::endl;
        return;
    }

    try {
        _udp_server = std::make_unique<UDPServer>(_port, _max_clients);
        std::cout << "UDP Server started on port " << _port << std::endl;
        _running = true;
        _loop_thread = std::make_unique<std::thread>(&GameServerLoop::run, this);
        
        std::cout << "GameServerLoop started successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start GameServerLoop: " << e.what() << std::endl;
        _running = false;
    }
}

void GameServerLoop::stop()
{
    if (!_running) {
        return;
    }

    std::cout << "Stopping GameServerLoop..." << std::endl;
    _running = false;

    if (_loop_thread && _loop_thread->joinable()) {
        _loop_thread->join();
    }

    _udp_server.reset();
    std::cout << "GameServerLoop stopped" << std::endl;
}

void GameServerLoop::run()
{
    std::cout << "Game loop started" << std::endl;

    while (_running) {
        processMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "Game loop ended" << std::endl;
}

void GameServerLoop::processMessages()
{
    if (!_udp_server) {
        return;
    }
    auto messages = _udp_server->poll();

    for (const auto& msg : messages) {
        std::cout << "\n=== Message Received ===" << std::endl;
        std::cout << "Client ID: " << msg.client_id << std::endl;
        std::cout << "Endpoint: " << msg.client_endpoint << std::endl;
        std::cout << "Message size: " << msg.message.size() << " bytes" << std::endl;
        std::cout << "Raw data (hex): ";
        for (unsigned char c : msg.message) {
            printf("%02x ", c);
        }
        std::cout << std::endl;
        ParsedUdpMessage parsed = _protocol.parseMessage(msg.message);
        
        if (parsed.valid) {
            std::cout << "✓ Valid message parsed" << std::endl;
            std::cout << "  Type: 0x" << std::hex << static_cast<int>(parsed.type) << std::dec << std::endl;
            std::cout << "  Sequence: " << parsed.sequence_num << std::endl;
            std::cout << "  Data length: " << parsed.data.size() << " bytes" << std::endl;
        } else {
            std::cout << "✗ Invalid message format" << std::endl;
        }
        
        std::cout << "======================\n" << std::endl;
    }

    // Display current client count periodically
    static int frame_count = 0;
    if (++frame_count % 300 == 0) { // Every ~5 seconds at 60 FPS
        auto clients = _udp_server->getConnectedClients();
        std::cout << "[INFO] Connected clients: " << clients.size() << "/" << _max_clients << std::endl;
    }
}
