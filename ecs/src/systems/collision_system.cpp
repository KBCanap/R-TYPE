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

// ===============================
// STRUCTURES AND BASIC HELPERS
// ===============================

struct HitboxDimensions {
    float width, height, left, top;
};

static bool is_valid_player(const std::optional<component::position> &pos,
                           const std::optional<component::drawable> &drawable) {
    return pos && drawable && (drawable->tag == "player");
}

static const component::hitbox* get_hitbox_ptr(size_t idx, 
                                              sparse_array<component::hitbox> &hitboxes) {
    return (idx < hitboxes.size() && hitboxes[idx]) ? &(*hitboxes[idx]) : nullptr;
}

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

// ===============================
// POWERUP HELPERS
// ===============================

static void handle_shield_powerup_collision(registry &r, size_t player_idx) {
    sparse_array<component::shield> &shields = r.get_components<component::shield>();
    
    if (player_idx >= shields.size() || !shields[player_idx]) {
        r.add_component<component::shield>(entity(player_idx), component::shield(50, 50));
    } else {
        shields[player_idx]->current_shield = std::min(
            shields[player_idx]->current_shield + 50,
            shields[player_idx]->max_shield
        );
    }
}

static void handle_spread_powerup_collision(registry &r, size_t player_idx) {
    sparse_array<component::weapon> &weapons = r.get_components<component::weapon>();
    
    if (player_idx < weapons.size() && weapons[player_idx]) {
        weapons[player_idx]->projectile_count = 3;
        weapons[player_idx]->spread_angle = 15.0f;
    }
}

// ===============================
// DAMAGE HELPERS
// ===============================

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

static void handle_entity_damage(size_t entity_idx, int damage,
                                sparse_array<component::health> &healths,
                                sparse_array<component::position> &positions,
                                std::vector<size_t> &entities_to_kill,
                                std::vector<std::pair<float, float>> &explosion_positions) {
    
    bool has_health = (entity_idx < healths.size()) && healths[entity_idx];
    
    if (has_health) {
        healths[entity_idx]->pending_damage += damage;
    } else {
        entities_to_kill.push_back(entity_idx);
        if (entity_idx < positions.size() && positions[entity_idx]) {
            explosion_positions.push_back({positions[entity_idx]->x, positions[entity_idx]->y});
        }
    }
}

// ===============================
// SPECIALIZED COLLISION LOGIC
// ===============================

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

static bool process_powerup_collision(registry &r, size_t player_idx, size_t powerup_idx,
                                     const HitboxDimensions &player_box,
                                     sparse_array<component::position> &positions,
                                     sparse_array<component::drawable> &drawables,
                                     sparse_array<component::hitbox> &hitboxes,
                                     const std::string &powerup_tag,
                                     std::vector<size_t> &entities_to_kill) {
    
    std::optional<component::position> &powerup_pos = positions[powerup_idx];
    std::optional<component::drawable> &powerup_drawable = drawables[powerup_idx];

    if (!powerup_pos || !powerup_drawable || powerup_drawable->tag != powerup_tag || powerup_idx == player_idx) {
        return false;
    }

    const component::hitbox *pwhitbox = get_hitbox_ptr(powerup_idx, hitboxes);
    HitboxDimensions powerup_box = calculate_target_hitbox(*powerup_pos, *powerup_drawable, pwhitbox);

    bool collision = check_aabb_collision(
        player_box.left, player_box.top, player_box.width, player_box.height,
        powerup_box.left, powerup_box.top, powerup_box.width, powerup_box.height);

    if (collision) {
        if (powerup_tag == "powerup") {
            handle_shield_powerup_collision(r, player_idx);
        } else if (powerup_tag == "spread_powerup") {
            handle_spread_powerup_collision(r, player_idx);
        }
        entities_to_kill.push_back(powerup_idx);
        return true;
    }
    
    return false;
}

static void handle_platformer_stomp(registry &r, size_t player_idx, size_t enemy_idx,
                                   sparse_array<component::velocity> &velocities,
                                   sparse_array<component::enemy_stunned> &stunneds) {
    std::optional<component::enemy_stunned> &enemy_stunned = stunneds[enemy_idx];
    std::optional<component::velocity> &player_vel = velocities[player_idx];
    std::optional<component::velocity> &enemy_vel = velocities[enemy_idx];

    // Stun the enemy
    if (enemy_stunned) {
        enemy_stunned->stunned = true;
        enemy_stunned->knockback_velocity = 0.0f;
    }

    // Stop enemy movement
    if (enemy_vel) {
        enemy_vel->vx = 0.0f;
        enemy_vel->vy = 0.0f;
    }

    if (player_vel) {
        player_vel->vy = -500.0f;
    }
}

