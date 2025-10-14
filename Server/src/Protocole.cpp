/*
** EPITECH PROJECT, 2025
** Protocol.cpp
** File description:
** Protocol message handling implementation
*/

#include "Protocole.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

Protocol::Protocol() {}

ParsedMessage Protocol::parseMessage(const std::string &raw_message) {
    ParsedMessage result = {MessageType::TCP_ERROR, {}, false};

    if (raw_message.size() < HEADER_SIZE) {
        std::cerr << "Message too short: " << raw_message.size() << " bytes"
                  << std::endl;
        return result;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(raw_message.data());

    uint8_t msg_type = data[0];
    if (!isValidMessageType(msg_type)) {
        std::cerr << "Invalid message type: " << static_cast<int>(msg_type)
                  << std::endl;
        return result;
    }

    uint32_t data_length = extractDataLength(data);

    if (raw_message.size() != HEADER_SIZE + data_length) {
        std::cerr << "Invalid message size. Expected: "
                  << HEADER_SIZE + data_length
                  << ", Got: " << raw_message.size() << std::endl;
        return result;
    }

    MessageType type = static_cast<MessageType>(msg_type);
    if (!isValidDataLength(type, data_length)) {
        std::cerr << "Invalid data length for message type" << std::endl;
        return result;
    }

    std::vector<uint8_t> payload;
    if (data_length > 0) {
        payload.assign(data + HEADER_SIZE, data + HEADER_SIZE + data_length);
    }

    result.type = type;
    result.data = payload;
    result.valid = true;

    return result;
}

std::string Protocol::createConnectAck(uint8_t player_id) {
    std::vector<uint8_t> data = {player_id};
    return createMessage(MessageType::TCP_CONNECT_ACK, data);
}

std::string Protocol::createConnectNak(ConnectError error_code) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(error_code)};
    return createMessage(MessageType::TCP_CONNECT_NAK, data);
}

std::string Protocol::createGameStart(uint16_t udp_port) {
    uint16_t port_network = htons(udp_port);
    std::vector<uint8_t> data(2);
    std::memcpy(data.data(), &port_network, 2);
    return createMessage(MessageType::TCP_GAME_START, data);
}

std::string Protocol::createError(ProtocolError error_code) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(error_code)};
    return createMessage(MessageType::TCP_ERROR, data);
}

uint32_t Protocol::extractDataLength(const uint8_t *header) {
    uint32_t length = 0;
    length |= (static_cast<uint32_t>(header[1]) << 16);
    length |= (static_cast<uint32_t>(header[2]) << 8);
    length |= static_cast<uint32_t>(header[3]);
    return length;
}

std::string Protocol::createMessage(MessageType type,
                                    const std::vector<uint8_t> &data) {
    std::string message;
    message.reserve(HEADER_SIZE + data.size());

    message.push_back(static_cast<uint8_t>(type));

    uint32_t length = data.size();
    message.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
    message.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
    message.push_back(static_cast<uint8_t>(length & 0xFF));

    message.insert(message.end(), data.begin(), data.end());

    return message;
}

bool Protocol::isValidMessageType(uint8_t type) {
    switch (static_cast<MessageType>(type)) {
    case MessageType::TCP_CONNECT:
    case MessageType::TCP_CONNECT_ACK:
    case MessageType::TCP_CONNECT_NAK:
    case MessageType::TCP_READY:
    case MessageType::TCP_GAME_START:
    case MessageType::TCP_ERROR:
        return true;
    default:
        return false;
    }
}

bool Protocol::isValidDataLength(MessageType type, uint32_t length) {
    switch (type) {
    case MessageType::TCP_CONNECT:
    case MessageType::TCP_READY:
        return length == 0;
    case MessageType::TCP_CONNECT_ACK:
    case MessageType::TCP_CONNECT_NAK:
    case MessageType::TCP_ERROR:
        return length == 1;
    case MessageType::TCP_GAME_START:
        return length == 2;
    default:
        return false;
    }
}
