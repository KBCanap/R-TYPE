/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_systems - Systems specific to Mario platformer game
*/

#include "systems.hpp"
#include <algorithm>
#include <iostream>

namespace systems {

// Gravity system - applies gravity acceleration to entities
void gravity_system(registry &r, sparse_array<component::gravity> &gravities,
                    sparse_array<component::velocity> &velocities, float dt) {
    auto &deads = r.get_components<component::dead>();

    for (size_t i = 0; i < gravities.size() && i < velocities.size(); ++i) {
        auto &gravity = gravities[i];
        auto &velocity = velocities[i];

        if (!gravity || !velocity)
            continue;

        // Check if entity is dead
        auto dead = (i < deads.size()) ? deads[i] : std::nullopt;

        // Apply gravity acceleration
        velocity->vy += gravity->acceleration * dt;

        // Cap falling speed (unless dead - let them fall through)
        if (!dead || !dead->ignore_platforms) {
            if (velocity->vy > gravity->max_velocity) {
                velocity->vy = gravity->max_velocity;
            }
        }
    }
}

// Platform collision system - handles collision detection and resolution
void platform_collision_system(
    registry &r, sparse_array<component::position> &positions,
    sparse_array<component::velocity> &velocities,
    sparse_array<component::gravity> &gravities,
    sparse_array<component::platform_tag> &platforms,
    sparse_array<component::hitbox> &hitboxes) {

    auto &deads = r.get_components<component::dead>();

    // For each entity with gravity (player/enemies)
    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &vel = velocities[i];
        auto &grav = gravities[i];
        auto &hitbox = hitboxes[i];

        if (!pos || !vel || !grav || !hitbox)
            continue;

        // Check if entity is dead and should ignore platforms
        auto dead = (i < deads.size()) ? deads[i] : std::nullopt;
        if (dead && dead->ignore_platforms) {
            grav->on_ground = false;
            continue; // Skip collision for dead entities
        }

        // Reset on_ground status
        grav->on_ground = false;

        // Check collision with each platform
        for (size_t j = 0; j < positions.size(); ++j) {
            auto &plat_pos = positions[j];
            auto &plat_tag = platforms[j];
            auto &plat_hitbox = hitboxes[j];

            if (!plat_pos || !plat_tag || !plat_hitbox)
                continue;

            // Calculate entity bounds
            float entity_left = pos->x + hitbox->offset_x;
            float entity_right = entity_left + hitbox->width;
            float entity_top = pos->y + hitbox->offset_y;
            float entity_bottom = entity_top + hitbox->height;

            // Calculate platform bounds
            float plat_left = plat_pos->x + plat_hitbox->offset_x;
            float plat_right = plat_left + plat_hitbox->width;
            float plat_top = plat_pos->y + plat_hitbox->offset_y;
            float plat_bottom = plat_top + plat_hitbox->height;

            // Check for collision
            bool x_overlap = entity_right > plat_left && entity_left < plat_right;
            bool y_overlap =
                entity_bottom > plat_top && entity_top < plat_bottom;

            if (x_overlap && y_overlap) {
                // Calculate overlap amounts
                float overlap_left = entity_right - plat_left;
                float overlap_right = plat_right - entity_left;
                float overlap_top = entity_bottom - plat_top;
                float overlap_bottom = plat_bottom - entity_top;

                // Find smallest overlap
                float min_overlap =
                    std::min({overlap_left, overlap_right, overlap_top,
                              overlap_bottom});

                // Resolve collision based on smallest overlap
                if (min_overlap == overlap_top && vel->vy > 0) {
                    // Landing on platform from above
                    pos->y = plat_top - hitbox->height - hitbox->offset_y;
                    vel->vy = 0.0f;
                    grav->on_ground = true;
                } else if (min_overlap == overlap_bottom && vel->vy < 0) {
                    // Hit platform from below
                    pos->y = plat_bottom - hitbox->offset_y;
                    vel->vy = 0.0f;
                } else if (min_overlap == overlap_left && vel->vx > 0) {
                    // Hit platform from left
                    pos->x = plat_left - hitbox->width - hitbox->offset_x;
                    vel->vx = 0.0f;
                } else if (min_overlap == overlap_right && vel->vx < 0) {
                    // Hit platform from right
                    pos->x = plat_right - hitbox->offset_x;
                    vel->vx = 0.0f;
                }
            }
        }
    }
}

} // namespace systems