static void handle_platformer_stunned_collision(size_t player_idx, size_t enemy_idx,
                                               const component::position &player_pos,
                                               const component::position &enemy_pos,
                                               const component::hitbox &player_hitbox,
                                               const component::hitbox &enemy_hitbox,
                                               sparse_array<component::velocity> &velocities,
                                               sparse_array<component::enemy_stunned> &stunneds) {
    std::optional<component::velocity> &player_vel = velocities[player_idx];
    std::optional<component::enemy_stunned> &enemy_stunned = stunneds[enemy_idx];

    float player_center_x = player_pos.x + player_hitbox.width / 2.0f;
    float enemy_center_x = enemy_pos.x + enemy_hitbox.width / 2.0f;
    float knockback_dir = (enemy_center_x > player_center_x) ? 1.0f : -1.0f;

    if (enemy_stunned) {
        enemy_stunned->knockback_velocity = knockback_dir * 600.0f;
    }
    
    if (player_vel) {
        player_vel->vx = -knockback_dir * 100.0f;
    }
}

static void handle_platformer_death(registry &r, size_t player_idx,
                                   sparse_array<component::velocity> &velocities,
                                   sparse_array<component::dead> &deads) {
    bool already_dead = (player_idx < deads.size()) && deads[player_idx];

    if (!already_dead) {
        r.add_component<component::dead>(entity(player_idx), component::dead(0.0f, -800.0f));

        std::optional<component::velocity> &player_vel = velocities[player_idx];
        if (player_vel) {
            player_vel->vy = -800.0f;
            player_vel->vx = 0.0f;
        }
    }
}

// ===============================
// MAIN COLLISION PROCESSORS
// ===============================

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

static void process_player_powerup_collisions(registry &r, size_t player_idx,
                                            const HitboxDimensions &player_box,
                                            sparse_array<component::position> &positions,
                                            sparse_array<component::drawable> &drawables,
                                            sparse_array<component::hitbox> &hitboxes,
                                            std::vector<size_t> &entities_to_kill) {
    
    size_t max_entities = std::min(positions.size(), drawables.size());
    
    for (size_t powerup_idx = 0; powerup_idx < max_entities; ++powerup_idx) {
        // Shield powerup
        process_powerup_collision(r, player_idx, powerup_idx, player_box, positions, 
                                 drawables, hitboxes, "powerup", entities_to_kill);
        
        // Spread powerup
        process_powerup_collision(r, player_idx, powerup_idx, player_box, positions, 
                                 drawables, hitboxes, "spread_powerup", entities_to_kill);
    }
}

