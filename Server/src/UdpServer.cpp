/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** UdpServer
*/

#include "UdpServer.hpp"
#include <iostream>

UDPServer::UDPServer(uint16_t port, uint32_t max_clients)
    : socket_(ctx_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      max_clients_(max_clients) {
    start_receive();
    thread_ = std::thread([this]() { ctx_.run(); });
}

UDPServer::~UDPServer() {
    ctx_.stop();
    if (thread_.joinable()) thread_.join();
}

void UDPServer::start_receive()
{
    auto buf = std::make_shared<std::array<char, 1024>>();

    socket_.async_receive_from(
        asio::buffer(*buf), remote_endpoint_,
        [this, buf](std::error_code ec, std::size_t len) {
            if (!ec && len > 0) {
                auto existing_client = findClientByEndpoint(remote_endpoint_);
                
                if (existing_client) {
                    UdpClientMessage msg;
                    msg.client_id = existing_client->id;
                    msg.client_endpoint = existing_client->endpoint_str;
                    msg.message = std::string(buf->data(), len);
                    enqueueMessage(msg);
                } else if (canAcceptNewClient()) {
                    uint32_t client_id = generateClientId();
                    auto client = std::make_shared<UdpClientInfo>();
                    client->id = client_id;
                    client->endpoint = remote_endpoint_;
                    client->endpoint_str = endpointToString(remote_endpoint_);
                    client->is_active = true;
                    registerClient(client_id, client);
                    UdpClientMessage msg;
                    msg.client_id = client->id;
                    msg.client_endpoint = client->endpoint_str;
                    msg.message = std::string(buf->data(), len);
                    enqueueMessage(msg);
                } else {
                    // Max clients reached - ignore new client
                    std::cerr << "Max clients reached (" << max_clients_ 
                              << "). Ignoring new client from " 
                              << endpointToString(remote_endpoint_) << std::endl;
                }
            }
            
            start_receive();
        });
}

bool UDPServer::canAcceptNewClient() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size() < max_clients_;
}

uint32_t UDPServer::getCurrentClientCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<uint32_t>(clients_.size());
}

bool UDPServer::sendToClient(uint32_t client_id, const std::string& message)
{
    auto client = getClient(client_id);
    if (client && client->is_active) {
        try {
            socket_.send_to(asio::buffer(message), client->endpoint);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error sending to client " << client_id << ": " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

void UDPServer::disconnectClient(uint32_t client_id)
{
    unregisterClient(client_id);
}
