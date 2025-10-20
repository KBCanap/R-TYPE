/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** projectile_system
*/

#include "../../include/systems.hpp"
#include "../include/render/IRenderWindow.hpp"
#include <vector>

namespace systems {

void projectile_system(registry &r,
                       sparse_array<component::projectile> &projectiles,
                       sparse_array<component::position> &positions,
                       render::IRenderWindow &window, float dt) {
    sparse_array<component::velocity> &velocities = r.get_components<component::velocity>();
    sparse_array<component::projectile_behavior> &behaviors = r.get_components<component::projectile_behavior>();
    std::vector<size_t> entities_to_kill;
    render::Vector2u window_size = window.getSize();

    const float BOUNDARY_MARGIN = -50.0f;
    const float max_x = static_cast<float>(window_size.x) + 50.0f;
    const float max_y = static_cast<float>(window_size.y) + 50.0f;

    for (size_t i = 0; i < std::min({projectiles.size(), positions.size(),
                                     velocities.size()});
         ++i) {
        std::optional<component::projectile> &projectile = projectiles[i];
        std::optional<component::position> &pos = positions[i];
        std::optional<component::velocity> &vel = velocities[i];

        bool valid = projectile && pos && vel;
        if (!valid) continue;

        projectile->age += dt;

        // Compute all death conditions upfront
        bool expired = (projectile->age >= projectile->lifetime);
        bool piercing_exhausted = projectile->piercing && (projectile->hits >= projectile->max_hits);
        bool out_of_bounds = (pos->x < BOUNDARY_MARGIN) | (pos->x > max_x) |
                            (pos->y < BOUNDARY_MARGIN) | (pos->y > max_y);

        bool should_die = expired | piercing_exhausted | out_of_bounds;

        if (should_die) {
            entities_to_kill.push_back(i);
            continue;
        }

        // Apply behavior if available
        bool has_behavior = (i < behaviors.size()) && behaviors[i];
        if (has_behavior) {
            behaviors[i]->pattern.apply_pattern(
                vel->vx, vel->vy, pos->x, pos->y, projectile->age,
                projectile->speed, projectile->friendly);
        }
    }

    for (size_t entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

} // namespace systems
