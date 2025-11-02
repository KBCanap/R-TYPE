/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** score_system
*/

#include "../../include/systems.hpp"

namespace systems {

void score_system(registry &r, sparse_array<component::score> &scores,
                  float dt) {
    const float SCORE_INTERVAL = 1.0f;

    for (size_t i = 0; i < scores.size(); ++i) {
        std::optional<component::score> &score = scores[i];
        if (!score)
            continue;

        score->survival_time += dt;

        float time_since_award =
            score->survival_time - score->last_time_point_awarded;
        bool should_award = (time_since_award >= SCORE_INTERVAL);

        // Branchless score update
        score->current_score += should_award;
        score->last_time_point_awarded = should_award
                                             ? score->survival_time
                                             : score->last_time_point_awarded;
    }
}

} // namespace systems
