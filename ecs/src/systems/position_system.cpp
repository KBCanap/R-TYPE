/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** position_system
*/

#include "../../include/systems.hpp"
#include "../../include/render/IRenderWindow.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace systems {

void position_system(registry &r, sparse_array<component::position> &positions,
                     sparse_array<component::velocity> &velocities,
                     sparse_array<component::input> &inputs,
                     render::IRenderWindow &window, float current_time,
                     float dt) {
    sparse_array<component::drawable> &drawables = r.get_components<component::drawable>();
    sparse_array<component::ai_input> &ai_inputs = r.get_components<component::ai_input>();
    render::Vector2u window_size = window.getSize();
    const float min_y = 50.0f;
    const float max_y = static_cast<float>(window_size.y) - 50.0f;

    // First update positions
    for (size_t i = 0; i < std::min(positions.size(), velocities.size()); ++i) {
        std::optional<component::position> &pos = positions[i];
        std::optional<component::velocity> &vel = velocities[i];
        bool valid = pos && vel;
        float dt_mult = valid ? dt : 0.0f;

        if (valid) {
            pos->x += vel->vx * dt_mult;
            pos->y += vel->vy * dt_mult;
        }
    }

    // Then apply bouncing for bosses
    for (size_t i = 0;
         i < std::min({drawables.size(), positions.size(), velocities.size()});
         ++i) {
        std::optional<component::drawable> &drawable = drawables[i];
        std::optional<component::position> &pos = positions[i];
        std::optional<component::velocity> &vel = velocities[i];

        bool is_boss = drawable && (drawable->tag == "boss");
        bool has_components = pos && vel;

        // Apply bouncing logic if boss has AI with non-zero base_speed OR has vertical velocity
        bool should_bounce = false;
        if (i < ai_inputs.size() && ai_inputs[i]) {
            should_bounce = (ai_inputs[i]->movement_pattern.base_speed > 0.1f);
        }
        // Also enable bounce if boss has vertical velocity (for simple up-down movement)
        if (!should_bounce && vel && std::abs(vel->vy) > 0.1f) {
            should_bounce = true;
        }

        if (is_boss & has_components & should_bounce) {
            // Check bounds and reverse velocity if needed
            if (pos->y <= min_y && vel->vy < 0) {
                std::cout << "[PositionSystem] Boss hit top, bouncing down. y=" << pos->y << " vy=" << vel->vy << std::endl;
                vel->vy = -vel->vy;  // Bounce down
            } else if (pos->y >= max_y && vel->vy > 0) {
                std::cout << "[PositionSystem] Boss hit bottom, bouncing up. y=" << pos->y << " vy=" << vel->vy << " max_y=" << max_y << std::endl;
                vel->vy = -vel->vy;  // Bounce up
            }
        }
    }
}

} // namespace systems
