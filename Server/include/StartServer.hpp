/*
** EPITECH PROJECT, 2025
** StartServer.hpp
** File description:
** class that manage the launch the network and game logic
*/

#ifndef STARTSERVER_HPP_
#define STARTSERVER_HPP_

#include <dlfcn.h>
#include <atomic>
#include <string>
#include <vector>
#include <cstdint>
#include <csignal>
#include "Protocole.hpp"
#include "GameSession.hpp"

struct ClientMessage {
    std::string client_endpoint;
    std::string message;
    uint32_t client_id;
};

class StartServer {
    public:
        StartServer(int port);
        ~StartServer();

        void networkLoop();
        bool sendToClient(uint32_t client_id, const std::string& message);
        void disconnectClient(uint32_t client_id);
        std::vector<uint32_t> getConnectedClients();
        void stop();

        static StartServer* getInstance() { return _instance; }

    private:
        int _port;
        void* _handle = nullptr;
        void* _tcp_server = nullptr;
        Protocol _protocol;
        GameSession _game_session;

        std::atomic<bool> _running{true};

        std::vector<ClientMessage> (*_poll_tcp_messages)(void*) = nullptr;
        void (*_stop_tcp_server)(void*) = nullptr;
        bool (*_send_to_client)(void*, uint32_t, const char*) = nullptr;
        void (*_disconnect_client)(void*, uint32_t) = nullptr;
        std::vector<uint32_t> (*_get_connected_clients)(void*) = nullptr;

        void processMessage(const ClientMessage& message);
        void handleConnect(uint32_t client_id);
        void handleReady(uint32_t client_id);
        void startGame();
        void setupSignalHandlers();

        static StartServer* _instance;
        static void signalHandler(int signal);
};

#endif /* !STARTSERVER_HPP_ */
