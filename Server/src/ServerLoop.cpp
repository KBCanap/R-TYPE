/*
** EPITECH PROJECT, 2025
** ServerLoop.cpp
** File description:
** created by dylan adg
*/

#include "ServerLoop.hpp"
#include "UdpServer.hpp"
#include <iostream>
#include <asio.hpp>
#include <cstring>
#include <csignal>

static void signal_handler(int signal)
{
    std::cout << "\nArrêt du serveur (signal " << signal << ")..." << std::endl;
    if (g_io_context) {
        g_io_context->stop();
    }
}

ServerLoop::ServerLoop(int port, asio::io_context &context)
    : _io_context(context), _udp_server(_io_context, port), _port(port)
{
    std::cout << "Starting UDP server on port " << port << std::endl;
    
    g_io_context = &_io_context;
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    _udp_server.start_receive();
    std::cout << "En attente de connexions UDP..." << std::endl;
}

ServerLoop::~ServerLoop()
{
    std::cout << "ServerLoop destroyed" << std::endl;
}

int ServerLoop::run()
{
    try {
        _io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}

