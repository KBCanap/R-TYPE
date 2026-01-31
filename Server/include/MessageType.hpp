/*
** EPITECH PROJECT, 2025
** MessageTypes.hpp
** File description:
** Protocol message types and constants - Extended Lobby System
*/

#ifndef MESSAGETYPES_HPP_
#define MESSAGETYPES_HPP_

#include <cstdint>

// Message types
enum class MessageType : uint8_t {
    // Connection Messages
    TCP_CONNECT = 0x01,
    TCP_CONNECT_ACK = 0x02,
    TCP_CONNECT_NAK = 0x03,
    TCP_READY = 0x04,
    TCP_GAME_START = 0x05,

    // Lobby Navigation Messages
    LOBBY_LIST_REQUEST = 0x10,
    LOBBY_LIST_RESPONSE = 0x11,
    LOBBY_INFO_REQUEST = 0x12,
    LOBBY_INFO_RESPONSE = 0x13,

    // Join/Create Lobby
    CREATE_LOBBY = 0x14,
    CREATE_LOBBY_ACK = 0x15,
    JOIN_LOBBY = 0x16,
    JOIN_LOBBY_ACK = 0x17,
    JOIN_LOBBY_NAK = 0x18,
    LEAVE_LOBBY = 0x19,
    LEAVE_LOBBY_ACK = 0x1A,

    // Lobby Player Management
    PLAYER_JOINED = 0x1B,
    PLAYER_LEFT = 0x1C,

    // Game Session Management
    GAME_CANCELLED = 0x1D,

    // Errors
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
    LOBBY_FULL = 0x01,
    LOBBY_NOT_EXIST = 0x02,
    GAME_ALREADY_STARTED = 0x03,
    INVALID_USERNAME = 0x04,
    PROTOCOL_VIOLATION = 0x05,
    UNEXPECTED_MESSAGE = 0x06,
    TIMEOUT = 0x07,
    INTERNAL_SERVER_ERROR = 0x08,
    PLAYER_NOT_IN_LOBBY = 0x09,
    ALREADY_IN_LOBBY = 0x0A
};

// Lobby status
enum class LobbyStatus : uint8_t {
    WAITING = 0x00,
    READY = 0x01,
    IN_GAME = 0x02,
    CLOSING = 0x03
};

// Constants
static constexpr std::size_t HEADER_SIZE = 4;
static constexpr std::size_t MAX_PLAYERS = 4;
static constexpr std::size_t MIN_PLAYERS = 2;
static constexpr std::size_t MAX_USERNAME_LENGTH = 60;
static constexpr std::size_t MAX_LOBBY_NAME_LENGTH = 32;

// Protocol structures
struct TCPHeader {
    uint8_t msg_type;
    uint8_t data_length[3];
} __attribute__((packed));

struct PlayerInfo {
    uint8_t player_id;
    uint8_t ready_flag;
    uint16_t username_length;
    char username[MAX_USERNAME_LENGTH];
} __attribute__((packed));

struct LobbyInfo {
    uint16_t lobby_id;
    uint8_t player_count;
    uint8_t max_players;
    uint16_t lobby_name_length;
    char lobby_name[MAX_LOBBY_NAME_LENGTH];
    uint8_t status;
    uint8_t reserved[3];
} __attribute__((packed));

#endif /* !MESSAGETYPES_HPP_ */
