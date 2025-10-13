/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** AUdpServer - Abstract base class for UDP server implementations
*/

#ifndef AUDPSERVER_HPP_
#define AUDPSERVER_HPP_

#include "IUdpServer.hpp"
#include <mutex>
#include <deque>
#include <memory>
#include <unordered_map>
#include <asio.hpp>

struct UdpClientInfo {
    uint32_t id;
    asio::ip::udp::endpoint endpoint;
    std::string endpoint_str;
    bool is_active;
};

/**
 * @brief Abstract base class providing common UDP server functionality
 * 
 * This class implements shared functionality across different UDP server types.
 * Unlike TCP, UDP is connectionless, so clients are tracked by their endpoints.
 */
class AUdpServer : public IUdpServer {
    public:
        virtual ~AUdpServer() = default;

        /**
         * @brief Poll for incoming messages from clients
         * Thread-safe implementation that retrieves and clears pending messages
         */
        std::vector<UdpClientMessage> poll() override {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<UdpClientMessage> out(messages_.begin(), messages_.end());
            messages_.clear();
            return out;
        }

        /**
         * @brief Get list of all known clients
         * Thread-safe implementation
         */
        std::vector<uint32_t> getConnectedClients() override {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<uint32_t> client_ids;
            for (const auto& pair : clients_) {
                if (pair.second->is_active) {
                    client_ids.push_back(pair.first);
                }
            }
            return client_ids;
        }

        /**
         * @brief Send message to a specific client
         * Must be implemented by derived classes with protocol-specific logic
         */
        bool sendToClient(uint32_t client_id, const std::string& message) override = 0;

        /**
         * @brief Remove a client from the known clients list
         * Must be implemented by derived classes with protocol-specific logic
         */
        void disconnectClient(uint32_t client_id) override = 0;

    protected:
        /**
         * @brief Generate a unique client ID
         */
        uint32_t generateClientId() {
            return next_client_id_++;
        }

        /**
         * @brief Add a message to the message queue (thread-safe)
         */
        void enqueueMessage(const UdpClientMessage& msg) {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.emplace_back(msg);
        }

        /**
         * @brief Register a new client or update existing one (thread-safe)
         */
        void registerClient(uint32_t client_id, std::shared_ptr<UdpClientInfo> client) {
            std::lock_guard<std::mutex> lock(mutex_);
            clients_[client_id] = client;
        }

        /**
         * @brief Unregister a client (thread-safe)
         */
        void unregisterClient(uint32_t client_id) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = clients_.find(client_id);
            if (it != clients_.end()) {
                it->second->is_active = false;
                clients_.erase(client_id);
            }
        }

        /**
         * @brief Get a client info safely
         */
        std::shared_ptr<UdpClientInfo> getClient(uint32_t client_id) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = clients_.find(client_id);
            if (it != clients_.end()) {
                return it->second;
            }
            return nullptr;
        }

        /**
         * @brief Find client by endpoint
         */
        std::shared_ptr<UdpClientInfo> findClientByEndpoint(const asio::ip::udp::endpoint& endpoint) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string endpoint_str = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
            for (auto& pair : clients_) {
                if (pair.second->endpoint_str == endpoint_str) {
                    return pair.second;
                }
            }
            return nullptr;
        }

        /**
         * @brief Convert endpoint to string
         */
        std::string endpointToString(const asio::ip::udp::endpoint& endpoint) {
            return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
        }

        // Protected members accessible to derived classes
        mutable std::mutex mutex_;
        std::deque<UdpClientMessage> messages_;
        std::unordered_map<uint32_t, std::shared_ptr<UdpClientInfo>> clients_;
        uint32_t next_client_id_ = 1;
};

#endif /* !AUDPSERVER_HPP_ */
