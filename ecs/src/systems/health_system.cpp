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
        std::optional<component::score> &score = scores[score_idx];
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
    sparse_array<component::position> &positions = r.get_components<component::position>();
    sparse_array<component::drawable> &drawables = r.get_components<component::drawable>();
    sparse_array<component::score> &scores = r.get_components<component::score>();
    sparse_array<component::shield> &shields = r.get_components<component::shield>();

    for (size_t i = 0; i < healths.size(); ++i) {
        std::optional<component::health> &health = healths[i];
        if (!health) continue;

        int damage = health->pending_damage;
        health->pending_damage = 0;

        // Apply damage to shield first, then to health
        if (i < shields.size() && shields[i]) {
            int shield_damage = std::min(damage, shields[i]->current_shield);
            shields[i]->current_shield -= shield_damage;
            shields[i]->current_shield = std::max(0, shields[i]->current_shield);
            damage -= shield_damage;
        }

        // Apply remaining damage to health
        health->current_hp -= damage;
        health->current_hp = std::max(0, health->current_hp);

        if (health->current_hp > 0) continue;

        entities_to_kill.push_back(i);
        handle_entity_death(r, i, positions, drawables, scores);
    }

    for (size_t entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

} // namespace systems
