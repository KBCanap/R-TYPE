/*
** EPITECH PROJECT, 2025
** ServerLoop.hpp
** File description:
** created by dylan adg
*/

#ifndef SERVERLOOP_HPP_
#define SERVERLOOP_HPP_
#include "UdpServer.hpp"
#include <asio.hpp>

static asio::io_context* g_io_context = nullptr;

class ServerLoop {
    public:
        ServerLoop(int port, asio::io_context &context);
        ~ServerLoop();

        int run();

    private:
        asio::io_context& _io_context;
        UdpServer _udp_server;
        int _port;
};

#endif /* !SERVERLOOP_HPP_ */
