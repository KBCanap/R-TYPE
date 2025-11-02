#include "../include/network/NetworkManager.hpp"
#include "../include/network/ASIOSocket.hpp"
#include <cstring>
#include <iostream>

namespace network {

NetworkManager::NetworkManager()
    : ANetworkManager(std::make_unique<ASIOContext>()) {}

std::vector<TCPMessage> NetworkManager::pollTCP() {
    auto raw_messages = pollRawTCP();
    std::vector<TCPMessage> processed_messages;

    for (const auto &raw_msg : raw_messages) {
        TCPMessage msg = PacketProcessor::parseTCPMessage(raw_msg.data);
        processed_messages.push_back(msg);
    }

    return processed_messages;
}

std::vector<UDPPacket> NetworkManager::pollUDP() {
    auto raw_packets = pollRawUDP();

    for (const auto &raw_packet : raw_packets) {
        UDPPacket packet = PacketProcessor::parseUDPPacket(raw_packet.data);

        if (packet.msg_type == UDPMessageType::PLAYER_ASSIGNMENT) {
            handlePlayerAssignment(packet);
        }

        packet_processor_.addPacket(packet);
    }

    auto processed_packets = packet_processor_.getProcessedPackets();

    return processed_packets;
}

bool NetworkManager::sendTCP(MessageType msg_type,
                             const std::vector<uint8_t> &data) {
    TCPMessage msg;
    msg.msg_type = msg_type;
    msg.data_length = static_cast<uint32_t>(data.size());
    msg.data = data;

    auto serialized = PacketProcessor::serializeTCPMessage(msg);
    return sendRawTCP(serialized);
}

bool NetworkManager::sendUDP(const UDPPacket &packet) {
    auto serialized = PacketProcessor::serializeUDPPacket(packet);
    return sendRawUDP(serialized);
}

bool NetworkManager::sendPlayerInput(const PlayerInputData &input) {
    UDPPacket packet;
    packet.msg_type = UDPMessageType::PLAYER_INPUT;
    packet.sequence_num = packet_processor_.getNextSendSequence();
    packet.payload = PacketProcessor::serializePlayerInput(input);
    packet.data_length = static_cast<uint32_t>(packet.payload.size());

    return sendUDP(packet);
}

bool NetworkManager::sendClientPing(uint32_t timestamp) {
    uint8_t player_id = getPlayerID();

    UDPPacket ping_packet =
        PacketProcessor::createClientPing(timestamp, player_id);
    ping_packet.sequence_num = packet_processor_.getNextSendSequence();
    return sendUDP(ping_packet);
}

void NetworkManager::handlePlayerAssignment(const UDPPacket &packet) {
    if (packet.payload.size() != 4) {
        return;
    }

    uint32_t net_id = PacketProcessor::parsePlayerAssignment(packet.payload);

    {
        std::lock_guard<std::mutex> lock(player_mutex_);
        assigned_player_net_id_ = net_id;
        player_assigned_ = true;
    }

    updateState(ConnectionState::IN_GAME);
}

void NetworkManager::initializeUDPSocket() {
    resetConnectionState();
    ANetworkManager::initializeUDPSocket();

    if (getConnectionState() == ConnectionState::ERROR) {
        return;
    }

    auto current_time = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());

    if (sendClientPing(current_time)) {
        udp_ping_sent_ = true;
        last_ping_time_ = std::chrono::steady_clock::now();
        ping_start_time_ = last_ping_time_;
        ping_retry_count_ = 0;
    } else {
        updateState(ConnectionState::ERROR);
    }
}

void NetworkManager::update(float dt) {
    ANetworkManager::update(dt);

    auto state = getConnectionState();
    auto now = std::chrono::steady_clock::now();

    if (state == ConnectionState::GAME_STARTING && udp_ping_sent_) {
        bool is_assigned;
        {
            std::lock_guard<std::mutex> lock(player_mutex_);
            is_assigned = player_assigned_;
        }

        if (is_assigned) {
            return;
        }

        auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                                 now - ping_start_time_)
                                 .count();

        if (total_elapsed >= PING_TOTAL_TIMEOUT_S) {
            updateState(ConnectionState::ERROR);
            disconnect();
            return;
        }

        auto retry_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                                 now - last_ping_time_)
                                 .count();

        if (retry_elapsed >= PING_RETRY_INTERVAL_S) {
            if (ping_retry_count_ < MAX_PING_RETRIES) {
                auto current_time = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count());

                if (sendClientPing(current_time)) {
                    ping_retry_count_++;
                    last_ping_time_ = now;
                } else {
                    updateState(ConnectionState::ERROR);
                    disconnect();
                }
            }
        }
    }
}

uint32_t NetworkManager::getAssignedPlayerNetId() const {
    std::lock_guard<std::mutex> lock(player_mutex_);
    return assigned_player_net_id_;
}

void NetworkManager::resetConnectionState() {
    std::lock_guard<std::mutex> lock(player_mutex_);
    assigned_player_net_id_ = 0;
    player_assigned_ = false;
    udp_ping_sent_ = false;
    ping_retry_count_ = 0;
}

void NetworkManager::sendPlayerFire() {
    UDPPacket packet;
    packet.msg_type = UDPMessageType::PLAYER_INPUT;
    packet.sequence_num = packet_processor_.getNextSendSequence();

    packet.payload = {0x02, 0x00};
    packet.data_length = 2;

    sendUDP(packet);
}

void NetworkManager::sendPlayerInput(uint8_t direction) {
    UDPPacket packet;
    packet.msg_type = UDPMessageType::PLAYER_INPUT;
    packet.sequence_num = packet_processor_.getNextSendSequence();
    packet.payload = {0x01, direction};
    packet.data_length = 2;

    sendUDP(packet);
}

} // namespace network
