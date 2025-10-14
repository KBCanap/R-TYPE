/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameServerLoop.hpp
*/

#ifndef GAMESERVERLOOP_HPP_
#define GAMESERVERLOOP_HPP_

#include "GameLogic.hpp"
#include "UdpProtocole.hpp"
#include "UdpServer.hpp"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <memory>
#include <thread>

class GameServerLoop {
  public:
    GameServerLoop(uint16_t port = 4242, uint32_t max_clients = 4);
    ~GameServerLoop();

    void start();
    void stop();
    bool isRunning() const { return _running; }

    static void signalHandler(int signal);
    static GameServerLoop *instance;
    void broadcastEntityUpdates();

  private:
    void run();
    void processMessages();
    void setupSignalHandlers();

    uint16_t _port;
    uint32_t _max_clients;
    bool _in_game;
    std::chrono::time_point<std::chrono::steady_clock> _last_tick;
    uint32_t _sequence_num;
    std::atomic<bool> _running;
    std::unique_ptr<UDPServer> _udp_server;
    std::unique_ptr<std::thread> _loop_thread;
    std::unique_ptr<GameLogic> _game_logic;
    UdpProtocole _protocol;
};

#endif /* !GAMESERVERLOOP_HPP_ */