static void process_player_enemy_collisions(registry &r,
                                           sparse_array<component::position> &positions,
                                           sparse_array<component::drawable> &drawables,
                                           sparse_array<component::hitbox> &hitboxes,
                                           sparse_array<component::health> &healths,
                                           std::vector<size_t> &entities_to_kill,
                                           std::vector<std::pair<float, float>> &explosion_positions) {
    
    const int COLLISION_DAMAGE = 50;
    size_t max_entities = std::min(positions.size(), drawables.size());

    sparse_array<component::velocity> &velocities = r.get_components<component::velocity>();
    sparse_array<component::gravity> &gravities = r.get_components<component::gravity>();
    sparse_array<component::enemy_stunned> &stunneds = r.get_components<component::enemy_stunned>();
    sparse_array<component::dead> &deads = r.get_components<component::dead>();

    for (size_t player_idx = 0; player_idx < max_entities; ++player_idx) {
        std::optional<component::position> &player_pos = positions[player_idx];
        std::optional<component::drawable> &player_drawable = drawables[player_idx];
        std::optional<component::hitbox> &player_hitbox = hitboxes[player_idx];

        if (!is_valid_player(player_pos, player_drawable) || !player_hitbox) continue;

        bool player_already_dead = (player_idx < deads.size()) && deads[player_idx];
        if (player_already_dead) continue;

        float player_left = player_pos->x + player_hitbox->offset_x;
        float player_top = player_pos->y + player_hitbox->offset_y;

        for (size_t enemy_idx = 0; enemy_idx < max_entities; ++enemy_idx) {
            if (enemy_idx == player_idx) continue;
            
            std::optional<component::position> &enemy_pos = positions[enemy_idx];
            std::optional<component::drawable> &enemy_drawable = drawables[enemy_idx];
            std::optional<component::hitbox> &enemy_hitbox = hitboxes[enemy_idx];

            if (!enemy_pos || !enemy_drawable || !enemy_hitbox || !is_enemy_tag(enemy_drawable->tag)) continue;

            float enemy_left = enemy_pos->x + enemy_hitbox->offset_x;
            float enemy_top = enemy_pos->y + enemy_hitbox->offset_y;

            bool collision = check_aabb_collision(
                player_left, player_top, player_hitbox->width, player_hitbox->height,
                enemy_left, enemy_top, enemy_hitbox->width, enemy_hitbox->height);

            if (!collision) continue;

            bool has_gravity = (player_idx < gravities.size()) && gravities[player_idx];
            bool has_velocity = (player_idx < velocities.size()) && velocities[player_idx];
            bool is_platformer = has_gravity && has_velocity;

            if (is_platformer) {
                std::optional<component::velocity> &player_vel = velocities[player_idx];
                std::optional<component::enemy_stunned> &enemy_stunned = stunneds[enemy_idx];

                if (enemy_stunned && enemy_stunned->stunned) {
                    handle_platformer_stunned_collision(player_idx, enemy_idx, *player_pos, 
                                                       *enemy_pos, *player_hitbox, *enemy_hitbox,
                                                       velocities, stunneds);
                } else {
                    float player_bottom = player_top + player_hitbox->height;
                    float stomp_threshold = enemy_top + enemy_hitbox->height * 0.5f;

                    bool is_stomping = player_vel && player_vel->vy > 0.0f &&
                                      player_bottom >= enemy_top &&
                                      player_bottom <= stomp_threshold;

                    if (is_stomping) {
                        handle_platformer_stomp(r, player_idx, enemy_idx, velocities, stunneds);
                    } else {
                        handle_platformer_death(r, player_idx, velocities, deads);
                    }
                }
                break; // Exit enemy loop after handling platformer collision
            }

            handle_entity_damage(player_idx, COLLISION_DAMAGE, healths, positions, 
                                entities_to_kill, explosion_positions);
            handle_entity_damage(enemy_idx, COLLISION_DAMAGE, healths, positions, 
                                entities_to_kill, explosion_positions);
            break;
        }
    }
}

// ===============================
// MAIN SYSTEM
// ===============================

void collision_system(registry &r, sparse_array<component::position> &positions,
                      sparse_array<component::drawable> &drawables,
                      sparse_array<component::projectile> &projectiles,
                      sparse_array<component::hitbox> &hitboxes) {
    
    std::vector<size_t> entities_to_kill;
    std::vector<std::pair<float, float>> explosion_positions;
    sparse_array<component::score> &scores = r.get_components<component::score>();
    sparse_array<component::health> &healths = r.get_components<component::health>();

    // Process projectile collisions
    for (size_t proj_idx = 0; proj_idx < projectiles.size(); ++proj_idx) {
        std::optional<component::projectile> &projectile = projectiles[proj_idx];
        std::optional<component::position> &proj_pos = positions[proj_idx];

        if (!projectile || !proj_pos) continue;

        size_t max_targets = std::min(positions.size(), drawables.size());
        for (size_t target_idx = 0; target_idx < max_targets; ++target_idx) {
            bool should_break = process_projectile_collision(
                proj_idx, target_idx, positions, drawables, projectiles, hitboxes,
                healths, scores, entities_to_kill, explosion_positions);

            if (should_break) {
                entities_to_kill.push_back(proj_idx);
                break;
            }
        }
    }

    // Process player-powerup collisions
    size_t max_entities = std::min(positions.size(), drawables.size());
    for (size_t player_idx = 0; player_idx < max_entities; ++player_idx) {
        std::optional<component::position> &player_pos = positions[player_idx];
        std::optional<component::drawable> &player_drawable = drawables[player_idx];

        if (!is_valid_player(player_pos, player_drawable)) continue;

        const component::hitbox *phitbox = get_hitbox_ptr(player_idx, hitboxes);
        HitboxDimensions player_box = calculate_target_hitbox(*player_pos, *player_drawable, phitbox);

        process_player_powerup_collisions(r, player_idx, player_box, positions, 
                                         drawables, hitboxes, entities_to_kill);
    }

    // Process player-enemy collisions
    process_player_enemy_collisions(r, positions, drawables, hitboxes, healths, 
                                   entities_to_kill, explosion_positions);

    // Cleanup and effects
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