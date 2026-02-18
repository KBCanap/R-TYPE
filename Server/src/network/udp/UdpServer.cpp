/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** UdpServer
*/

#include "network/udp/UdpServer.hpp"
#include <iostream>

UDPServer::UDPServer(uint16_t port, uint32_t max_clients)
    : socket_(ctx_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      max_clients_(max_clients) {
    start_receive();
    thread_ = std::thread([this]() { ctx_.run(); });
}

UDPServer::~UDPServer() {
    ctx_.stop();
    if (thread_.joinable())
        thread_.join();
}

void UDPServer::start_receive() {
    auto buf = std::make_shared<std::array<char, 1024>>();

    socket_.async_receive_from(
        asio::buffer(*buf), remote_endpoint_,
        [this, buf](std::error_code ec, std::size_t len) {
            if (!ec && len > 0) {
                auto existing_client = findClientByEndpoint(remote_endpoint_);

                if (existing_client) {
                    // Update last activity timestamp
                    existing_client->last_activity = std::chrono::steady_clock::now();

                    UdpClientMessage msg;
                    msg.client_id = existing_client->id;
                    msg.client_endpoint = existing_client->endpoint_str;
                    msg.message = std::string(buf->data(), len);
                    std::cout << "Received message from client "
                              << existing_client->id << " ("
                              << existing_client->endpoint_str << "): " << len
                              << " bytes" << std::endl;
                    enqueueMessage(msg);
                } else if (canAcceptNewClient()) {
                    uint32_t client_id = generateClientId();
                    auto client = std::make_shared<UdpClientInfo>();
                    client->id = client_id;
                    client->endpoint = remote_endpoint_;
                    client->endpoint_str = endpointToString(remote_endpoint_);
                    client->is_active = true;
                    client->last_activity = std::chrono::steady_clock::now();
                    registerClient(client_id, client);
                    UdpClientMessage msg;
                    msg.client_id = client->id;
                    msg.client_endpoint = client->endpoint_str;
                    msg.message = std::string(buf->data(), len);
                    enqueueMessage(msg);
                } else {
                    std::cerr << "Max clients reached (" << max_clients_
                              << "). Ignoring new client from "
                              << endpointToString(remote_endpoint_)
                              << std::endl;
                }
            }

            start_receive();
        });
}

bool UDPServer::canAcceptNewClient() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size() < max_clients_;
}

uint32_t UDPServer::getCurrentClientCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<uint32_t>(clients_.size());
}

bool UDPServer::sendToClient(uint32_t client_id, const std::string &message) {
    auto client = getClient(client_id);
    if (client && client->is_active) {
        try {
            socket_.send_to(asio::buffer(message), client->endpoint);
            return true;
        } catch (const std::exception &e) {
            std::cerr << "Error sending to client " << client_id << ": "
                      << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

void UDPServer::disconnectClient(uint32_t client_id) {
    unregisterClient(client_id);
}

void UDPServer::checkAndDisconnectInactiveClients(std::chrono::seconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    std::vector<uint32_t> inactive_clients;

    for (const auto &[client_id, client] : clients_) {
        if (client->is_active) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - client->last_activity);

            if (elapsed >= timeout) {
                std::cout << "Client " << client_id << " (" << client->endpoint_str
                          << ") timed out after " << elapsed.count()
                          << " seconds of inactivity" << std::endl;
                inactive_clients.push_back(client_id);
            }
        }
    }

    // Disconnect inactive clients (must be done outside the iteration)
    for (uint32_t client_id : inactive_clients) {
        auto it = clients_.find(client_id);
        if (it != clients_.end()) {
            it->second->is_active = false;
            clients_.erase(client_id);
        }
    }
}
