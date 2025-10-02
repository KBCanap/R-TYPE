/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** Event
*/

#pragma once
#include <any>
#include <cstdint>

/**
 * @brief Enum for event identification
 * Enum is faster than string
 */
enum class EventType : uint32_t {
    Collision,
    Damage,
    LevelUp,
    PlayerHit,
    ItemPickup,
    GameOver,
    // Add more event types here as needed
};

/**
 * @brief Struct for the custom event
 *
 */
struct Event {
    EventType type;
    std::any payload;

    Event(EventType t, std::any p = {}) : type(t), payload(std::move(p)) {}
};
