/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** enemy_system - Enemy patrol and AI behavior
*/

#include "systems.hpp"

namespace systems {

void enemy_system(registry &r,
                  sparse_array<component::position> &positions,
                  sparse_array<component::velocity> &velocities,
                  sparse_array<component::controllable> &controllables,
                  sparse_array<component::gravity> &gravities,
                  sparse_array<component::drawable> &drawables,
                  sparse_array<component::hitbox> &hitboxes,
                  float window_width)
{
    for (size_t i = 0; i < positions.size(); ++i) {
        if (!positions[i].has_value())
            continue;
        if (i >= velocities.size() || !velocities[i].has_value())
            continue;
        if (i >= controllables.size() || !controllables[i].has_value())
            continue;
        if (i >= gravities.size() || !gravities[i].has_value())
            continue;
        if (i >= drawables.size() || !drawables[i].has_value())
            continue;
        if (i >= hitboxes.size() || !hitboxes[i].has_value())
            continue;

        auto &pos = positions[i];
        auto &vel = velocities[i];
        auto &ctrl = controllables[i];
        auto &grav = gravities[i];
        auto &draw = drawables[i];
        auto &hitbox = hitboxes[i];

        // Only process enemies (check tag)
        if (draw->tag != "enemy")
            continue;

        // last_vy stores direction: 1.0 = right, -1.0 = left
        // speed stores the movement speed
        bool moving_right = ctrl->last_vy > 0.0f;
        bool was_on_ground = (ctrl->last_vy > -0.5f && ctrl->last_vy < 1.5f);

        // Check if enemy just left the ground (fell off platform)
        if (was_on_ground && grav->on_ground && !was_on_ground) {
            // Change direction when falling off platform
            moving_right = !moving_right;
            ctrl->last_vy = moving_right ? 1.0f : -1.0f;
        }

        // Check if enemy hit a wall (left or right edge of window)
        float entity_left = pos->x + hitbox->offset_x;
        float entity_right = pos->x + hitbox->offset_x + hitbox->width;

        if (entity_left <= 0.0f && !moving_right) {
            // Hit left wall while moving left - change direction
            moving_right = true;
            ctrl->last_vy = 1.0f;
        } else if (entity_right >= window_width && moving_right) {
            // Hit right wall while moving right - change direction
            moving_right = false;
            ctrl->last_vy = -1.0f;
        }

        // Set horizontal velocity based on direction
        if (moving_right) {
            vel->vx = ctrl->speed;
        } else {
            vel->vx = -ctrl->speed;
        }
    }
}

} // namespace systems
