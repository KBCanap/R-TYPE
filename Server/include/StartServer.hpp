/*
** EPITECH PROJECT, 2025
** StartServer.hpp
** File description:
** class that manage the launch the network and game logic
*/

#ifndef STARTSERVER_HPP_
#define STARTSERVER_HPP_

#include "GameServer.hpp"
#include "GameSession.hpp"
#include "Protocole.hpp"
#include "TcpServer.hpp"
#include <atomic>
#include <csignal>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class StartServer {
  public:
    StartServer(int port);
    ~StartServer();

    void networkLoop();
    bool sendToClient(uint32_t client_id, const std::string &message);
    void disconnectClient(uint32_t client_id);
    std::vector<uint32_t> getConnectedClients();
    void stop();

    static StartServer *getInstance() { return _instance; }

  private:
    int _port;
    std::unique_ptr<TCPServer> _tcp_server;
    std::unique_ptr<GameServer> _game_server;
    Protocol _protocol;
    GameSession _game_session;
    bool _game_started{false};

    std::atomic<bool> _running{true};

    void processMessage(const ClientMessage &message);
    void handleConnect(uint32_t client_id);
    void handleReady(uint32_t client_id);
    void startGame();
    void setupSignalHandlers();

    static StartServer *_instance;
    static void signalHandler(int signal);
};

#endif /* !STARTSERVER_HPP_ */
