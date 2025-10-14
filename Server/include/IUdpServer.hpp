/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** IUdpServer - Interface for UDP server implementations
*/

#ifndef IUDPSERVER_HPP_
#define IUDPSERVER_HPP_

#include <cstdint>
#include <string>
#include <vector>

struct UdpClientMessage {
    std::string client_endpoint;
    std::string message;
    uint32_t client_id;
};

/**
 * @brief Interface for UDP server implementations
 *
 * This interface defines the contract that all UDP server implementations must
 * follow. Unlike TCP, UDP is connectionless, so client management is based on
 * endpoints.
 */
class IUdpServer {
  public:
    virtual ~IUdpServer() = default;

    /**
     * @brief Poll for incoming messages from clients
     * @return Vector of UdpClientMessage containing all pending messages
     */
    virtual std::vector<UdpClientMessage> poll() = 0;

    /**
     * @brief Send a message to a specific client endpoint
     * @param client_id The unique identifier of the client
     * @param message The message to send
     * @return true if the message was sent successfully, false otherwise
     */
    virtual bool sendToClient(uint32_t client_id,
                              const std::string &message) = 0;

    /**
     * @brief Remove a client from the known clients list
     * @param client_id The unique identifier of the client to remove
     */
    virtual void disconnectClient(uint32_t client_id) = 0;

    /**
     * @brief Get list of all known clients
     * @return Vector of client IDs currently tracked
     */
    virtual std::vector<uint32_t> getConnectedClients() = 0;
};

#endif /* !IUDPSERVER_HPP_ */
