/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ANetworkManager
*/

#include "../include/network/ANetworkManager.hpp"
#include <iostream>

namespace network {

ANetworkManager::ANetworkManager(std::unique_ptr<IIOContext> io_context)
    : io_context_(std::move(io_context)), tcp_socket_(nullptr),
      udp_socket_(nullptr), state_(ConnectionState::DISCONNECTED),
      player_id_(0), udp_port_(0), connection_timeout_s_(10.0f),
      ready_timeout_s_(30.0f), tcp_header_buffer_(4), tcp_payload_buffer_(2048),
      udp_read_buffer_(2048) {}

ANetworkManager::~ANetworkManager() { disconnect(); }

ConnectionResult ANetworkManager::connectTCP(const std::string &host,
                                             uint16_t port) {
    ConnectionResult result;
    result.success = false;
    result.player_id = 0;

    tcp_socket_.reset(io_context_->createTCPSocket());

    if (!tcp_socket_->connect(host, port)) {
        result.error_message = "Failed to connect TCP socket";
        return result;
    }

    server_host_ = host;
    updateState(ConnectionState::CONNECTING);
    connection_start_ = std::chrono::steady_clock::now();
    last_activity_ = connection_start_;

    startReadTCPHeader();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "Starting I/O thread..." << std::endl;
    io_thread_ = std::thread([this]() {
        try {
            io_context_->run();
        } catch (const std::exception &e) {
            std::cerr << "I/O thread exception: " << e.what() << std::endl;
            updateState(ConnectionState::ERROR);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::cout << "Sending TCP_CONNECT..." << std::endl;
    std::vector<uint8_t> connect_msg = {0x01, 0x00, 0x00, 0x00};
    if (!sendRawTCP(connect_msg)) {
        result.error_message = "Failed to send TCP_CONNECT";
        disconnect();
        return result;
    }

    std::cout << "TCP_CONNECT sent, waiting for response..." << std::endl;
    result.success = true;
    return result;
}

void ANetworkManager::startReadTCPHeader() {
    if (!tcp_socket_ || !tcp_socket_->isOpen()) {
        return;
    }

    tcp_socket_->asyncReadExactly(
        tcp_header_buffer_, 4, [this](bool success, size_t bytes_transferred) {
            auto current_state = getConnectionState();
            if (current_state == ConnectionState::IN_GAME ||
                current_state == ConnectionState::DISCONNECTED) {
                return;
            }

            if (!success || bytes_transferred != 4) {
                if (current_state != ConnectionState::DISCONNECTED &&
                    current_state != ConnectionState::ERROR) {
                    std::cerr << "Failed to read TCP header" << std::endl;
                    updateState(ConnectionState::ERROR);
                }
                return;
            }

            uint8_t msg_type = tcp_header_buffer_[0];

            uint32_t data_length = 0;
            data_length |= static_cast<uint32_t>(tcp_header_buffer_[1]) << 16;
            data_length |= static_cast<uint32_t>(tcp_header_buffer_[2]) << 8;
            data_length |= static_cast<uint32_t>(tcp_header_buffer_[3]);

            last_activity_ = std::chrono::steady_clock::now();

            if (data_length == 0) {
                std::vector<uint8_t> complete_msg = tcp_header_buffer_;
                processCompleteTCPMessage(complete_msg);
                startReadTCPHeader();
            } else {
                readTCPPayload(msg_type, data_length);
            }
        });
}

void ANetworkManager::readTCPPayload(uint8_t msg_type, uint32_t data_length) {
    const uint32_t MAX_PAYLOAD_SIZE = 65536;

    if (data_length > MAX_PAYLOAD_SIZE) {
        std::cerr << "TCP payload too large: " << data_length << " bytes"
                  << std::endl;
        updateState(ConnectionState::ERROR);
        return;
    }

    if (!tcp_socket_ || !tcp_socket_->isOpen()) {
        return;
    }

    if (tcp_payload_buffer_.size() < data_length) {
        tcp_payload_buffer_.resize(data_length);
    }

    tcp_socket_->asyncReadExactly(
        tcp_payload_buffer_, data_length,
        [this, msg_type, data_length](bool success, size_t bytes_transferred) {
            if (!success || bytes_transferred != data_length) {
                std::cerr << "Failed to read TCP payload" << std::endl;
                updateState(ConnectionState::ERROR);
                return;
            }

            std::vector<uint8_t> complete_msg;
            complete_msg.reserve(4 + data_length);
            complete_msg.insert(complete_msg.end(), tcp_header_buffer_.begin(),
                                tcp_header_buffer_.end());
            complete_msg.insert(complete_msg.end(), tcp_payload_buffer_.begin(),
                                tcp_payload_buffer_.begin() + data_length);

            processCompleteTCPMessage(complete_msg);
            startReadTCPHeader();
        });
}

void ANetworkManager::processCompleteTCPMessage(
    const std::vector<uint8_t> &complete_msg) {
    if (complete_msg.size() < 4) {
        return;
    }

    uint8_t msg_type = complete_msg[0];

    switch (static_cast<MessageType>(msg_type)) {
    case MessageType::TCP_CONNECT_ACK:
        if (complete_msg.size() >= 5) {
            player_id_ = complete_msg[4];
            updateState(ConnectionState::CONNECTED);
            std::cout << "TCP_CONNECT_ACK received, Player ID: "
                      << static_cast<int>(player_id_) << std::endl;
        } else {
            std::cerr << "Invalid TCP_CONNECT_ACK size" << std::endl;
        }
        break;

    case MessageType::TCP_CONNECT_NAK:
        if (complete_msg.size() >= 5) {
            uint8_t error_code = complete_msg[4];
            std::cerr << "TCP_CONNECT_NAK received, error: "
                      << static_cast<int>(error_code) << std::endl;
        } else {
            std::cerr << "TCP_CONNECT_NAK received" << std::endl;
        }
        updateState(ConnectionState::ERROR);
        break;

    case MessageType::TCP_GAME_START:
        if (complete_msg.size() >= 6) {
            udp_port_ =
                (static_cast<uint16_t>(complete_msg[4]) << 8) | complete_msg[5];
            std::cout << "TCP_GAME_START received, UDP Port: " << udp_port_
                      << std::endl;
            updateState(ConnectionState::GAME_STARTING);
            initializeUDPSocket();
        } else {
            std::cerr << "Invalid TCP_GAME_START size" << std::endl;
        }
        break;

    case MessageType::TCP_ERROR:
        if (complete_msg.size() >= 5) {
            uint8_t error_code = complete_msg[4];
            std::cerr << "TCP_ERROR received, code: "
                      << static_cast<int>(error_code) << std::endl;
        } else {
            std::cerr << "TCP_ERROR received" << std::endl;
        }
        updateState(ConnectionState::ERROR);
        break;

    default:
        std::cerr << "Unknown TCP message type: " << static_cast<int>(msg_type)
                  << std::endl;
        break;
    }

    RawTCPMessage raw_msg;
    raw_msg.data = complete_msg;
    raw_msg.timestamp = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(tcp_queue_mutex_);
    raw_tcp_queue_.push(raw_msg);
}

void ANetworkManager::disconnect() {
    updateState(ConnectionState::DISCONNECTED);

    if (tcp_socket_ && tcp_socket_->isOpen()) {
        tcp_socket_->disconnect();
    }

    if (udp_socket_ && udp_socket_->isOpen()) {
        udp_socket_->close();
    }

    if (io_context_) {
        io_context_->stop();
    }

    if (io_thread_.joinable()) {
        io_thread_.join();
    }

    tcp_socket_.reset();
    udp_socket_.reset();

    player_id_ = 0;
    udp_port_ = 0;
}

ConnectionState ANetworkManager::getConnectionState() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_;
}

bool ANetworkManager::isConnected() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_ != ConnectionState::DISCONNECTED &&
           state_ != ConnectionState::ERROR;
}

bool ANetworkManager::sendTCP(MessageType msg_type,
                              const std::vector<uint8_t> &data) {
    return false;
}

bool ANetworkManager::sendUDP(const UDPPacket &packet) {
    return false;
}

bool ANetworkManager::sendPlayerInput(const PlayerInputData &input) {
    return false;
}

std::vector<TCPMessage> ANetworkManager::pollTCP() {
    return {};
}

std::vector<UDPPacket> ANetworkManager::pollUDP() {
    return {};
}

bool ANetworkManager::hasMessages() const {
    std::lock_guard<std::mutex> lock(tcp_queue_mutex_);
    return !raw_tcp_queue_.empty();
}

uint8_t ANetworkManager::getPlayerID() const { return player_id_; }

uint16_t ANetworkManager::getUDPPort() const { return udp_port_; }

float ANetworkManager::getLatency() const { return 0.0f; }

void ANetworkManager::update(float dt) { checkTimeout(); }

bool ANetworkManager::sendRawTCP(const std::vector<uint8_t> &data) {
    if (!tcp_socket_ || !tcp_socket_->isOpen()) {
        return false;
    }

    size_t sent = tcp_socket_->send(data);
    if (sent > 0) {
        last_activity_ = std::chrono::steady_clock::now();
        return true;
    }

    updateState(ConnectionState::ERROR);
    return false;
}

bool ANetworkManager::sendRawUDP(const std::vector<uint8_t> &data) {
    if (!udp_socket_ || !udp_socket_->isOpen()) {
        return false;
    }

    size_t sent = udp_socket_->sendTo(data);
    return sent > 0;
}

std::vector<RawTCPMessage> ANetworkManager::pollRawTCP() {
    std::lock_guard<std::mutex> lock(tcp_queue_mutex_);
    std::vector<RawTCPMessage> messages;

    while (!raw_tcp_queue_.empty()) {
        messages.push_back(raw_tcp_queue_.front());
        raw_tcp_queue_.pop();
    }

    return messages;
}

std::vector<RawUDPPacket> ANetworkManager::pollRawUDP() {
    std::lock_guard<std::mutex> lock(udp_queue_mutex_);
    std::vector<RawUDPPacket> packets;

    while (!raw_udp_queue_.empty()) {
        packets.push_back(raw_udp_queue_.front());
        raw_udp_queue_.pop();
    }

    return packets;
}

void ANetworkManager::updateState(ConnectionState new_state) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (state_ != new_state) {
        state_ = new_state;
    }
}

void ANetworkManager::checkTimeout() {
    auto now = std::chrono::steady_clock::now();
    auto state = getConnectionState();

    if (state == ConnectionState::CONNECTING) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - connection_start_)
                           .count();

