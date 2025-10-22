/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** gravity_system
*/

#include "systems.hpp"

namespace systems {

void gravity_system(registry &r, sparse_array<component::gravity> &gravities,
                    sparse_array<component::velocity> &velocities, float dt)
{
    for (size_t i = 0; i < gravities.size() && i < velocities.size(); ++i) {
        auto &gravity = gravities[i];
        auto &velocity = velocities[i];

        if (!gravity.has_value() || !velocity.has_value())
            continue;

        // Apply gravity force to vertical velocity
        if (!gravity->on_ground) {
            velocity->vy += gravity->force * dt;

            // Clamp to terminal velocity
            if (velocity->vy > gravity->terminal_velocity) {
                velocity->vy = gravity->terminal_velocity;
            }
        }
    }
}

} // namespace systems
