/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** platform_collision_system
*/

#include "systems.hpp"

namespace systems {

void platform_collision_system(registry &r,
                                sparse_array<component::position> &positions,
                                sparse_array<component::velocity> &velocities,
                                sparse_array<component::gravity> &gravities,
                                sparse_array<component::platform_tag> &platforms,
                                sparse_array<component::hitbox> &hitboxes)
{
    // Check all entities with gravity against all platforms
    for (size_t i = 0; i < positions.size(); ++i) {
        std::optional<component::position> &pos = positions[i];
        if (!pos.has_value())
            continue;
        if (i >= velocities.size() || !velocities[i].has_value())
            continue;
        if (i >= gravities.size() || !gravities[i].has_value())
            continue;
        if (i >= hitboxes.size() || !hitboxes[i].has_value())
            continue;

        std::optional<component::velocity> &vel = velocities[i];
        std::optional<component::gravity> &grav = gravities[i];
        std::optional<component::hitbox> &hitbox = hitboxes[i];

        // Entity bounding box
        float entity_left = pos->x + hitbox->offset_x;
        float entity_right = pos->x + hitbox->offset_x + hitbox->width;
        float entity_top = pos->y + hitbox->offset_y;
        float entity_bottom = pos->y + hitbox->offset_y + hitbox->height;

        grav->on_ground = false;
        constexpr float ground_tolerance = 2.0f; // Tolerance for ground detection

        // Check collision with all platforms
        for (size_t j = 0; j < positions.size(); ++j) {
            if (i == j)
                continue;
            if (!positions[j].has_value())
                continue;
            if (j >= platforms.size() || !platforms[j].has_value())
                continue;
            if (j >= hitboxes.size() || !hitboxes[j].has_value())
                continue;

            std::optional<component::position> &plat_pos = positions[j];
            std::optional<component::platform_tag> &platform = platforms[j];
            std::optional<component::hitbox> &plat_hitbox = hitboxes[j];

            // Platform bounding box
            float plat_left = plat_pos->x + plat_hitbox->offset_x;
            float plat_right = plat_pos->x + plat_hitbox->offset_x + plat_hitbox->width;
            float plat_top = plat_pos->y + plat_hitbox->offset_y;
            float plat_bottom = plat_pos->y + plat_hitbox->offset_y + plat_hitbox->height;

            // Check if entity is colliding with platform
            bool overlapping_x = entity_right > plat_left && entity_left < plat_right;
            bool overlapping_y = entity_bottom > plat_top && entity_top < plat_bottom;

            // Also check if entity is standing on top of platform (with tolerance)
            bool standing_on_top = overlapping_x &&
                                    entity_bottom >= plat_top - ground_tolerance &&
                                    entity_bottom <= plat_top + ground_tolerance &&
                                    vel->vy >= 0; // Only when falling or stationary

            if (standing_on_top) {
                // Snap to platform top
                pos->y = plat_top - hitbox->height - hitbox->offset_y;
                vel->vy = 0;
                grav->on_ground = true;
            } else if (overlapping_x && overlapping_y) {
                // Calculate overlap amounts
                float overlap_left = entity_right - plat_left;
                float overlap_right = plat_right - entity_left;
                float overlap_top = entity_bottom - plat_top;
                float overlap_bottom = plat_bottom - entity_top;

                // Find smallest overlap
                float min_overlap = overlap_left;
                int direction = 0; // 0=left, 1=right, 2=top, 3=bottom

                if (overlap_right < min_overlap) {
                    min_overlap = overlap_right;
                    direction = 1;
                }
                if (overlap_top < min_overlap) {
                    min_overlap = overlap_top;
                    direction = 2;
                }
                if (overlap_bottom < min_overlap) {
                    min_overlap = overlap_bottom;
                    direction = 3;
                }

                // Resolve collision based on smallest overlap
                if (direction == 0) { // Push left
                    pos->x -= overlap_left;
                    vel->vx = 0;
                } else if (direction == 1) { // Push right
                    pos->x += overlap_right;
                    vel->vx = 0;
                } else if (direction == 2) { // Push up (landing on platform)
                    pos->y -= overlap_top;
                    vel->vy = 0;
                    grav->on_ground = true;
                } else if (direction == 3) { // Push down (hitting ceiling)
                    pos->y += overlap_bottom;
                    vel->vy = 0;
                }
            }
        }
    }
}

} // namespace systems
