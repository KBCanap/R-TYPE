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

        // PLAYER_ASSIGNMENT is handled separately
        if (packet.msg_type == UDPMessageType::PLAYER_ASSIGNMENT) {
            handlePlayerAssignment(packet);
            continue;
        }

        packet_processor_.addPacket(packet);
    }

    return packet_processor_.getProcessedPackets();
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
        std::cerr << "Invalid PLAYER_ASSIGNMENT size: " << packet.payload.size()
                  << std::endl;
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
        std::cerr << "Failed to send CLIENT_PING" << std::endl;
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
            std::cerr << "PLAYER_ASSIGNMENT timeout after "
                      << PING_TOTAL_TIMEOUT_S << "s" << std::endl;
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
                    std::cerr << "Failed to retry CLIENT_PING" << std::endl;
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

void NetworkManager::sendPlayerInput(uint8_t direction) {
    UDPPacket packet;
    packet.msg_type = UDPMessageType::PLAYER_INPUT;
    packet.sequence_num = packet_processor_.getNextSendSequence();
    packet.payload = {0x00, direction};
    packet.data_length = 2;

    sendUDP(packet);
}

void NetworkManager::processMessages() {
    auto tcp_messages = pollTCP();
    (void)tcp_messages;

    auto udp_packets = pollUDP();
    for (const auto &packet : udp_packets) {
        switch (packet.msg_type) {
        case UDPMessageType::ENTITY_CREATE:
            if (packet.payload.size() >= 17) {
                uint32_t net_id =
                    (static_cast<uint32_t>(packet.payload[0]) << 24) |
                    (static_cast<uint32_t>(packet.payload[1]) << 16) |
                    (static_cast<uint32_t>(packet.payload[2]) << 8) |
                    static_cast<uint32_t>(packet.payload[3]);

                uint8_t type = packet.payload[4];

                uint32_t health =
                    (static_cast<uint32_t>(packet.payload[5]) << 24) |
                    (static_cast<uint32_t>(packet.payload[6]) << 16) |
                    (static_cast<uint32_t>(packet.payload[7]) << 8) |
                    static_cast<uint32_t>(packet.payload[8]);

                uint32_t pos_x_bits =
                    (static_cast<uint32_t>(packet.payload[9]) << 24) |
                    (static_cast<uint32_t>(packet.payload[10]) << 16) |
                    (static_cast<uint32_t>(packet.payload[11]) << 8) |
                    static_cast<uint32_t>(packet.payload[12]);
                float pos_x;
                std::memcpy(&pos_x, &pos_x_bits, sizeof(float));

                uint32_t pos_y_bits =
                    (static_cast<uint32_t>(packet.payload[13]) << 24) |
                    (static_cast<uint32_t>(packet.payload[14]) << 16) |
                    (static_cast<uint32_t>(packet.payload[15]) << 8) |
                    static_cast<uint32_t>(packet.payload[16]);
                float pos_y;
                std::memcpy(&pos_y, &pos_y_bits, sizeof(float));

                NetworkEntity entity;
                entity.type = type;
                entity.pos_x = pos_x;
                entity.pos_y = pos_y;
                entity.health = health;

                network_entities_[net_id] = entity;
            }
            break;

        case UDPMessageType::ENTITY_UPDATE:
            for (size_t i = 0; i + 16 <= packet.payload.size(); i += 16) {
                uint32_t net_id =
                    (static_cast<uint32_t>(packet.payload[i]) << 24) |
                    (static_cast<uint32_t>(packet.payload[i + 1]) << 16) |
                    (static_cast<uint32_t>(packet.payload[i + 2]) << 8) |
                    static_cast<uint32_t>(packet.payload[i + 3]);

                uint32_t health =
                    (static_cast<uint32_t>(packet.payload[i + 4]) << 24) |
                    (static_cast<uint32_t>(packet.payload[i + 5]) << 16) |
                    (static_cast<uint32_t>(packet.payload[i + 6]) << 8) |
                    static_cast<uint32_t>(packet.payload[i + 7]);

                uint32_t pos_x_bits =
                    (static_cast<uint32_t>(packet.payload[i + 8]) << 24) |
                    (static_cast<uint32_t>(packet.payload[i + 9]) << 16) |
                    (static_cast<uint32_t>(packet.payload[i + 10]) << 8) |
                    static_cast<uint32_t>(packet.payload[i + 11]);
                float pos_x;
                std::memcpy(&pos_x, &pos_x_bits, sizeof(float));

                uint32_t pos_y_bits =
                    (static_cast<uint32_t>(packet.payload[i + 12]) << 24) |
                    (static_cast<uint32_t>(packet.payload[i + 13]) << 16) |
                    (static_cast<uint32_t>(packet.payload[i + 14]) << 8) |
                    static_cast<uint32_t>(packet.payload[i + 15]);
                float pos_y;
                std::memcpy(&pos_y, &pos_y_bits, sizeof(float));

                auto it = network_entities_.find(net_id);
                if (it != network_entities_.end()) {
                    it->second.pos_x = pos_x;
                    it->second.pos_y = pos_y;
                    it->second.health = health;
                }
            }
            break;

        case UDPMessageType::ENTITY_DESTROY:
            if (packet.payload.size() >= 4) {
                uint32_t net_id =
                    (static_cast<uint32_t>(packet.payload[0]) << 24) |
                    (static_cast<uint32_t>(packet.payload[1]) << 16) |
                    (static_cast<uint32_t>(packet.payload[2]) << 8) |
                    static_cast<uint32_t>(packet.payload[3]);

                network_entities_.erase(net_id);
            }
            break;

        case UDPMessageType::GAME_STATE:
            if (packet.payload.size() >= 4) {
                game_score_ = (static_cast<uint32_t>(packet.payload[0]) << 24) |
                              (static_cast<uint32_t>(packet.payload[1]) << 16) |
                              (static_cast<uint32_t>(packet.payload[2]) << 8) |
                              static_cast<uint32_t>(packet.payload[3]);
            }
            break;

        default:
            break;
        }
    }
}

} // namespace network
