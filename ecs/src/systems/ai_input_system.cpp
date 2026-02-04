/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ai_input_system
*/

#include "../../include/systems.hpp"
#include <cmath>

namespace systems {

void ai_input_system(registry &r, sparse_array<component::ai_input> &ai_inputs,
                     float dt) {
    sparse_array<component::position> &positions = r.get_components<component::position>();
    sparse_array<component::velocity> &velocities = r.get_components<component::velocity>();
    sparse_array<component::enemy_stunned> &stunneds = r.get_components<component::enemy_stunned>();
    sparse_array<component::drawable> &drawables = r.get_components<component::drawable>();
    sparse_array<component::gravity> &gravities = r.get_components<component::gravity>();
    sparse_array<component::hitbox> &hitboxes = r.get_components<component::hitbox>();

    static std::vector<bool> was_on_ground;
    static std::vector<int> landing_count;

    std::vector<size_t> entities_to_kill;

    for (size_t i = 0; i < ai_inputs.size(); ++i) {
        std::optional<component::ai_input> &ai_input = ai_inputs[i];
        if (!ai_input)
            continue;

        // Check if this is a stunned enemy (Mario platformer)
        bool has_stunned = (i < stunneds.size()) && stunneds[i];
        bool has_drawable = (i < drawables.size()) && drawables[i];
        bool has_velocity = (i < velocities.size()) && velocities[i];
        bool has_position = (i < positions.size()) && positions[i];
        bool has_hitbox = (i < hitboxes.size()) && hitboxes[i];

        if (has_stunned && stunneds[i]->stunned && has_drawable &&
            drawables[i]->tag == "enemy" && has_velocity) {
            // Stunned enemy - apply knockback velocity instead of AI movement
            velocities[i]->vx = stunneds[i]->knockback_velocity;

            // Check if stunned enemy is off-screen (kill it)
            if (has_position && has_hitbox) {
                if (positions[i]->x + hitboxes[i]->width < -200.0f ||
                    positions[i]->x > 2000.0f) {
                    entities_to_kill.push_back(i);
                    continue;
                }
            }

            // Update stun timer only if not being knocked back
            if (std::abs(stunneds[i]->knockback_velocity) < 1.0f) {
                stunneds[i]->stun_timer += dt;

                // Check for recovery
                if (stunneds[i]->stun_timer >= stunneds[i]->recovery_time) {
                    stunneds[i]->stunned = false;
                    stunneds[i]->stun_timer = 0.0f;
                    stunneds[i]->angry = true;

                    // Angry enemies move 50% faster
                    float speed_multiplier = 1.5f;
                    ai_input->movement_pattern.base_speed =
                        std::abs(ai_input->movement_pattern.base_speed) *
                        speed_multiplier *
                        (ai_input->movement_pattern.base_speed >= 0 ? 1.0f : -1.0f);
                }
            }

            continue; // Skip normal AI processing
        }

        // Enemy platform landing behavior (Mario platformer)
        bool has_gravity = (i < gravities.size()) && gravities[i];
        if (has_gravity && has_drawable && drawables[i]->tag == "enemy") {
            // Ensure tracking vectors are large enough
            if (was_on_ground.size() <= i) {
                was_on_ground.resize(i + 1, false);
            }
            if (landing_count.size() <= i) {
                landing_count.resize(i + 1, 0);
            }

            // If enemy just landed on a platform (transition from air to ground)
            if (gravities[i]->on_ground && !was_on_ground[i]) {
                landing_count[i]++;

                // Only reverse direction from the second landing onwards
                if (landing_count[i] >= 2) {
                    ai_input->movement_pattern.base_speed = -ai_input->movement_pattern.base_speed;
                }
            }

            // Update tracking
            was_on_ground[i] = gravities[i]->on_ground;
        }

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
            if (has_gravity && has_drawable && drawables[i]->tag == "enemy") {
                velocities[i]->vx = ai_input->movement_pattern.base_speed;
            } else {
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
