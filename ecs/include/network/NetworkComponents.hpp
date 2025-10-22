#pragma once
#include "../components.hpp"
#include <cstdint>

namespace component {

/**
 * @struct network_entity
 * @brief Marks entity as networked with server-assigned ID
 */
struct network_entity {
    uint32_t network_id;
    uint8_t owner_player_id;
    bool is_local;
    float last_update_time;
    std::string entity_type;
    bool synced;

    network_entity(uint32_t net_id = 0, uint8_t owner = 0, bool local = false,
                   const std::string& type = "unknown")
        : network_id(net_id), owner_player_id(owner), is_local(local),
          last_update_time(0.0f), entity_type(type) {}
};

/**
 * @struct network_state
 * @brief Stores last known network state for interpolation
 */
struct network_state {
    position last_position;
    velocity last_velocity;
    uint32_t last_sequence;
    float interpolation_time;

    network_state()
        : last_position(0.0f, 0.0f), last_velocity(0.0f, 0.0f),
          last_sequence(0), interpolation_time(0.0f) {}
};

/**
 * @struct network_input
 * @brief Network-tracked input for client prediction
 */
struct network_input {
    uint32_t input_sequence;
    input input_data;

    network_input() : input_sequence(0), input_data() {}
};

/**
 * @struct network_transform
 * @brief Compressed transform data for network transmission
 */
struct network_transform {
    float pos_x;
    float pos_y;
    float vel_x;
    float vel_y;
    uint32_t timestamp;

    network_transform()
        : pos_x(0.0f), pos_y(0.0f), vel_x(0.0f), vel_y(0.0f), timestamp(0) {}

    network_transform(const position &pos, const velocity &vel, uint32_t ts)
        : pos_x(pos.x), pos_y(pos.y), vel_x(vel.vx), vel_y(vel.vy),
          timestamp(ts) {}
};

} // namespace component