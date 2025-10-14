/*
** EPITECH PROJECT, 2025
** UdpProtocole.cpp
** File description:
** UDP Protocol message handling implementation for R-Type
*/

#include "UdpProtocole.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

UdpProtocole::UdpProtocole()
{
}

ParsedUdpMessage UdpProtocole::parseMessage(const std::string& raw_message)
{
    ParsedUdpMessage result = {UdpMessageType::CLIENT_PING, 0, {}, false};

    if (raw_message.size() < UDP_HEADER_SIZE) {
        std::cerr << "UDP Message too short: " << raw_message.size() << " bytes" << std::endl;
        return result;
    }

    const uint8_t* data = reinterpret_cast<const uint8_t*>(raw_message.data());

    uint8_t msg_type = data[0];
    if (!isValidMessageType(msg_type)) {
        std::cerr << "Invalid UDP message type: " << static_cast<int>(msg_type) << std::endl;
        return result;
    }

    uint32_t data_length = extractDataLength(data);
    uint32_t sequence_num = extractSequenceNum(data);

    if (raw_message.size() != UDP_HEADER_SIZE + data_length) {
        std::cerr << "Invalid UDP message size. Expected: " << UDP_HEADER_SIZE + data_length
                  << ", Got: " << raw_message.size() << std::endl;
        return result;
    }

    UdpMessageType type = static_cast<UdpMessageType>(msg_type);
    if (!isValidDataLength(type, data_length)) {
        std::cerr << "Invalid data length for UDP message type" << std::endl;
        return result;
    }

    std::vector<uint8_t> payload;
    if (data_length > 0) {
        payload.assign(data + UDP_HEADER_SIZE, data + UDP_HEADER_SIZE + data_length);
    }

    result.type = type;
    result.sequence_num = sequence_num;
    result.data = payload;
    result.valid = true;

    return result;
}

std::string UdpProtocole::createPlayerAssignment(uint32_t player_net_id, uint32_t sequence_num)
{
    std::vector<uint8_t> data(4);
    uint32_t net_id_network = htonl(player_net_id);
    std::memcpy(data.data(), &net_id_network, 4);
    return createMessage(UdpMessageType::PLAYER_ASSIGNMENT, sequence_num, data);
}

std::string UdpProtocole::createEntityCreate(const Entity& entity, uint32_t sequence_num)
{
    std::vector<uint8_t> data(ENTITY_CREATE_SIZE);
    uint8_t* ptr = data.data();

    uint32_t net_id_network = htonl(entity.net_id);
    std::memcpy(ptr, &net_id_network, 4);
    ptr += 4;

    *ptr = static_cast<uint8_t>(entity.type);
    ptr += 1;

    uint32_t health_network = htonl(entity.health);
    std::memcpy(ptr, &health_network, 4);
    ptr += 4;

    uint32_t pos_x_network = htonf(entity.position_x);
    std::memcpy(ptr, &pos_x_network, 4);
    ptr += 4;

    uint32_t pos_y_network = htonf(entity.position_y);
    std::memcpy(ptr, &pos_y_network, 4);

    return createMessage(UdpMessageType::ENTITY_CREATE, sequence_num, data);
}

std::string UdpProtocole::createEntityUpdate(const std::vector<Entity>& entities, uint32_t sequence_num)
{
    std::vector<uint8_t> data(entities.size() * ENTITY_UPDATE_SIZE);
    uint8_t* ptr = data.data();

    for (const auto& entity : entities) {
        uint32_t net_id_network = htonl(entity.net_id);
        std::memcpy(ptr, &net_id_network, 4);
        ptr += 4;

        uint32_t health_network = htonl(entity.health);
        std::memcpy(ptr, &health_network, 4);
        ptr += 4;

        uint32_t pos_x_network = htonf(entity.position_x);
        std::memcpy(ptr, &pos_x_network, 4);
        ptr += 4;

        uint32_t pos_y_network = htonf(entity.position_y);
        std::memcpy(ptr, &pos_y_network, 4);
        ptr += 4;
    }

    return createMessage(UdpMessageType::ENTITY_UPDATE, sequence_num, data);
}

std::string UdpProtocole::createEntityDestroy(const std::vector<uint32_t>& net_ids, uint32_t sequence_num)
{
    std::vector<uint8_t> data(net_ids.size() * 4);
    uint8_t* ptr = data.data();

    for (uint32_t net_id : net_ids) {
        uint32_t net_id_network = htonl(net_id);
        std::memcpy(ptr, &net_id_network, 4);
        ptr += 4;
    }

    return createMessage(UdpMessageType::ENTITY_DESTROY, sequence_num, data);
}

