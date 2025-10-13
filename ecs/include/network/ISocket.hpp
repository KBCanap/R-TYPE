#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace network {

/**
 * @class ITCPSocket
 * @brief Abstract TCP socket interface
 */
class ITCPSocket {
  public:
    virtual ~ITCPSocket() = default;

    /**
     * @brief Connect to remote host
     * @param host Hostname or IP address
     * @param port Port number
     * @return true if connected successfully
     */
    virtual bool connect(const std::string &host, uint16_t port) = 0;

    /**
     * @brief Disconnect socket
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if socket is open
     * @return true if connected
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Send data synchronously
     * @param data Data to send
     * @return Number of bytes sent
     */
    virtual size_t send(const std::vector<uint8_t> &data) = 0;

    /**
     * @brief Start asynchronous read operation (read what's available)
     * @param buffer Buffer to read into
     * @param callback Callback when data arrives
     */
    virtual void asyncRead(std::vector<uint8_t> &buffer,
                           std::function<void(bool, size_t)> callback) = 0;

    /**
     * @brief Read exactly N bytes asynchronously
     * @param buffer Buffer to read into (must be at least size N)
     * @param size Exact number of bytes to read
     * @param callback Callback when complete
     */
    virtual void
    asyncReadExactly(std::vector<uint8_t> &buffer, size_t size,
                     std::function<void(bool, size_t)> callback) = 0;
};

/**
 * @class IUDPSocket
 * @brief Abstract UDP socket interface
 */
class IUDPSocket {
  public:
    virtual ~IUDPSocket() = default;

    /**
     * @brief Open UDP socket
     * @return true if opened successfully
     */
    virtual bool open() = 0;

    /**
     * @brief Close UDP socket
     */
    virtual void close() = 0;

    /**
     * @brief Check if socket is open
     * @return true if open
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Set remote endpoint
     * @param host Remote hostname or IP
     * @param port Remote port
     * @return true if endpoint set successfully
     */
    virtual bool setRemoteEndpoint(const std::string &host, uint16_t port) = 0;

    /**
     * @brief Send data to remote endpoint
     * @param data Data to send
     * @return Number of bytes sent
     */
    virtual size_t sendTo(const std::vector<uint8_t> &data) = 0;

    /**
     * @brief Start asynchronous receive operation
     * @param buffer Buffer to receive into
     * @param callback Callback when data arrives
     */
    virtual void asyncReceive(std::vector<uint8_t> &buffer,
                              std::function<void(bool, size_t)> callback) = 0;
};

/**
 * @class IIOContext
 * @brief Abstract I/O context interface
 */
class IIOContext {
  public:
    virtual ~IIOContext() = default;

    /**
     * @brief Run the I/O context (blocking)
     */
    virtual void run() = 0;

    /**
     * @brief Stop the I/O context
     */
    virtual void stop() = 0;

    /**
     * @brief Create a TCP socket
     * @return Unique pointer to TCP socket
     */
    virtual ITCPSocket *createTCPSocket() = 0;

    /**
     * @brief Create a UDP socket
     * @return Unique pointer to UDP socket
     */
    virtual IUDPSocket *createUDPSocket() = 0;

    /**
     * @brief Post a task to the I/O context
     * @param task The task to be executed
     */
    virtual void post(std::function<void()> task) = 0;
};

} // namespace network