/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** weapon_system
*/

#include "../../include/systems.hpp"
#include "algorithm"

namespace systems {

void weapon_system(registry &r, sparse_array<component::weapon> &weapons,
                   sparse_array<component::position> &positions,
                   sparse_array<component::input> &inputs,
                   sparse_array<component::ai_input> &ai_inputs,
                   float current_time) {

    for (size_t i = 0; i < std::min({weapons.size(), positions.size()}); ++i) {
        std::optional<component::weapon> &weapon = weapons[i];
        std::optional<component::position> &pos = positions[i];

        // Branchless input check
        bool has_input = (i < inputs.size()) && inputs[i] && inputs[i]->fire;
        bool has_ai_input = (i < ai_inputs.size()) && ai_inputs[i] && ai_inputs[i]->fire;
        bool can_fire_input = has_input | has_ai_input;

        if (weapon && pos && can_fire_input) {
            float time_since_last_shot = current_time - weapon->last_shot_time;
            float time_since_last_burst = current_time - weapon->last_burst_time;
            float min_interval = 1.0f / weapon->fire_rate;

            // Compute conditions without branching logic where possible
            bool is_burst_start = (weapon->current_burst == 0) && (time_since_last_shot >= min_interval);
            bool is_burst_continue = (weapon->current_burst > 0) &&
                                     (weapon->current_burst < weapon->burst_count) &&
                                     (time_since_last_burst >= weapon->burst_interval);
            bool should_reset_burst = (weapon->current_burst >= weapon->burst_count);

            bool can_fire = false;

            if (weapon->is_burst) {
                weapon->current_burst *= !should_reset_burst;  // Reset to 0 if should_reset

                can_fire = is_burst_start | is_burst_continue;

                weapon->last_shot_time = is_burst_start ? current_time : weapon->last_shot_time;
                weapon->last_burst_time = (is_burst_start | is_burst_continue) ? current_time : weapon->last_burst_time;
                weapon->current_burst += (is_burst_start | is_burst_continue);
            } else {
                can_fire = time_since_last_shot >= min_interval;
                weapon->last_shot_time = can_fire ? current_time : weapon->last_shot_time;
            }

            if (can_fire) {
                weapon->fire(r, *pos, weapon->friendly);
            }
        }
    }
}

} // namespace systems
