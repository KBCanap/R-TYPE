#pragma once
#include "components.hpp"
#include "registery.hpp"

namespace weapon_presets {

// Standard weapon (single shot, straight)
inline component::weapon standard_weapon() {
    return component::weapon(
        2.0f,                                      // fire_rate
        true,                                      // friendly
        1,                                         // projectile_count
        0.0f,                                      // spread_angle
        component::projectile_pattern::straight(), // movement_pattern
        20.0f,                                     // damage
        500.0f,                                    // speed
        5.0f,                                      // lifetime
        false,                                     // piercing
        1,                                         // max_hits
        false                                      // is_burst
    );
}

// Burst weapon (3-shot burst)
inline component::weapon burst_weapon() {
    return component::weapon(
        2.0f,                                      // fire_rate
        true,                                      // friendly
        1,                                         // projectile_count
        0.0f,                                      // spread_angle
        component::projectile_pattern::straight(), // movement_pattern
        20.0f,                                     // damage
        500.0f,                                    // speed
        5.0f,                                      // lifetime
        false,                                     // piercing
        1,                                         // max_hits
        true,                                      // is_burst
        3,                                         // burst_count
        0.1f                                       // burst_interval
    );
}

} // namespace weapon_presets
