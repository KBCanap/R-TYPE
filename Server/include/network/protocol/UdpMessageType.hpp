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
    PLAYER_INPUT = 0x20,
    VICTORY = 0x30
};

// Entity types
enum EntityType : uint8_t {
    PLAYER = 0x01,
    ENEMY = 0x02,               // Level 1 standard enemy (r-typesheet9.gif)
    PROJECTILE = 0x03,
    ALLIED_PROJECTILE = 0x04,
    BOSS = 0x05,                // Level 1 boss (r-typesheet17.gif)
    POWERUP_SHIELD = 0x06,      // Shield power-up
    POWERUP_SPREAD = 0x07,      // Spread power-up
    ENEMY_LEVEL2 = 0x08,        // Level 2 standard enemy (r-typesheet5.gif)
    ENEMY_LEVEL2_SPREAD = 0x09, // Level 2 spread enemy (r-typesheet11.gif)
    BOSS_LEVEL2_PART1 = 0x0A,   // Level 2 boss left part (r-typesheet38.gif)
    BOSS_LEVEL2_PART2 = 0x0B,   // Level 2 boss center part (r-typesheet38.gif)
    BOSS_LEVEL2_PART3 = 0x0C,   // Level 2 boss right part (r-typesheet38.gif)
    POWERUP_COMPANION = 0x0D,   // Companion power-up pickup (r-typesheet27.gif left)
    COMPANION = 0x0E            // Active companion following player (r-typesheet27.gif right)
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
static constexpr std::size_t ENTITY_CREATE_SIZE = 21;
static constexpr std::size_t ENTITY_UPDATE_SIZE = 25;
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
    uint32_t shield;
    float position_x;
    float position_y;
} __attribute__((packed));

struct EntityUpdateData {
    uint32_t net_id;
    EntityType entity_type;
    uint32_t health;
    uint32_t shield;
    float position_x;
    float position_y;
    uint32_t score;
} __attribute__((packed));

struct PlayerInputData {
    EventType event_type;
    Direction direction;
} __attribute__((packed));

#endif /* !UDPMESSAGETYPE_HPP_ */
