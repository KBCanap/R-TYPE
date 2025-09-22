/*
** EPITECH PROJECT, 2025
** UdpServer.hpp
** File description:
** created by dylan adg
*/

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include <asio.hpp>
#include <array>
#include <string>
#include "GameManager.hpp"

class UdpServer {
    public:
        UdpServer(asio::io_context &context, int port);
        ~UdpServer();

        void start_receive();

    private:
        void handle_receive(const std::error_code& error, std::size_t bytes_transferred);
        void handle_send(const std::error_code& error, std::size_t bytes_transferred);
        void send_to_client(const std::string& message, const asio::ip::udp::endpoint& client_endpoint);
        void broadcast_to_all_players(const std::string& message);

        void handle_connect_request(const asio::ip::udp::endpoint& client_endpoint);
        void handle_ready_request(const std::string& playerId);
        void handle_game_message(const std::string& playerId, const std::string& message);


        void start_game_loop();
        void handle_game_tick();

        asio::io_context& _io_context;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _remote_endpoint;
        std::array<char, 4028> _recv_buffer;

        GameManager _game_manager;
        asio::steady_timer _game_timer;

        std::string endpoint_to_string(const asio::ip::udp::endpoint& endpoint);
        std::vector<std::string> split_message(const std::string& message, char delimiter);
};

#endif /* !UDPSERVER_HPP_ */