std::string UdpProtocole::createGameState(const std::vector<Entity>& entities, uint32_t sequence_num)
{
    std::vector<uint8_t> data(4 + entities.size() * ENTITY_CREATE_SIZE);
    uint8_t* ptr = data.data();

    uint32_t entity_count = htonl(static_cast<uint32_t>(entities.size()));
    std::memcpy(ptr, &entity_count, 4);
    ptr += 4;

    for (const auto& entity : entities) {
        uint32_t net_id_network = htonl(entity.net_id);
        std::memcpy(ptr, &net_id_network, 4);
        ptr += 4;

        *ptr = static_cast<uint8_t>(entity.type);
        ptr += 1;

        uint32_t health_network = htonl(entity.health);
        std::memcpy(ptr, &health_network, 4);
        ptr += 4;

        uint32_t pos_x_network = htonf(entity.position_x);
        std::memcpy(ptr, &pos_x_network, 4);
        ptr += 4;

        uint32_t pos_y_network = htonf(entity.position_y);
        std::memcpy(ptr, &pos_y_network, 4);
        ptr += 4;
    }

    return createMessage(UdpMessageType::GAME_STATE, sequence_num, data);
}

uint32_t UdpProtocole::extractHealth24bit(const uint8_t* data)
{
    uint32_t health = 0;
    health |= (static_cast<uint32_t>(data[0]) << 16);
    health |= (static_cast<uint32_t>(data[1]) << 8);
    health |= static_cast<uint32_t>(data[2]);
    return health;
}

void UdpProtocole::packHealth24bit(uint8_t* dest, uint32_t health)
{
    dest[0] = static_cast<uint8_t>((health >> 16) & 0xFF);
    dest[1] = static_cast<uint8_t>((health >> 8) & 0xFF);
    dest[2] = static_cast<uint8_t>(health & 0xFF);
}

float UdpProtocole::ntohf(uint32_t netfloat)
{
    uint32_t hostlong = ntohl(netfloat);
    float result;
    std::memcpy(&result, &hostlong, sizeof(float));
    return result;
}

uint32_t UdpProtocole::htonf(float hostfloat)
{
    uint32_t temp;
    std::memcpy(&temp, &hostfloat, sizeof(float));
    return htonl(temp);
}

uint32_t UdpProtocole::extractDataLength(const uint8_t* header)
{
    uint32_t length = 0;
    length |= (static_cast<uint32_t>(header[1]) << 16);
    length |= (static_cast<uint32_t>(header[2]) << 8);
    length |= static_cast<uint32_t>(header[3]);
    return length;
}

uint32_t UdpProtocole::extractSequenceNum(const uint8_t* header)
{
    uint32_t seq_num;
    std::memcpy(&seq_num, header + 4, 4);
    return ntohl(seq_num);
}

std::string UdpProtocole::createMessage(UdpMessageType type, uint32_t sequence_num, const std::vector<uint8_t>& data)
{
    std::string message;
    message.reserve(UDP_HEADER_SIZE + data.size());

    // MSG_TYPE (1 byte)
    message.push_back(static_cast<uint8_t>(type));

    // DATA_LENGTH (3 bytes, big endian)
    uint32_t length = data.size();
    message.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
    message.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
    message.push_back(static_cast<uint8_t>(length & 0xFF));

    // SEQUENCE_NUM (4 bytes, network byte order)
    uint32_t seq_network = htonl(sequence_num);
    const uint8_t* seq_bytes = reinterpret_cast<const uint8_t*>(&seq_network);
    message.append(reinterpret_cast<const char*>(seq_bytes), 4);

    // DATA (variable length)
    message.insert(message.end(), data.begin(), data.end());

    return message;
}

bool UdpProtocole::isValidMessageType(uint8_t type)
{
    switch (static_cast<UdpMessageType>(type)) {
        case UdpMessageType::CLIENT_PING:
        case UdpMessageType::PLAYER_ASSIGNMENT:
        case UdpMessageType::ENTITY_CREATE:
        case UdpMessageType::ENTITY_UPDATE:
        case UdpMessageType::ENTITY_DESTROY:
        case UdpMessageType::GAME_STATE:
        case UdpMessageType::PLAYER_INPUT:
            return true;
        default:
            return false;
    }
}

bool UdpProtocole::isValidDataLength(UdpMessageType type, uint32_t length)
{
    switch (type) {
        case UdpMessageType::CLIENT_PING:
            return length == 4;
        case UdpMessageType::PLAYER_ASSIGNMENT:
            return length == 4;
        case UdpMessageType::ENTITY_CREATE:
            return length == ENTITY_CREATE_SIZE;
        case UdpMessageType::ENTITY_UPDATE:
            return length > 0 && (length % ENTITY_UPDATE_SIZE) == 0;
        case UdpMessageType::ENTITY_DESTROY:
            return length > 0 && (length % 4) == 0;
        case UdpMessageType::GAME_STATE:
            return length >= 4;
        case UdpMessageType::PLAYER_INPUT:
            return length == 2;
        default:
            return false;
    }
}