        if (elapsed >= connection_timeout_s_) {
            std::cerr << "Connection timeout" << std::endl;
            updateState(ConnectionState::ERROR);
            disconnect();
        }
    }

    if (state == ConnectionState::WAITING) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - last_activity_)
                           .count();

        if (elapsed >= ready_timeout_s_) {
            std::cerr << "Ready timeout" << std::endl;
            updateState(ConnectionState::ERROR);
            disconnect();
        }
    }
}

void ANetworkManager::initializeUDPSocket() {
    udp_socket_.reset(io_context_->createUDPSocket());

    if (!udp_socket_->open()) {
        std::cerr << "Failed to open UDP socket" << std::endl;
        updateState(ConnectionState::ERROR);
        return;
    }

    if (!udp_socket_->setRemoteEndpoint(server_host_, udp_port_)) {
        std::cerr << "Failed to set UDP remote endpoint" << std::endl;
        updateState(ConnectionState::ERROR);
        return;
    }

    std::cout << "UDP socket initialized" << std::endl;
    startAsyncUDPReceive();
}

void ANetworkManager::startAsyncUDPReceive() {
    if (!udp_socket_ || !udp_socket_->isOpen()) {
        return;
    }

    udp_socket_->asyncReceive(
        udp_read_buffer_, [this](bool success, size_t bytes_transferred) {
            auto current_state = getConnectionState();
            if (current_state == ConnectionState::DISCONNECTED ||
                current_state == ConnectionState::ERROR) {
                return;
            }

            if (success && bytes_transferred > 0) {
                std::vector<uint8_t> data(udp_read_buffer_.begin(),
                                          udp_read_buffer_.begin() +
                                              bytes_transferred);

                RawUDPPacket raw_packet;
                raw_packet.data = data;
                raw_packet.timestamp = std::chrono::steady_clock::now();

                {
                    std::lock_guard<std::mutex> lock(udp_queue_mutex_);
                    raw_udp_queue_.push(raw_packet);
                }
            }

            if (current_state != ConnectionState::DISCONNECTED &&
                current_state != ConnectionState::ERROR) {
                startAsyncUDPReceive();
            }
        });
}

} // namespace network