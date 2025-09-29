#pragma once
#include <cstdint>
#include <any>

/**
 * @brief Enum for event identification
 * Enum is faster than string or user custom define
 *
 */
enum class EventType : uint32_t {
    Collision,
    Damage,
    LevelUp,
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