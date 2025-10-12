/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** UdpServer
*/

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include <cstdint>
#include <string>
#include <asio.hpp>
#include <thread>
#include "AUdpServer.hpp"

class UDPServer : public AUdpServer {
    public:
        UDPServer(uint16_t port, uint32_t max_clients = 4);
        ~UDPServer();

        bool sendToClient(uint32_t client_id, const std::string& message) override;
        void disconnectClient(uint32_t client_id) override;

        uint32_t getMaxClients() const { return max_clients_; }
        uint32_t getCurrentClientCount() const;

    private:
        void start_receive();
        bool canAcceptNewClient() const;

        asio::io_context ctx_;
        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint remote_endpoint_;
        std::thread thread_;
        uint32_t max_clients_;
};

#endif /* !UDPSERVER_HPP_ */
