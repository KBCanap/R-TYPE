/*
** EPITECH PROJECT, 2025
** MessageTypes.hpp
** File description:
** Protocol message types and constants
*/

#ifndef MESSAGETYPES_HPP_
#define MESSAGETYPES_HPP_

#include <cstdint>

// Message types
enum class MessageType : uint8_t {
    TCP_CONNECT = 0x01,
    TCP_CONNECT_ACK = 0x02,
    TCP_CONNECT_NAK = 0x03,
    TCP_READY = 0x04,
    TCP_GAME_START = 0x05,
    TCP_ERROR = 0xFF
};

// Error codes for CONNECT_NAK
enum class ConnectError : uint8_t {
    GAME_FULL = 0x01,
    GAME_STARTED = 0x02,
    SERVER_ERROR = 0x03
};

// Error codes for TCP_ERROR
enum class ProtocolError : uint8_t {
    PROTOCOL_VIOLATION = 0x01,
    UNEXPECTED_MESSAGE = 0x02,
    TIMEOUT = 0x03
};

static constexpr std::size_t HEADER_SIZE = 4;
static constexpr std::size_t MAX_PLAYERS = 4;
static constexpr std::size_t MIN_PORT = 1024;
static constexpr std::size_t MAX_PORT = 65535;


struct TCPHeader {
    uint8_t msg_type;
    uint8_t data_length[3];
} __attribute__((packed)); // Assure qu'il n'y a pas de padding

#endif /* !MESSAGETYPES_HPP_ */
