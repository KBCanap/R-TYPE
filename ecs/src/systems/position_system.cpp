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
    render::Vector2u window_size = window.getSize();
    const float max_y = static_cast<float>(window_size.y) - 100.0f;

    for (size_t i = 0;
         i < std::min({drawables.size(), positions.size(), velocities.size()});
         ++i) {
        std::optional<component::drawable> &drawable = drawables[i];
        std::optional<component::position> &pos = positions[i];
        std::optional<component::velocity> &vel = velocities[i];


        bool is_boss = drawable && (drawable->tag == "boss");
        bool has_components = pos && vel;

        if (is_boss & has_components) {
            // Branchless clamping: compute both directions and select
            float is_below_min = (pos->y <= 50.0f) ? 1.0f : 0.0f;
            float is_above_max = (pos->y >= max_y) ? 1.0f : 0.0f;
            float abs_vy = std::abs(vel->vy);

            vel->vy = abs_vy * is_below_min +
                      (-abs_vy) * is_above_max +
                      vel->vy * (1.0f - is_below_min - is_above_max);
        }
    }

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
}

} // namespace systems
