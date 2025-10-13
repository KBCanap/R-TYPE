/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** IServer - Interface for server implementations
*/

#ifndef ISERVER_HPP_
#define ISERVER_HPP_

#include <cstdint>
#include <string>
#include <vector>

struct ClientMessage {
    std::string client_endpoint;
    std::string message;
    uint32_t client_id; 
};

/**
 * @brief Interface for server implementations
 * 
 * This interface defines the contract that all server implementations must follow.
 * It provides methods for polling messages, sending data to clients, and managing connections.
 */
class ITcpServer {
    public:
        virtual ~ITcpServer() = default;

        /**
         * @brief Poll for incoming messages from clients
         * @return Vector of ClientMessage containing all pending messages
         */
        virtual std::vector<ClientMessage> poll() = 0;

        /**
         * @brief Send a message to a specific client
         * @param client_id The unique identifier of the client
         * @param message The message to send
         * @return true if the message was sent successfully, false otherwise
         */
        virtual bool sendToClient(uint32_t client_id, const std::string& message) = 0;

        /**
         * @brief Disconnect a specific client
         * @param client_id The unique identifier of the client to disconnect
         */
        virtual void disconnectClient(uint32_t client_id) = 0;

        /**
         * @brief Get list of all connected clients
         * @return Vector of client IDs currently connected
         */
        virtual std::vector<uint32_t> getConnectedClients() = 0;
};

#endif /* !ISERVER_HPP_ */
