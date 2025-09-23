#include "../include/components.hpp"
#include <cmath>

namespace component {

projectile_pattern::projectile_pattern(const std::string& type, float p1, float p2, float p3, float p4)
    : pattern_type(type), param1(p1), param2(p2), param3(p3), param4(p4) {}

void projectile_pattern::apply_pattern(float& vx, float& vy, float pos_x, float pos_y, float age, float speed, bool friendly) const {
    if (custom_function) {
        custom_function(vx, vy, pos_x, pos_y, age, speed, friendly);
        return;
    }

    float base_direction = friendly ? 1.0f : -1.0f;

    if (pattern_type == "straight") {
        // Mouvement linéaire - garde la vélocité initiale (permet le spread)
        float current_speed = std::sqrt(vx * vx + vy * vy);
        if (current_speed > 0) {
            float scale = speed / current_speed;
            vx *= scale;
            vy *= scale;
        }
    } else if (pattern_type == "wave") {
        // Mouvement en vague sinusoïdale
        // param1 = amplitude, param2 = frequency, param3 = phase_offset
        vx = base_direction * speed;
        vy = std::sin((pos_x + param3) * param2) * param1;
    } else if (pattern_type == "spiral") {
        // Mouvement en spirale
        // param1 = amplitude, param2 = turn_speed, param3 = phase_offset
        float angle = age * param2 + param3;
        float spiral_radius = param1 * (age * 0.5f);

        vx = base_direction * speed + spiral_radius * std::cos(angle);
        vy = spiral_radius * std::sin(angle);
    } else if (pattern_type == "bounce") {
        // Mouvement avec rebonds
        float screen_width = 800.0f;
        float screen_height = 600.0f;

        // Rebondir sur les bords
        if (pos_x <= 0 || pos_x >= screen_width) {
            vx = -vx;
        }
        if (pos_y <= 0 || pos_y >= screen_height) {
            vy = -vy;
        }
    } else if (pattern_type == "spread") {
        // Mouvement en éventail - applique un angle progressif aux projectiles
        // param1 = spread_angle (angle total de l'éventail)
        // Le spread est géré au niveau de l'arme, ce pattern garde juste la direction initiale
        float current_speed = std::sqrt(vx * vx + vy * vy);
        if (current_speed > 0) {
            float scale = speed / current_speed;
            vx *= scale;
            vy *= scale;
        }
    }
}

// Patterns prédéfinis
projectile_pattern projectile_pattern::straight() {
    return projectile_pattern("straight");
}

projectile_pattern projectile_pattern::wave(float amplitude, float frequency, float phase_offset) {
    return projectile_pattern("wave", amplitude, frequency, phase_offset);
}

projectile_pattern projectile_pattern::spiral(float amplitude, float turn_speed, float phase_offset) {
    return projectile_pattern("spiral", amplitude, turn_speed, phase_offset);
}

projectile_pattern projectile_pattern::bounce() {
    return projectile_pattern("bounce");
}

projectile_pattern projectile_pattern::spread(float spread_angle) {
    return projectile_pattern("spread", spread_angle);
}

}