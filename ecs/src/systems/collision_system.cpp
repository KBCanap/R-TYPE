/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** collision_system
*/

#include "../../include/systems.hpp"
#include <algorithm>
#include <vector>

namespace systems {

struct HitboxDimensions {
    float width, height, left, top;
};

static HitboxDimensions calculate_target_hitbox(
    const component::position &pos,
    const component::drawable &drawable,
    const component::hitbox *hitbox) {

    if (hitbox) {
        return {hitbox->width, hitbox->height,
                pos.x + hitbox->offset_x, pos.y + hitbox->offset_y};
    }

    if (drawable.use_sprite && drawable.sprite_rect.width > 0) {
        float w = drawable.sprite_rect.width * drawable.scale;
        float h = drawable.sprite_rect.height * drawable.scale;
        return {w, h, pos.x, pos.y};
    }

    return {drawable.size, drawable.size, pos.x, pos.y};
}

static bool check_aabb_collision(float left1, float top1, float width1, float height1,
                                  float left2, float top2, float width2, float height2) {
    return (left1 < left2 + width2) && (left1 + width1 > left2) &&
           (top1 < top2 + height2) && (top1 + height1 > top2);
}

static void award_enemy_kill_score(sparse_array<component::score> &scores,
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

static void handle_projectile_hit(
    size_t target_idx, const component::position &target_pos,
    const component::drawable &target_drawable,
    const component::projectile &projectile,
    sparse_array<component::health> &healths,
    sparse_array<component::score> &scores,
    sparse_array<component::drawable> &drawables,
    std::vector<size_t> &entities_to_kill,
    std::vector<std::pair<float, float>> &explosion_positions) {

    bool is_player = (target_drawable.tag == "player");
    bool is_enemy = is_enemy_tag(target_drawable.tag);
    bool is_boss = (target_drawable.tag == "boss");
    bool friendly_hits_enemy = projectile.friendly && is_enemy;

    bool has_health = (target_idx < healths.size()) && healths[target_idx];

    if (has_health) {
        healths[target_idx]->pending_damage += static_cast<int>(projectile.damage);

        if (is_player || (friendly_hits_enemy && is_boss)) {
            explosion_positions.push_back({target_pos.x, target_pos.y});
        }
        return;
    }

    entities_to_kill.push_back(target_idx);
    explosion_positions.push_back({target_pos.x, target_pos.y});

    if (friendly_hits_enemy) {
        award_enemy_kill_score(scores, drawables, 5);
    }
}

static bool process_projectile_collision(
    size_t proj_idx, size_t target_idx,
    sparse_array<component::position> &positions,
    sparse_array<component::drawable> &drawables,
    sparse_array<component::projectile> &projectiles,
    sparse_array<component::hitbox> &hitboxes,
    sparse_array<component::health> &healths,
    sparse_array<component::score> &scores,
    std::vector<size_t> &entities_to_kill,
    std::vector<std::pair<float, float>> &explosion_positions) {

    std::optional<component::position> &target_pos = positions[target_idx];
    std::optional<component::drawable> &target_drawable = drawables[target_idx];
    std::optional<component::projectile> &projectile = projectiles[proj_idx];
    std::optional<component::position> &proj_pos = positions[proj_idx];

    bool valid_target = target_pos && target_drawable &&
                       (target_idx != proj_idx) && !projectiles[target_idx];
    if (!valid_target) return false;

    const float proj_width = 13.0f;
    const float proj_height = 8.0f;

    const component::hitbox *hitbox = nullptr;
    if (target_idx < hitboxes.size() && hitboxes[target_idx]) {
        hitbox = &(*hitboxes[target_idx]);
    }
    HitboxDimensions target_box = calculate_target_hitbox(*target_pos, *target_drawable, hitbox);

    bool collision = check_aabb_collision(proj_pos->x, proj_pos->y, proj_width, proj_height,
                                          target_box.left, target_box.top,
                                          target_box.width, target_box.height);
    if (!collision) return false;

    bool is_player = (target_drawable->tag == "player");
    bool is_enemy = is_enemy_tag(target_drawable->tag);
    bool friendly_hits_enemy = projectile->friendly && is_enemy;
    bool enemy_hits_player = !projectile->friendly && is_player;

    if (!friendly_hits_enemy && !enemy_hits_player) return false;

    handle_projectile_hit(target_idx, *target_pos, *target_drawable, *projectile,
                         healths, scores, drawables, entities_to_kill, explosion_positions);

    projectile->hits += projectile->piercing;
    return !projectile->piercing;
}

void collision_system(registry &r, sparse_array<component::position> &positions,
                      sparse_array<component::drawable> &drawables,
                      sparse_array<component::projectile> &projectiles,
                      sparse_array<component::hitbox> &hitboxes) {
    std::vector<size_t> entities_to_kill;
    std::vector<std::pair<float, float>> explosion_positions;
    sparse_array<component::score> &scores = r.get_components<component::score>();
    sparse_array<component::health> &healths = r.get_components<component::health>();

    for (size_t proj_idx = 0; proj_idx < projectiles.size(); ++proj_idx) {
        std::optional<component::projectile> &projectile = projectiles[proj_idx];
        std::optional<component::position> &proj_pos = positions[proj_idx];

        if (!projectile || !proj_pos) continue;

        for (size_t target_idx = 0; target_idx < std::min(positions.size(), drawables.size()); ++target_idx) {
            bool should_break = process_projectile_collision(
                proj_idx, target_idx, positions, drawables, projectiles, hitboxes,
                healths, scores, entities_to_kill, explosion_positions);

            if (should_break) {
                entities_to_kill.push_back(proj_idx);
                break;
            }
        }
    }

    // Player-Powerup collision detection
    for (size_t player_idx = 0; player_idx < std::min(positions.size(), drawables.size()); ++player_idx) {
        std::optional<component::position> &player_pos = positions[player_idx];
        std::optional<component::drawable> &player_drawable = drawables[player_idx];
        std::optional<component::hitbox> &player_hitbox = hitboxes[player_idx];

        bool valid_player = player_pos && player_drawable &&
                           (player_drawable->tag == "player");
        if (!valid_player) continue;

        const component::hitbox *phitbox = nullptr;
        if (player_idx < hitboxes.size() && player_hitbox) {
            phitbox = &(*player_hitbox);
        }
        HitboxDimensions player_box = calculate_target_hitbox(*player_pos, *player_drawable, phitbox);

        for (size_t powerup_idx = 0; powerup_idx < std::min(positions.size(), drawables.size()); ++powerup_idx) {
            std::optional<component::position> &powerup_pos = positions[powerup_idx];
            std::optional<component::drawable> &powerup_drawable = drawables[powerup_idx];

            bool valid_powerup = powerup_pos && powerup_drawable &&
                                (powerup_drawable->tag == "powerup") && (powerup_idx != player_idx);
            if (!valid_powerup) continue;

            const component::hitbox *pwhitbox = nullptr;
            if (powerup_idx < hitboxes.size() && hitboxes[powerup_idx]) {
                pwhitbox = &(*hitboxes[powerup_idx]);
            }
            HitboxDimensions powerup_box = calculate_target_hitbox(*powerup_pos, *powerup_drawable, pwhitbox);

            bool collision = check_aabb_collision(
                player_box.left, player_box.top, player_box.width, player_box.height,
                powerup_box.left, powerup_box.top, powerup_box.width, powerup_box.height);

            if (collision) {
                // Add or create shield component with 50 shield points
                sparse_array<component::shield> &shields = r.get_components<component::shield>();
                if (player_idx >= shields.size() || !shields[player_idx]) {
                    r.add_component<component::shield>(entity(player_idx), component::shield(50, 50));
                } else {
                    // Add 50 shield points, max 50
                    shields[player_idx]->current_shield = std::min(
                        shields[player_idx]->current_shield + 50,
                        shields[player_idx]->max_shield
                    );
                }

                // Destroy powerup
                entities_to_kill.push_back(powerup_idx);
            }
        }
    }

    // Player-Enemy collision detection
    const int COLLISION_DAMAGE = 50;

    for (size_t player_idx = 0; player_idx < std::min(positions.size(), drawables.size()); ++player_idx) {
        std::optional<component::position> &player_pos = positions[player_idx];
        std::optional<component::drawable> &player_drawable = drawables[player_idx];
        std::optional<component::hitbox> &player_hitbox = hitboxes[player_idx];

        bool valid_player = player_pos && player_drawable && player_hitbox &&
                           (player_drawable->tag == "player");
        if (!valid_player) continue;

        float player_left = player_pos->x + player_hitbox->offset_x;
        float player_top = player_pos->y + player_hitbox->offset_y;

        for (size_t enemy_idx = 0; enemy_idx < std::min(positions.size(), drawables.size()); ++enemy_idx) {
            std::optional<component::position> &enemy_pos = positions[enemy_idx];
            std::optional<component::drawable> &enemy_drawable = drawables[enemy_idx];
            std::optional<component::hitbox> &enemy_hitbox = hitboxes[enemy_idx];

            bool valid_enemy = enemy_pos && enemy_drawable && enemy_hitbox &&
                              is_enemy_tag(enemy_drawable->tag) && (enemy_idx != player_idx);
            if (!valid_enemy) continue;

            float enemy_left = enemy_pos->x + enemy_hitbox->offset_x;
            float enemy_top = enemy_pos->y + enemy_hitbox->offset_y;

            bool collision = check_aabb_collision(
                player_left, player_top, player_hitbox->width, player_hitbox->height,
                enemy_left, enemy_top, enemy_hitbox->width, enemy_hitbox->height);

            if (!collision) continue;

            bool player_has_health = (player_idx < healths.size()) && healths[player_idx];
            if (player_has_health) {
                healths[player_idx]->pending_damage += COLLISION_DAMAGE;
            } else {
                entities_to_kill.push_back(player_idx);
                explosion_positions.push_back({player_pos->x, player_pos->y});
            }

            bool enemy_has_health = (enemy_idx < healths.size()) && healths[enemy_idx];
            if (enemy_has_health) {
                healths[enemy_idx]->pending_damage += COLLISION_DAMAGE;
            } else {
                entities_to_kill.push_back(enemy_idx);
                explosion_positions.push_back({enemy_pos->x, enemy_pos->y});
            }

            break;
        }
    }

    std::sort(entities_to_kill.begin(), entities_to_kill.end());
    entities_to_kill.erase(
        std::unique(entities_to_kill.begin(), entities_to_kill.end()),
        entities_to_kill.end());

    for (size_t entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }

    for (const std::pair<float, float> &explosion_pos : explosion_positions) {
        create_explosion(r, explosion_pos.first, explosion_pos.second);
    }
}

} // namespace systems
