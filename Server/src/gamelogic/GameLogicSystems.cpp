#include "gamelogic/GameLogic.hpp"
#include "../../ecs/include/GameConstants.hpp"
#include <algorithm>
#include <cmath>

void GameLogic::inputSystem(registry &reg, sparse_array<InputState> &inputs,
                            sparse_array<Velocity> &velocities,
                            sparse_array<PlayerComponent> &players, float dt) {
    const float MOVE_SPEED = 0.5f;

    for (size_t i = 0; i < players.size(); ++i) {
        auto &player_opt = players[i];
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto &vel_opt = velocities[i];
        auto &input_opt = inputs[i];

        if (vel_opt && input_opt) {
            Velocity &vel = vel_opt.value();
            InputState &input = input_opt.value();

            vel.vx = 0.0f;
            vel.vy = 0.0f;

            if (input.up)    vel.vy -= MOVE_SPEED;
            if (input.down)  vel.vy += MOVE_SPEED;
            if (input.left)  vel.vx -= MOVE_SPEED;
            if (input.right) vel.vx += MOVE_SPEED;

            if ((input.up || input.down) && (input.left || input.right)) {
                float length = std::sqrt(vel.vx * vel.vx + vel.vy * vel.vy);
                if (length > 0) {
                    vel.vx = (vel.vx / length) * MOVE_SPEED;
                    vel.vy = (vel.vy / length) * MOVE_SPEED;
                }
            }
        }
    }
}

void GameLogic::movementSystem(registry &reg, sparse_array<Position> &positions,
                               sparse_array<Velocity> &velocities, float dt) {
    auto &players = reg.get_components<PlayerComponent>();

    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto &pos_opt = positions[i];
        auto &vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            Position &pos = pos_opt.value();
            Velocity &vel = vel_opt.value();

            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;

            bool is_player = (i < players.size()) && players[i].has_value();
            if (is_player) {
                pos.x = std::max(0.0f, std::min(pos.x, 1.0f));
                pos.y = std::max(0.0f, std::min(pos.y, 1.0f));
            }
        }
    }
}

void GameLogic::weaponSystem(registry &reg, sparse_array<Weapon> &weapons,
                             sparse_array<Position> &positions,
                             sparse_array<InputState> &inputs,
                             sparse_array<PlayerComponent> &players,
                             float game_time) {
    for (size_t i = 0; i < weapons.size(); ++i) {
        auto &weapon_opt = weapons[i];
        if (weapon_opt) {
            if (weapon_opt.value().fire_timer > 0.0f) {
                weapon_opt.value().fire_timer -= game_time;
            }
        }
    }
}

void GameLogic::projectileSystem(registry &reg,
                                 sparse_array<Projectile> &projectiles,
                                 sparse_array<Position> &positions, float dt) {
    for (size_t i = 0; i < projectiles.size(); ++i) {
        auto &proj_opt = projectiles[i];
        if (proj_opt) {
            proj_opt.value().lifetime -= dt;
        }
    }
}

int GameLogic::applyDamageWithShield(registry &reg, size_t entity_idx, int damage) {
    auto &shields = reg.get_components<Shield>();
    auto &shield_opt = shields[entity_idx];

    int remaining_damage = damage;
    if (shield_opt && shield_opt.value().current_shield > 0) {
        int shield_damage = std::min(remaining_damage, shield_opt.value().current_shield);
        shield_opt.value().current_shield -= shield_damage;
        remaining_damage -= shield_damage;
    }
    return remaining_damage;
}

