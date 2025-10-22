#pragma once
#include "INetwork.hpp"
#include "ISocket.hpp"
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace network {

/**
 * @struct RawTCPMessage
 * @brief Raw TCP message without processing
 */
struct RawTCPMessage {
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * @struct RawUDPPacket
 * @brief Raw UDP packet without processing
 */
struct RawUDPPacket {
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * @class ANetworkManager
 * @brief Abstract network manager with basic socket operations only
 */
class ANetworkManager : public INetwork {
  public:
    /**
     * @brief Construct network manager with I/O context
     * @param io_context Abstract I/O context
     */
    explicit ANetworkManager(std::unique_ptr<IIOContext> io_context);

    /**
     * @brief Destruct network manager
     */
    virtual ~ANetworkManager();

    ConnectionResult connectTCP(const std::string &host, uint16_t port,
                                const std::string &username = "Player") override;
    void disconnect() override;
    ConnectionState getConnectionState() const override;
    bool isConnected() const override;
    bool sendTCP(MessageType msg_type,
                 const std::vector<uint8_t> &data) override;
    bool sendUDP(const UDPPacket &packet) override;
    bool sendPlayerInput(const PlayerInputData &input) override;
    std::vector<TCPMessage> pollTCP() override;
    std::vector<UDPPacket> pollUDP() override;
    bool hasMessages() const override;
    uint8_t getPlayerID() const override;
    uint16_t getUDPPort() const override;
    float getLatency() const override;
    void update(float dt) override;

    /**
     * @brief Send raw TCP data
     * @param data Raw bytes to send
     * @return true if sent successfully
     */
    bool sendRawTCP(const std::vector<uint8_t> &data);

    /**
     * @brief Send raw UDP data
     * @param data Raw bytes to send
     * @return true if sent successfully
     */
    bool sendRawUDP(const std::vector<uint8_t> &data);

    /**
     * @brief Poll raw TCP messages
     * @return Vector of raw TCP messages
     */
    std::vector<RawTCPMessage> pollRawTCP();

    /**
     * @brief Poll raw UDP packets
     * @return Vector of raw UDP packets
     */
    std::vector<RawUDPPacket> pollRawUDP();

  protected:
    /**
     * @brief Update connection state machine
     * @param new_state New connection state
     */
    void updateState(ConnectionState new_state);

    /**
     * @brief Check connection timeout
     */
    void checkTimeout();

    /**
     * @brief Initialize UDP socket only (without sending ping)
     * Should be overridden by NetworkManager to send CLIENT_PING
     */
    virtual void initializeUDPSocket();

    /**
     * @brief Process complete TCP message after reading
     * @param complete_msg Complete message with header + data
     */
    void processCompleteTCPMessage(const std::vector<uint8_t> &complete_msg);

    /**
     * @brief Start reading TCP header (4 bytes)
     */
    void startReadTCPHeader();

    /**
     * @brief Read TCP payload after header is received
     * @param msg_type Message type from header
     * @param data_length Payload length from header
     */
    void readTCPPayload(uint8_t msg_type, uint32_t data_length);

    /**
     * @brief Start asynchronous UDP receive
     */
    void startAsyncUDPReceive();

    std::unique_ptr<IIOContext> io_context_;
    std::unique_ptr<ITCPSocket> tcp_socket_;
    std::unique_ptr<IUDPSocket> udp_socket_;
    std::thread io_thread_;

    mutable std::mutex state_mutex_;
    mutable std::mutex tcp_queue_mutex_;
    mutable std::mutex udp_queue_mutex_;

    ConnectionState state_;
    uint8_t player_id_;
    uint16_t udp_port_;
    std::string server_host_;

    std::queue<RawTCPMessage> raw_tcp_queue_;
    std::queue<RawUDPPacket> raw_udp_queue_;

    std::vector<uint8_t> tcp_header_buffer_;
    std::vector<uint8_t> tcp_payload_buffer_;
    std::vector<uint8_t> udp_read_buffer_;

    std::chrono::steady_clock::time_point last_activity_;
    std::chrono::steady_clock::time_point connection_start_;
    float connection_timeout_s_;
    float ready_timeout_s_;
};

} // namespace network