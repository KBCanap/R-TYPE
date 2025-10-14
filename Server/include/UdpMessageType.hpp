/*
** EPITECH PROJECT, 2025
** UdpMessageType.hpp
** File description:
** UDP Protocol message types and constants for R-Type
*/

#ifndef UDPMESSAGETYPE_HPP_
#define UDPMESSAGETYPE_HPP_

#include <cstdint>

// UDP Message types
enum UdpMessageType : uint8_t {
    CLIENT_PING = 0x00,
    PLAYER_ASSIGNMENT = 0x01,
    ENTITY_CREATE = 0x10,
    ENTITY_UPDATE = 0x11,
    ENTITY_DESTROY = 0x12,
    GAME_STATE = 0x13,
    PLAYER_INPUT = 0x20
};

// Entity types
enum EntityType : uint8_t {
    PLAYER = 0x01,
    ENEMY = 0x02,
    PROJECTILE = 0x03,
    ALLIED_PROJECTILE = 0x04
};

// Player input event types
enum EventType : uint8_t { MOVE = 0x01, SHOOT = 0x02, QUIT = 0x03 };

// Movement directions
enum Direction : uint8_t {
    NONE = 0x00,
    UP = 0x01,   // bit 0
    DOWN = 0x02, // bit 1
    LEFT = 0x04, // bit 2
    RIGHT = 0x08 // bit 3
};

// Constants
static constexpr std::size_t UDP_HEADER_SIZE = 8;
static constexpr std::size_t ENTITY_CREATE_SIZE = 17;
static constexpr std::size_t ENTITY_UPDATE_SIZE = 16;
static constexpr std::size_t ENTITY_DESTROY_SIZE = 4;

// UDP Header structure
struct UdpHeader {
    uint8_t msg_type;
    uint8_t data_length[3];
    uint32_t sequence_num;
} __attribute__((packed));

// Entity data structures
struct EntityCreateData {
    uint32_t net_id;
    EntityType entity_type;
    uint32_t health;
    float position_x;
    float position_y;
} __attribute__((packed));

struct EntityUpdateData {
    uint32_t net_id;
    uint32_t health;
    float position_x;
    float position_y;
} __attribute__((packed));

struct PlayerInputData {
    EventType event_type;
    Direction direction;
} __attribute__((packed));

#endif /* !UDPMESSAGETYPE_HPP_ */
