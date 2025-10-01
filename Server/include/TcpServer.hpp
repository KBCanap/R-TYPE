/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** TcpServer
*/

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include <cstdint>
#include <string>
#include <asio.hpp>
#include <thread>
#include "ATcpServer.hpp"

class TCPServer : public ATcpServer {
    public:
        TCPServer(uint16_t port);
        ~TCPServer();

        bool sendToClient(uint32_t client_id, const std::string& message) override;
        void disconnectClient(uint32_t client_id) override;

    private:
        void start_accept();
        void start_read(std::shared_ptr<ClientConnection> client);
        std::string getEndpointString(const asio::ip::tcp::socket& socket);

        asio::io_context ctx_;
        asio::ip::tcp::acceptor acceptor_;
        std::thread thread_;
};

#endif /* !TCPSERVER_HPP_ */
