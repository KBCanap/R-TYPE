/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** weapon
*/

#include "../include/components.hpp"
#include "../include/registery.hpp"
#include <cmath>

namespace component {

void weapon::fire(registry &r, const position &shooter_pos, bool is_friendly) {
    if (fire_function) {
        fire_function(r, shooter_pos, is_friendly);
        return;
    }

    float base_angle = 0.0f;
    float angle_step = spread_angle;

    if (projectile_count > 1) {
        base_angle = -spread_angle * (projectile_count - 1) / 2.0f;
    }

    for (int i = 0; i < projectile_count; ++i) {
        float angle = base_angle + (angle_step * i);
        float angle_rad = angle * 3.14159f / 180.0f;

        // Calculate velocity with spread
        float vx = projectile_speed * std::cos(angle_rad);
        float vy = projectile_speed * std::sin(angle_rad);

        if (!is_friendly) {
            vx = -std::abs(vx); // Enemy projectiles go left
        } else {
            vx = std::abs(vx); // Player projectiles go right
        }

        entity projectile_entity = r.spawn_entity();

        r.add_component(projectile_entity,
                        position(shooter_pos.x + (is_friendly ? 50.0f : -20.0f),
                                 shooter_pos.y + 25.0f));

        r.add_component(projectile_entity, velocity(vx, vy));

        r.add_component(projectile_entity,
                        projectile(projectile_damage, projectile_speed,
                                   is_friendly, "bullet", projectile_lifetime,
                                   projectile_piercing, projectile_max_hits));

        r.add_component(projectile_entity,
                        projectile_behavior(movement_pattern));

        r.add_component(projectile_entity,
                        drawable("assets/sprites/r-typesheet1.gif",
                                 projectile_sprite_rect, 1.0f, "projectile"));

        r.add_component(
            projectile_entity,
            hitbox(static_cast<float>(projectile_sprite_rect.width) * 2.0f,
                   static_cast<float>(projectile_sprite_rect.height) * 2.0f,
                   0.0f, 0.0f));
    }
}

} // namespace component
