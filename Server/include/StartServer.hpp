/*
** EPITECH PROJECT, 2025
** StartServer.hpp
** File description:
** class that manage the launch the network and game logic
*/

#ifndef STARTSERVER_HPP_
#define STARTSERVER_HPP_

#include <atomic>
#include <string>
#include <vector>
#include <cstdint>
#include <csignal>
#include <memory>
#include "Protocole.hpp"
#include "GameSession.hpp"
#include "TcpServer.hpp"

class StartServer {
    public:
        StartServer(int port, int udp_port);
        ~StartServer();

        short networkLoop();
        bool sendToClient(uint32_t client_id, const std::string& message);
        void disconnectClient(uint32_t client_id);
        std::vector<uint32_t> getConnectedClients();
        void stop();

        static StartServer* getInstance() { return _instance; }

    private:
        int _port;
        int _udp_port;
        bool _in_game = false;
        std::unique_ptr<TCPServer> _tcp_server;
        Protocol _protocol;
        GameSession _game_session;

        std::atomic<bool> _running{true};

        void processMessage(const ClientMessage& message);
        void handleConnect(uint32_t client_id);
        void handleReady(uint32_t client_id);
        void startGame();
        void setupSignalHandlers();

        static StartServer* _instance;
        static void signalHandler(int signal);
};

#endif /* !STARTSERVER_HPP_ */
