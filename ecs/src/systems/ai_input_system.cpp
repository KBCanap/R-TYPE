/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ai_input_system
*/

#include "../../include/systems.hpp"

namespace systems {

void ai_input_system(registry &r, sparse_array<component::ai_input> &ai_inputs,
                     float dt) {
    sparse_array<component::position> &positions =
        r.get_components<component::position>();
    sparse_array<component::velocity> &velocities =
        r.get_components<component::velocity>();

    for (size_t i = 0; i < ai_inputs.size(); ++i) {
        std::optional<component::ai_input> &ai_input = ai_inputs[i];
        if (!ai_input)
            continue;

        // Update fire timer and state
        ai_input->fire_timer += dt;
        bool should_fire = (ai_input->fire_timer >= ai_input->fire_interval);

        ai_input->fire = should_fire;
        ai_input->fire_timer *= (1 - should_fire); // Reset to 0 if firing

        // Check movement pattern
        bool has_components = (i < positions.size()) && positions[i] &&
                              (i < velocities.size()) && velocities[i];
        bool has_movement = (ai_input->movement_pattern.base_speed != 0.0f);

        if (has_components & has_movement) {
            ai_input->movement_pattern.apply_pattern(
                velocities[i]->vx, velocities[i]->vy, positions[i]->x,
                positions[i]->y, dt);
        }
    }
}

} // namespace systems
