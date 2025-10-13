/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ATcpServer - Abstract base class for TCP server implementations
*/

#ifndef ATCPSERVER_HPP_
#define ATCPSERVER_HPP_

#include "ITcpServer.hpp"
#include <mutex>
#include <deque>
#include <memory>
#include <unordered_map>
#include <asio.hpp>

struct ClientConnection {
    uint32_t id;
    std::shared_ptr<asio::ip::tcp::socket> socket;
    std::string endpoint;
    bool is_connected;
};

/**
 * @brief Abstract base class providing common TCP server functionality
 * 
 * This class implements shared functionality across different TCP server types
 * while leaving protocol-specific details to derived classes.
 */
class ATcpServer : public ITcpServer {
    public:
        virtual ~ATcpServer() = default;

        /**
         * @brief Poll for incoming messages from clients
         * Thread-safe implementation that retrieves and clears pending messages
         */
        std::vector<ClientMessage> poll() override {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<ClientMessage> out(messages_.begin(), messages_.end());
            messages_.clear();
            return out;
        }

        /**
         * @brief Get list of all connected clients
         * Thread-safe implementation
         */
        std::vector<uint32_t> getConnectedClients() override {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<uint32_t> client_ids;
            for (const auto& pair : clients_) {
                if (pair.second->is_connected) {
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
         * @brief Disconnect a specific client
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
        void enqueueMessage(const ClientMessage& msg) {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.emplace_back(msg);
        }

        /**
         * @brief Register a new client connection (thread-safe)
         */
        void registerClient(uint32_t client_id, std::shared_ptr<ClientConnection> client) {
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
                it->second->is_connected = false;
                clients_.erase(client_id);
            }
        }

        /**
         * @brief Get a client connection safely
         */
        std::shared_ptr<ClientConnection> getClient(uint32_t client_id) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = clients_.find(client_id);
            if (it != clients_.end()) {
                return it->second;
            }
            return nullptr;
        }

        // Protected members accessible to derived classes
        mutable std::mutex mutex_;
        std::deque<ClientMessage> messages_;
        std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> clients_;
        uint32_t next_client_id_ = 1;
};

#endif /* !ATCPSERVER_HPP_ */
