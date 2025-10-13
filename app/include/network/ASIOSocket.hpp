#pragma once
#include "../../../ecs/include/network/ISocket.hpp"
#include <asio.hpp>
#include <memory>

namespace network {

/**
 * @class ASIOTCPSocket
 * @brief ASIO implementation of ITCPSocket
 */
class ASIOTCPSocket : public ITCPSocket {
  public:
    /**
     * @brief Construct ASIO TCP socket
     * @param io_context ASIO io_context reference
     */
    explicit ASIOTCPSocket(asio::io_context &io_context);
    ~ASIOTCPSocket() override;

    bool connect(const std::string &host, uint16_t port) override;
    void disconnect() override;
    bool isOpen() const override;
    size_t send(const std::vector<uint8_t> &data) override;
    void asyncRead(std::vector<uint8_t> &buffer,
                   std::function<void(bool, size_t)> callback) override;
    void asyncReadExactly(std::vector<uint8_t> &buffer, size_t size,
                          std::function<void(bool, size_t)> callback) override;

  private:
    asio::ip::tcp::socket socket_;
};

/**
 * @class ASIOUDPSocket
 * @brief ASIO implementation of IUDPSocket
 */
class ASIOUDPSocket : public IUDPSocket {
  public:
    /**
     * @brief Construct ASIO UDP socket
     * @param io_context ASIO io_context reference
     */
    explicit ASIOUDPSocket(asio::io_context &io_context);
    ~ASIOUDPSocket() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;
    bool setRemoteEndpoint(const std::string &host, uint16_t port) override;
    size_t sendTo(const std::vector<uint8_t> &data) override;
    void asyncReceive(std::vector<uint8_t> &buffer,
                      std::function<void(bool, size_t)> callback) override;

  private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
};

/**
 * @class ASIOContext
 * @brief ASIO implementation of IIOContext
 */
class ASIOContext : public IIOContext {
  public:
    /**
     * @brief Construct ASIO context
     */
    ASIOContext();
    ~ASIOContext() override;

    void run() override;
    void stop() override;
    ITCPSocket *createTCPSocket() override;
    IUDPSocket *createUDPSocket() override;

    /**
     * @brief Get reference to underlying io_context
     * @return Reference to asio::io_context
     */
    asio::io_context &getIOContext() { return io_context_; }

  private:
    asio::io_context io_context_;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>>
        work_guard_;
};

} // namespace network