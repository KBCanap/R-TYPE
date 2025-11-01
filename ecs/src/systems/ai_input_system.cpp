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
    sparse_array<component::position> &positions = r.get_components<component::position>();
    sparse_array<component::velocity> &velocities = r.get_components<component::velocity>();
    sparse_array<component::enemy_stunned> &stunneds = r.get_components<component::enemy_stunned>();
    sparse_array<component::drawable> &drawables = r.get_components<component::drawable>();
    sparse_array<component::gravity> &gravities = r.get_components<component::gravity>();
    sparse_array<component::hitbox> &hitboxes = r.get_components<component::hitbox>();

    // Track landing state for enemies (Mario platformer)
    static std::vector<bool> was_on_ground;
    static std::vector<int> landing_count;

    // Track entities to kill (off-screen)
    std::vector<size_t> entities_to_kill;

    for (size_t i = 0; i < ai_inputs.size(); ++i) {
        std::optional<component::ai_input> &ai_input = ai_inputs[i];
        if (!ai_input) continue;

        // Check if this is a stunned enemy (Mario platformer)
        auto stunned = (i < stunneds.size()) ? stunneds[i] : std::nullopt;
        auto drawable = (i < drawables.size()) ? drawables[i] : std::nullopt;
        auto velocity = (i < velocities.size()) ? velocities[i] : std::nullopt;
        auto position = (i < positions.size()) ? positions[i] : std::nullopt;
        auto hitbox = (i < hitboxes.size()) ? hitboxes[i] : std::nullopt;

        if (stunned && stunned->stunned && drawable && drawable->tag == "enemy" && velocity) {
            // Stunned enemy - apply knockback velocity instead of AI movement
            velocity->vx = stunned->knockback_velocity;

            // Check if stunned enemy is off-screen (kill it)
            if (position && hitbox) {
                // Consider off-screen if completely outside the play area
                // Using a large margin (e.g., -200 to 2000) to handle different screen sizes
                if (position->x + hitbox->width < -200.0f || position->x > 2000.0f) {
                    entities_to_kill.push_back(i);
                }
            }

            continue; // Skip normal AI processing
        }

        // Enemy platform landing behavior (Mario platformer)
        auto gravity = (i < gravities.size()) ? gravities[i] : std::nullopt;
        if (gravity && drawable && drawable->tag == "enemy") {
            // Ensure tracking vectors are large enough
            if (was_on_ground.size() <= i) {
                was_on_ground.resize(i + 1, false);
            }
            if (landing_count.size() <= i) {
                landing_count.resize(i + 1, 0);
            }

            // If enemy just landed on a platform (transition from air to ground)
            if (gravity->on_ground && !was_on_ground[i]) {
                landing_count[i]++;

                // Only reverse direction from the second landing onwards
                if (landing_count[i] >= 2) {
                    ai_input->movement_pattern.base_speed = -ai_input->movement_pattern.base_speed;
                }
            }

            // Update tracking
            was_on_ground[i] = gravity->on_ground;
        }

        // Update fire timer and state
        ai_input->fire_timer += dt;
        bool should_fire = (ai_input->fire_timer >= ai_input->fire_interval);

        ai_input->fire = should_fire;
        ai_input->fire_timer *= (1 - should_fire);  // Reset to 0 if firing

        // Check movement pattern
        bool has_components = (i < positions.size()) && positions[i] &&
                             (i < velocities.size()) && velocities[i];
        bool has_movement = (ai_input->movement_pattern.base_speed != 0.0f);

        if (has_components & has_movement) {
            // For platformer enemies (with gravity), only control horizontal movement
            if (gravity && drawable && drawable->tag == "enemy") {
                // Only set horizontal velocity, let gravity handle vertical
                velocities[i]->vx = ai_input->movement_pattern.base_speed;
            } else {
                // Normal R-Type enemies - full pattern control
                ai_input->movement_pattern.apply_pattern(
                    velocities[i]->vx, velocities[i]->vy,
                    positions[i]->x, positions[i]->y, dt);
            }
        }
    }

    // Kill entities that went off-screen
    for (size_t entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

} // namespace systems
