/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** health_system
*/

#include "../../include/systems.hpp"
#include <algorithm>
#include <vector>

namespace systems {

static void award_score_for_enemy_kill(sparse_array<component::score> &scores,
                                        sparse_array<component::drawable> &drawables,
                                        int points) {
    for (size_t score_idx = 0; score_idx < scores.size(); ++score_idx) {
        auto &score = scores[score_idx];
        if (!score) continue;
        if (score_idx >= drawables.size()) continue;
        if (!drawables[score_idx]) continue;
        if (drawables[score_idx]->tag != "player") continue;

        score->current_score += points;
        score->enemies_killed += 1;
        break;
    }
}

static void handle_entity_death(registry &r, size_t entity_idx,
                                 sparse_array<component::position> &positions,
                                 sparse_array<component::drawable> &drawables,
                                 sparse_array<component::score> &scores) {
    const int ENEMY_KILL_SCORE = 5;

    bool has_position = (entity_idx < positions.size()) && positions[entity_idx];
    if (has_position) {
        create_explosion(r, positions[entity_idx]->x, positions[entity_idx]->y);
    }

    bool is_enemy = (entity_idx < drawables.size()) && drawables[entity_idx] &&
                    is_enemy_tag(drawables[entity_idx]->tag);
    if (is_enemy) {
        award_score_for_enemy_kill(scores, drawables, ENEMY_KILL_SCORE);
    }
}

void health_system(registry &r, sparse_array<component::health> &healths,
                   float dt) {
    std::vector<size_t> entities_to_kill;
    auto &positions = r.get_components<component::position>();
    auto &drawables = r.get_components<component::drawable>();
    auto &scores = r.get_components<component::score>();

    for (size_t i = 0; i < healths.size(); ++i) {
        auto &health = healths[i];
        if (!health) continue;

        health->current_hp -= health->pending_damage;
        health->pending_damage = 0;
        health->current_hp = std::max(0, std::min(health->current_hp, health->max_hp));

        if (health->current_hp > 0) continue;

        entities_to_kill.push_back(i);
        handle_entity_death(r, i, positions, drawables, scores);
    }

    for (auto entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

} // namespace systems
