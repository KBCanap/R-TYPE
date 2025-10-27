/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** PacketProcessor
*/

#include "../include/network/PacketProcessor.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream> // Pour std::cerr et std::cout

namespace network {

PacketProcessor::PacketProcessor()
    : next_send_sequence_(0), last_received_sequence_(0) {}

uint32_t PacketProcessor::getNextSendSequence() {
    return next_send_sequence_++;
}

void PacketProcessor::addPacket(const UDPPacket &packet) {
    processed_packets_.push(packet);
}

std::vector<UDPPacket> PacketProcessor::getProcessedPackets() {
    std::vector<UDPPacket> result;

    while (!processed_packets_.empty()) {
        result.push_back(processed_packets_.front());
        processed_packets_.pop();
    }

    return result;
}

TCPMessage PacketProcessor::parseTCPMessage(const std::vector<uint8_t> &data) {
    TCPMessage msg;

    if (data.size() < 4) {
        msg.msg_type = MessageType::TCP_ERROR;
        msg.data_length = 0;
        return msg;
    }

    msg.msg_type = static_cast<MessageType>(data[0]);

    msg.data_length = 0;
    msg.data_length |= static_cast<uint32_t>(data[1]) << 16;
    msg.data_length |= static_cast<uint32_t>(data[2]) << 8;
    msg.data_length |= static_cast<uint32_t>(data[3]);

    if (data.size() > 4) {
        msg.data.assign(data.begin() + 4, data.end());
    }

    return msg;
}

std::vector<uint8_t>
PacketProcessor::serializeTCPMessage(const TCPMessage &msg) {
    std::vector<uint8_t> result;
    result.reserve(4 + msg.data.size());

    result.push_back(static_cast<uint8_t>(msg.msg_type));

    result.push_back((msg.data_length >> 16) & 0xFF);
    result.push_back((msg.data_length >> 8) & 0xFF);
    result.push_back(msg.data_length & 0xFF);

    result.insert(result.end(), msg.data.begin(), msg.data.end());

    return result;
}

UDPPacket PacketProcessor::parseUDPPacket(const std::vector<uint8_t> &data) {
    UDPPacket packet;

    if (data.size() < 8) {
        packet.msg_type = static_cast<UDPMessageType>(0xFF);
        packet.data_length = 0;
        packet.sequence_num = 0;
        return packet;
    }

    packet.msg_type = static_cast<UDPMessageType>(data[0]);

    packet.data_length = 0;
    packet.data_length |= static_cast<uint32_t>(data[1]) << 16;
    packet.data_length |= static_cast<uint32_t>(data[2]) << 8;
    packet.data_length |= static_cast<uint32_t>(data[3]);

    packet.sequence_num = 0;
    packet.sequence_num |= static_cast<uint32_t>(data[4]) << 24;
    packet.sequence_num |= static_cast<uint32_t>(data[5]) << 16;
    packet.sequence_num |= static_cast<uint32_t>(data[6]) << 8;
    packet.sequence_num |= static_cast<uint32_t>(data[7]);

    if (data.size() > 8) {
        packet.payload.assign(data.begin() + 8, data.end());
    }

    return packet;
}

std::vector<uint8_t>
PacketProcessor::serializeUDPPacket(const UDPPacket &packet) {
    std::vector<uint8_t> result;
    result.reserve(8 + packet.payload.size());

    result.push_back(static_cast<uint8_t>(packet.msg_type));

    result.push_back((packet.data_length >> 16) & 0xFF);
    result.push_back((packet.data_length >> 8) & 0xFF);
    result.push_back(packet.data_length & 0xFF);

    result.push_back((packet.sequence_num >> 24) & 0xFF);
    result.push_back((packet.sequence_num >> 16) & 0xFF);
    result.push_back((packet.sequence_num >> 8) & 0xFF);
    result.push_back(packet.sequence_num & 0xFF);

    result.insert(result.end(), packet.payload.begin(), packet.payload.end());

    return result;
}

EntityData
PacketProcessor::parseEntityCreate(const std::vector<uint8_t> &data) {
    EntityData entity;

    if (data.size() < 17) {
        return entity;
    }

    size_t offset = 0;

    uint32_t net_id_network;
    std::memcpy(&net_id_network, &data[offset], 4);
    entity.net_id = ntohl(net_id_network);
    offset += 4;

    entity.entity_type = static_cast<EntityType>(data[offset]);
    offset += 1;

    uint32_t health_network;
    std::memcpy(&health_network, &data[offset], 4);
    entity.health = ntohl(health_network);
    offset += 4;

    uint32_t pos_x_network;
    std::memcpy(&pos_x_network, &data[offset], 4);
    entity.position_x = networkToFloat(pos_x_network);
    offset += 4;

    uint32_t pos_y_network;
    std::memcpy(&pos_y_network, &data[offset], 4);
    entity.position_y = networkToFloat(pos_y_network);

    return entity;
}

std::vector<EntityUpdateData>
PacketProcessor::parseEntityUpdate(const std::vector<uint8_t> &data) {
    std::vector<EntityUpdateData> updates;

    if (data.size() % 16 != 0) {
        return updates;
    }

    size_t num_entities = data.size() / 16;

    for (size_t i = 0; i < num_entities; ++i) {
        size_t offset = i * 16;
        EntityUpdateData update;

        uint32_t net_id_network;
        std::memcpy(&net_id_network, &data[offset], 4);
        update.net_id = ntohl(net_id_network);
        offset += 4;

        uint32_t health_network;
        std::memcpy(&health_network, &data[offset], 4);
        update.health = ntohl(health_network);
        offset += 4;

        uint32_t pos_x_network;
        std::memcpy(&pos_x_network, &data[offset], 4);
        update.position_x = networkToFloat(pos_x_network);
        offset += 4;

        uint32_t pos_y_network;
        std::memcpy(&pos_y_network, &data[offset], 4);
        update.position_y = networkToFloat(pos_y_network);

        updates.push_back(update);
    }

    return updates;
}

std::vector<uint32_t>
PacketProcessor::parseEntityDestroy(const std::vector<uint8_t> &data) {
    std::vector<uint32_t> net_ids;

    if (data.size() % 4 != 0) {
        return net_ids;
    }

    size_t num_entities = data.size() / 4;

    for (size_t i = 0; i < num_entities; ++i) {
        size_t offset = i * 4;
        uint32_t net_id = (static_cast<uint32_t>(data[offset]) << 24) |
                          (static_cast<uint32_t>(data[offset + 1]) << 16) |
                          (static_cast<uint32_t>(data[offset + 2]) << 8) |
                          static_cast<uint32_t>(data[offset + 3]);
        net_ids.push_back(net_id);
    }

    return net_ids;
}

std::vector<EntityData>
PacketProcessor::parseGameState(const std::vector<uint8_t> &data) {
    std::vector<EntityData> entities;

    if (data.size() < 4) {
        return entities;
    }

    uint32_t entity_count = (static_cast<uint32_t>(data[0]) << 24) |
                            (static_cast<uint32_t>(data[1]) << 16) |
                            (static_cast<uint32_t>(data[2]) << 8) |
                            static_cast<uint32_t>(data[3]);

    if (data.size() < 4 + entity_count * 17) {
        return entities;
    }

    for (uint32_t i = 0; i < entity_count; ++i) {
        size_t offset = 4 + i * 17;
        std::vector<uint8_t> entity_data(data.begin() + offset,
                                         data.begin() + offset + 17);
        EntityData entity = parseEntityCreate(entity_data);
        entities.push_back(entity);
    }

    return entities;
}

uint32_t
PacketProcessor::parsePlayerAssignment(const std::vector<uint8_t> &data) {
    if (data.size() < 4) {
        return 0;
    }

    uint32_t player_net_id = (static_cast<uint32_t>(data[0]) << 24) |
                             (static_cast<uint32_t>(data[1]) << 16) |
                             (static_cast<uint32_t>(data[2]) << 8) |
                             static_cast<uint32_t>(data[3]);

    return player_net_id;
}

std::vector<uint8_t>
PacketProcessor::serializePlayerInput(const PlayerInputData &input) {
    std::vector<uint8_t> data;
    data.reserve(2);

    data.push_back(static_cast<uint8_t>(input.event_type));
    data.push_back(static_cast<uint8_t>(input.direction));

    return data;
}

UDPPacket PacketProcessor::createClientPing(uint32_t timestamp,
                                            uint8_t player_id) {
    (void)player_id;

    UDPPacket packet;
    packet.msg_type = UDPMessageType::CLIENT_PING;
    packet.data_length = 4;
    packet.sequence_num = 0;

    packet.payload.clear();
    packet.payload.reserve(4);
    packet.payload.push_back((timestamp >> 24) & 0xFF);
    packet.payload.push_back((timestamp >> 16) & 0xFF);
    packet.payload.push_back((timestamp >> 8) & 0xFF);
    packet.payload.push_back(timestamp & 0xFF);

    return packet;
}

uint32_t PacketProcessor::floatToNetwork(float value) {
    uint32_t network_value;
    std::memcpy(&network_value, &value, sizeof(float));
    return htonl(network_value);
}

float PacketProcessor::networkToFloat(uint32_t value) {
    uint32_t host_value = ntohl(value);
    float result;
    std::memcpy(&result, &host_value, sizeof(float));
    return result;
}

} // namespace network