void GameLogic::processProjectileCollisions(
    registry &reg, sparse_array<Position> &positions,
    sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
    sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
    sparse_array<Health> &healths, sparse_array<Score> &scores,
    sparse_array<NetworkComponent> &network_comps) {

    for (size_t proj_idx = 0; proj_idx < projectiles.size(); ++proj_idx) {
        auto &proj_opt = projectiles[proj_idx];
        auto &proj_pos_opt = positions[proj_idx];
        auto &proj_hitbox_opt = hitboxes[proj_idx];

        if (!proj_opt || !proj_pos_opt)
            continue;

        Projectile &proj = proj_opt.value();
        Position &proj_pos = proj_pos_opt.value();

        float proj_width = proj_hitbox_opt ? proj_hitbox_opt.value().width : 8.0f;
        float proj_height = proj_hitbox_opt ? proj_hitbox_opt.value().height : 8.0f;

        for (size_t target_idx = 0; target_idx < positions.size(); ++target_idx) {
            if (target_idx == proj_idx)
                continue;

            auto &target_pos_opt = positions[target_idx];
            auto &target_hitbox_opt = hitboxes[target_idx];
            auto &target_health_opt = healths[target_idx];

            if (!target_pos_opt || !target_hitbox_opt || !target_health_opt)
                continue;

            Position &target_pos = target_pos_opt.value();
            Hitbox &target_hitbox = target_hitbox_opt.value();

            bool target_is_player = players[target_idx].has_value();
            bool target_is_enemy = enemies[target_idx].has_value();

            bool valid_hit = (proj.is_player_projectile && target_is_enemy) ||
                            (!proj.is_player_projectile && target_is_player);

            if (!valid_hit)
                continue;

            float target_w = target_hitbox.width / 800.0f;
            float target_h = target_hitbox.height / 600.0f;
            float proj_w = proj_width / 800.0f;
            float proj_h = proj_height / 600.0f;

            bool collision =
                proj_pos.x < target_pos.x + target_w &&
                proj_pos.x + proj_w > target_pos.x &&
                proj_pos.y < target_pos.y + target_h &&
                proj_pos.y + proj_h > target_pos.y;

            if (collision) {
                Health &target_health = target_health_opt.value();

                if (target_health.current_hp <= 0) {
                    continue;
                }

                int hp_before = target_health.current_hp;
                int remaining_damage = proj.damage;

                if (target_is_player) {
                    remaining_damage = applyDamageWithShield(reg, target_idx, remaining_damage);
                }

                target_health.current_hp -= remaining_damage;

                auto &target_net_opt = network_comps[target_idx];
                if (target_net_opt) {
                    target_net_opt.value().needs_update = true;
                }

                if (proj.is_player_projectile && target_is_enemy && hp_before > 0 && target_health.current_hp <= 0) {
                    auto &enemy_opt = enemies[target_idx];
                    if (enemy_opt) {
                        int score_awarded = enemy_opt.value().score_value;
                        for (size_t p_idx = 0; p_idx < players.size(); ++p_idx) {
                            auto &p_opt = players[p_idx];
                            auto &score_opt = scores[p_idx];
                            if (p_opt && score_opt) {
                                score_opt.value().current_score += score_awarded;
                                break;
                            }
                        }
                    }
                }

                proj.lifetime = 0.0f;
                break;
            }
        }
    }
}

