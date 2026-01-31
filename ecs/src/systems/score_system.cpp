/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** score_system
*/

#include "../../include/systems.hpp"
#include <iostream>

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

        if (should_award) {
            score->current_score += 1;
            score->last_time_point_awarded = score->survival_time;

            // Debug log every time score is awarded
            static int award_count = 0;
            award_count++;
            if (award_count % 5 == 1) {  // Log every 5 seconds
                std::cout << "[SCORE DEBUG] Player " << i << " - Score: " << score->current_score
                          << " Survival: " << score->survival_time << "s" << std::endl;
            }
        }
    }
}

} // namespace systems
