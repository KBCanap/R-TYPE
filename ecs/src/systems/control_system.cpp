/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** control_system
*/

#include "../../include/systems.hpp"

namespace systems {

void control_system(registry &r,
                    sparse_array<component::controllable> &controllables,
                    sparse_array<component::velocity> &velocities,
                    sparse_array<component::input> &inputs, float /*dt*/) {
    auto &animations = r.get_components<component::animation>();
    auto &drawables = r.get_components<component::drawable>();

    struct InputMapping {
        bool component::input::*input_ptr;
        bool is_x_axis;
        float multiplier;
    };

    static const InputMapping input_mappings[] = {
        {&component::input::left, true, -1.0f},
        {&component::input::right, true, 1.0f},
        {&component::input::up, false, -1.0f},
        {&component::input::down, false, 1.0f}
    };

    for (size_t i = 0;
         i < std::min({controllables.size(), velocities.size(), inputs.size()});
         ++i) {
        auto &ctrl = controllables[i];
        auto &vel = velocities[i];
        auto &input = inputs[i];
        if (ctrl && vel && input) {
            vel->vx = vel->vy = 0;

            // Branchless input mapping (max 1 branch per iteration)
            for (const auto &mapping : input_mappings) {
                bool is_active = (*input).*mapping.input_ptr;
                float value = is_active ? (ctrl->speed * mapping.multiplier) : 0.0f;

                // Branchless assignment using pointers
                float *target = mapping.is_x_axis ? &vel->vx : &vel->vy;
                *target = is_active ? value : *target;
            }

            if (i < animations.size() && animations[i] &&
                i < drawables.size() && drawables[i] &&
                drawables[i]->tag == "player") {
                auto &anim = animations[i];

                struct AnimationState {
                    bool playing;
                    int frame;
                    bool loop;
                    bool reverse;
                    float time;
                };

                static const AnimationState anim_up_transition = {true, 3, false, false, 0.0f};
                static const AnimationState anim_up_hold = {false, 4, false, false, 0.0f};
                static const AnimationState anim_down = {false, 3, false, false, 0.0f};
                static const AnimationState anim_neutral = {false, 2, true, false, 0.0f};

                // Branchless state selection using index calculation
                bool is_moving_up = (vel->vy < 0);
                bool is_moving_down = (vel->vy > 0);
                bool transitioning_to_up = is_moving_up && (ctrl->last_vy >= 0);
                bool holding_up = is_moving_up && !transitioning_to_up && (anim->current_frame >= 4);

                // Index: 0=neutral, 1=down, 2=up_hold, 3=up_transition
                static const AnimationState* state_table[4] = {
                    &anim_neutral, &anim_down, &anim_up_hold, &anim_up_transition
                };

                int idx = transitioning_to_up * 3 + holding_up * 2 + is_moving_down * 1;
                const AnimationState* state = state_table[idx];

                anim->playing = state->playing;
                anim->current_frame = state->frame;
                anim->loop = state->loop;

                // Branchless extra settings application
                bool apply_extra = transitioning_to_up;
                anim->reverse = apply_extra ? state->reverse : anim->reverse;
                anim->current_time = apply_extra ? state->time : anim->current_time;

                ctrl->last_vy = vel->vy;
            }
        }
    }
}

} // namespace systems
