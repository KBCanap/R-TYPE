/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameServerLoop.hpp
*/

#ifndef GAMESERVERLOOP_HPP_
#define GAMESERVERLOOP_HPP_

#include "UdpProtocole.hpp"
#include "UdpServer.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include <csignal>

class GameServerLoop {
public:
    GameServerLoop(uint16_t port = 4242, uint32_t max_clients = 4);
    ~GameServerLoop();

    void start();
    void stop();
    bool isRunning() const { return _running; }
    
    static void signalHandler(int signal);
    static GameServerLoop* instance;

private:
    void run();
    void processMessages();
    void setupSignalHandlers();

    uint16_t _port;
    uint32_t _max_clients;
    bool _in_game;
    std::atomic<bool> _running;
    std::unique_ptr<UDPServer> _udp_server;
    std::unique_ptr<std::thread> _loop_thread;
    UdpProtocole _protocol;
};

#endif /* !GAMESERVERLOOP_HPP_ */