void GameLogic::processContactCollisions(
    registry &reg, sparse_array<Position> &positions,
    sparse_array<Hitbox> &hitboxes, sparse_array<PlayerComponent> &players,
    sparse_array<Enemy> &enemies, sparse_array<Health> &healths,
    sparse_array<NetworkComponent> &network_comps) {

    const int CONTACT_DAMAGE = game::CONTACT_DAMAGE;

    for (size_t player_idx = 0; player_idx < players.size(); ++player_idx) {
        auto &player_opt = players[player_idx];
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto &player_pos_opt = positions[player_idx];
        auto &player_hitbox_opt = hitboxes[player_idx];
        auto &player_health_opt = healths[player_idx];

        if (!player_pos_opt || !player_hitbox_opt || !player_health_opt)
            continue;

        if (player_health_opt.value().invulnerability_timer > 0.0f)
            continue;

        Position &player_pos = player_pos_opt.value();
        Hitbox &player_hitbox = player_hitbox_opt.value();

        float player_w = player_hitbox.width / 800.0f;
        float player_h = player_hitbox.height / 600.0f;

        for (size_t enemy_idx = 0; enemy_idx < enemies.size(); ++enemy_idx) {
            auto &enemy_opt = enemies[enemy_idx];
            if (!enemy_opt)
                continue;

            auto &enemy_pos_opt = positions[enemy_idx];
            auto &enemy_hitbox_opt = hitboxes[enemy_idx];
            auto &enemy_health_opt = healths[enemy_idx];

            if (!enemy_pos_opt || !enemy_hitbox_opt || !enemy_health_opt)
                continue;

            Position &enemy_pos = enemy_pos_opt.value();
            Hitbox &enemy_hitbox = enemy_hitbox_opt.value();

            float enemy_w = enemy_hitbox.width / 800.0f;
            float enemy_h = enemy_hitbox.height / 600.0f;

            bool collision =
                player_pos.x < enemy_pos.x + enemy_w &&
                player_pos.x + player_w > enemy_pos.x &&
                player_pos.y < enemy_pos.y + enemy_h &&
                player_pos.y + player_h > enemy_pos.y;

            if (collision) {
                int remaining_damage = applyDamageWithShield(reg, player_idx, CONTACT_DAMAGE);

                player_health_opt.value().current_hp -= remaining_damage;
                enemy_health_opt.value().current_hp -= CONTACT_DAMAGE;

                player_health_opt.value().invulnerability_timer = 0.5f;

                auto &player_net_opt = network_comps[player_idx];
                if (player_net_opt) {
                    player_net_opt.value().needs_update = true;
                }
                auto &enemy_net_opt = network_comps[enemy_idx];
                if (enemy_net_opt) {
                    enemy_net_opt.value().needs_update = true;
                }

                break;
            }
        }
    }
}

void GameLogic::collisionSystem(
    registry &reg, sparse_array<Position> &positions,
    sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
    sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
    sparse_array<Health> &healths, sparse_array<Score> &scores,
    sparse_array<NetworkComponent> &network_comps, float dt) {

    processProjectileCollisions(reg, positions, hitboxes, projectiles, players,
                                enemies, healths, scores, network_comps);

    processContactCollisions(reg, positions, hitboxes, players, enemies,
                            healths, network_comps);
}

void GameLogic::healthSystem(registry &reg, sparse_array<Health> &healths,
                             sparse_array<NetworkComponent> &network_comps,
                             float dt) {
    for (size_t i = 0; i < healths.size(); ++i) {
        auto &health_opt = healths[i];
        if (health_opt) {
            if (health_opt.value().invulnerability_timer > 0.0f) {
                health_opt.value().invulnerability_timer -= dt;
            }

            if (health_opt.value().current_hp <= 0) {
                auto &net_opt = network_comps[i];
                if (net_opt) {
                    net_opt.value().needs_update = true;
                }
            }
        }
    }
}

void GameLogic::enemyAISystem(registry &reg, sparse_array<Enemy> &enemies,
                              sparse_array<Position> &positions,
                              sparse_array<Velocity> &velocities, float dt) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto &enemy_opt = enemies[i];
        if (!enemy_opt)
            continue;

        auto &pos_opt = positions[i];
        auto &vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            Enemy &enemy = enemy_opt.value();
            Velocity &vel = vel_opt.value();

            enemy.pattern_timer += dt;

            if (enemy.enemy_type == 1) {
                vel.vy = std::sin(enemy.pattern_timer * 3.0f) * 0.3f;
            }
        }
    }
}

void GameLogic::bossAISystem(registry &reg, sparse_array<Boss> &bosses,
                             sparse_array<Position> &positions,
                             sparse_array<Velocity> &velocities,
                             sparse_array<Health> &healths, float dt) {
}
