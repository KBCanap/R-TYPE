/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** Tcpserver
*/

#include "TcpServer.hpp"
#include <iostream>

TCPServer::TCPServer(uint16_t port)
    : acceptor_(ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    start_accept();
    thread_ = std::thread([this]() { ctx_.run(); });
}

TCPServer::~TCPServer() {
    ctx_.stop();
    if (thread_.joinable()) thread_.join();
}

void TCPServer::start_accept()
{
    auto socket = std::make_shared<asio::ip::tcp::socket>(ctx_);

    acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
        if (!ec) {
            uint32_t client_id = generateClientId();
            std::string endpoint = getEndpointString(*socket);
            
            auto client = std::make_shared<ClientConnection>();
            client->id = client_id;
            client->socket = socket;
            client->endpoint = endpoint;
            client->is_connected = true;
            
            registerClient(client_id, client);
            start_read(client);
        }
        start_accept();
    });
}

void TCPServer::start_read(std::shared_ptr<ClientConnection> client)
{
    auto buf = std::make_shared<std::vector<char>>(1024);

    client->socket->async_read_some(asio::buffer(*buf), 
        [this, client, buf](std::error_code ec, std::size_t len) {
            if (!ec) {
                ClientMessage msg;
                msg.client_id = client->id;
                msg.client_endpoint = client->endpoint;
                msg.message = std::string(buf->data(), len);
                
                enqueueMessage(msg);
                start_read(client);
            } else {
                unregisterClient(client->id);
            }
        });
}

bool TCPServer::sendToClient(uint32_t client_id, const std::string& message)
{
    auto client = getClient(client_id);
    if (client && client->is_connected) {
        try {
            asio::write(*(client->socket), asio::buffer(message));
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error sending to client " << client_id << ": " << e.what() << std::endl;
            unregisterClient(client_id);
            return false;
        }
    }
    return false;
}

void TCPServer::disconnectClient(uint32_t client_id)
{
    auto client = getClient(client_id);
    if (client) {
        try {
            client->socket->close();
        } catch (...) {
        }
        unregisterClient(client_id);
    }
}

std::string TCPServer::getEndpointString(const asio::ip::tcp::socket& socket)
{
    try {
        auto remote = socket.remote_endpoint();
        return remote.address().to_string() + ":" + std::to_string(remote.port());
    } catch (const std::exception&) {
        return "unknown";
    }
}
