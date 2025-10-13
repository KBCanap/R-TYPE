/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ai_movement_pattern
*/

#include "../include/components.hpp"
#include <cmath>

namespace component {

void ai_movement_pattern::apply_pattern(float &vx, float &vy, float pos_x,
                                        float pos_y, float dt) {
    pattern_time += dt;

    if (pattern_type == "straight") {
        vx = -base_speed;
        vy = 0.0f;
    } else if (pattern_type == "wave") {
        vx = -base_speed;
        vy = amplitude * std::sin(frequency * pos_x + phase_offset);
    } else if (pattern_type == "sine_wave") {
        vx = -base_speed;
        vy = amplitude * std::sin(frequency * pattern_time + phase_offset);
    } else if (pattern_type == "zigzag") {
        vx = -base_speed;
        float triangle_wave =
            std::abs(std::fmod(frequency * pattern_time + phase_offset, 2.0f) -
                     1.0f) *
                2.0f -
            1.0f;
        vy = amplitude * triangle_wave;
    } else if (pattern_type == "circle") {
        if (start_x == 0.0f && start_y == 0.0f) {
            start_x = pos_x;
            start_y = pos_y;
        }
        float angle = frequency * pattern_time + phase_offset;
        float target_x =
            start_x + amplitude * std::cos(angle) - base_speed * pattern_time;
        float target_y = start_y + amplitude * std::sin(angle);

        vx = (target_x - pos_x) / dt;
        vy = (target_y - pos_y) / dt;
    } else {
        vx = -base_speed;
        vy = 0.0f;
    }
}

ai_movement_pattern ai_movement_pattern::straight(float speed) {
    ai_movement_pattern pattern("straight");
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::wave(float amplitude, float frequency,
                                              float speed) {
    ai_movement_pattern pattern("wave", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern
ai_movement_pattern::sine_wave(float amplitude, float frequency, float speed) {
    ai_movement_pattern pattern("sine_wave", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::zigzag(float amplitude,
                                                float frequency, float speed) {
    ai_movement_pattern pattern("zigzag", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::circle(float radius, float speed) {
    ai_movement_pattern pattern("circle", radius,
                                0.02f);
    pattern.base_speed = speed;
    return pattern;
}

} // namespace component