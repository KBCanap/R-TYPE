#include "GameLogic.hpp"
#include <algorithm>
#include <cmath>

void GameLogic::processEnemyShooting(float dt) {
    auto &enemies = _registry->get_components<Enemy>();
    auto &positions = _registry->get_components<Position>();

    for (size_t i = 0; i < enemies.size(); ++i) {
        auto &enemy_opt = enemies[i];
        auto &pos_opt = positions[i];

        if (!enemy_opt || !pos_opt)
            continue;

        Enemy &enemy = enemy_opt.value();
        Position &pos = pos_opt.value();

        if (enemy.enemy_type >= 100)
            continue;

        enemy.shoot_timer += dt;

        if (pos.x < 0.9f && enemy.shoot_timer >= enemy.shoot_interval) {
            enemy.shoot_timer = 0.0f;

            if (enemy.enemy_type == 11) {
                for (int j = -1; j <= 1; ++j) {
                    spawnProjectileAtAngle(pos.x - 0.02f, pos.y, j * 10.0f, false, 25);
                }
            } else {
                spawnProjectile(pos.x - 0.02f, pos.y, false, 25);
            }
        }
    }
}

void GameLogic::processBossShooting(float dt) {
    if (!_boss_active)
        return;

    auto &bosses = _registry->get_components<Boss>();
    auto &positions = _registry->get_components<Position>();
    auto &healths = _registry->get_components<Health>();
    auto &velocities = _registry->get_components<Velocity>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    if (!_boss_parts.empty()) {
        std::vector<entity> dead_parts;
        int alive_count = 0;
        entity last_alive_part = entity(0);

        float base_y = 0.5f;
        float amplitudes[3] = {0.08f, 0.1f, 0.067f};
        float frequencies[3] = {1.0f, 1.2f, 0.8f};

        for (entity part : _boss_parts) {
            auto &health_opt = healths[part];
            auto &pos_opt = positions[part];
            auto &boss_opt = bosses[part];

            if (!health_opt || !pos_opt || health_opt.value().current_hp <= 0) {
                dead_parts.push_back(part);
                continue;
            }

            alive_count++;
            last_alive_part = part;

            Position &pos = pos_opt.value();

            if (boss_opt) {
                Boss &boss = boss_opt.value();
                boss.phase_timer += dt;

                int idx = boss.phase;
                if (idx >= 0 && idx < 3) {
                    float amplitude = amplitudes[idx];
                    float frequency = frequencies[idx];
                    float offset_y = (idx == 1) ? -0.025f : 0.0f;
                    pos.y = base_y + offset_y + std::sin(boss.phase_timer * frequency * 2.0f * 3.14159f) * amplitude;
                }
            }
        }

        for (entity dead : dead_parts) {
            auto it = std::find(_boss_parts.begin(), _boss_parts.end(), dead);
            if (it != _boss_parts.end()) {
                auto &net_opt = network_comps[dead];
                if (net_opt) {
                    _destroyed_net_ids.push_back(net_opt.value().net_id);
                }
                if (dead == _boss) {
                    _boss = entity(0);
                }
                _registry->kill_entity(dead);
                _boss_parts.erase(it);
            }
        }

        if (_boss_parts.empty()) {
            _boss_active = false;
            _boss = entity(0);
            if (!_endless_mode) {
                _level_complete = true;
            }
            return;
        }

        if (alive_count == 1 && last_alive_part != entity(0)) {
            auto &boss_opt = bosses[last_alive_part];
            auto &pos_opt = positions[last_alive_part];

            if (boss_opt && pos_opt) {
                Boss &boss = boss_opt.value();
                Position &pos = pos_opt.value();

                if (boss.projectile_count == 0) {
                    boss.projectile_count = 5;
                    boss.spread_angle = 30.0f;
                }

                boss.shoot_timer += dt;
                if (boss.shoot_timer >= 1.0f) {
                    boss.shoot_timer = 0.0f;
                    float angle_step = boss.spread_angle / (boss.projectile_count - 1);
                    float start_angle = -boss.spread_angle / 2.0f;
                    for (int i = 0; i < boss.projectile_count; ++i) {
                        float angle = start_angle + i * angle_step;
                        spawnProjectileAtAngle(pos.x - 0.05f, pos.y, 180.0f + angle, false, 30);
                    }
                }
            }
        }

        if (alive_count == 3 && _boss != entity(0)) {
            auto &boss_opt = bosses[_boss];
            auto &pos_opt = positions[_boss];

            if (boss_opt && pos_opt) {
                Boss &boss = boss_opt.value();
                Position &pos = pos_opt.value();

                boss.shoot_timer += dt;
                if (boss.shoot_timer >= 1.5f) {
                    boss.shoot_timer = 0.0f;
                    float angle_step = boss.spread_angle / (boss.projectile_count - 1);
                    float start_angle = -boss.spread_angle / 2.0f;
                    for (int i = 0; i < boss.projectile_count; ++i) {
                        float angle = start_angle + i * angle_step;
                        spawnProjectileAtAngle(pos.x - 0.05f, pos.y, 180.0f + angle, false, 30);
                    }
                }
            }
        }

        return;
    }

    auto &boss_opt = bosses[_boss];
    auto &pos_opt = positions[_boss];
    auto &health_opt = healths[_boss];

    if (!boss_opt || !pos_opt || !health_opt)
        return;

    Boss &boss = boss_opt.value();
    Position &pos = pos_opt.value();

    if (health_opt.value().current_hp <= 0) {
        _boss_active = false;
        if (!_endless_mode) {
            _level_complete = true;
        }
        return;
    }

    auto &vel_opt = velocities[_boss];
    if (vel_opt) {
        if (pos.y <= 0.1f) {
            vel_opt.value().vy = std::abs(vel_opt.value().vy);
            pos.y = 0.1f;
        } else if (pos.y >= 0.9f) {
            vel_opt.value().vy = -std::abs(vel_opt.value().vy);
            pos.y = 0.9f;
        }
    }

    boss.shoot_timer += dt;
    float fire_interval = (boss.boss_type == 1) ? 2.0f : 1.5f;

    if (boss.shoot_timer >= fire_interval) {
        boss.shoot_timer = 0.0f;

        float angle_step = boss.spread_angle / (boss.projectile_count - 1);
        float start_angle = -boss.spread_angle / 2.0f;

        for (int i = 0; i < boss.projectile_count; ++i) {
            float angle = start_angle + i * angle_step;
            spawnProjectileAtAngle(pos.x - 0.05f, pos.y, 180.0f + angle, false, 30);
        }
    }
}

void GameLogic::processPowerUpCollisions() {
    auto &positions = _registry->get_components<Position>();
    auto &hitboxes = _registry->get_components<Hitbox>();
    auto &powerup_comps = _registry->get_components<PowerUp>();
    auto &weapons = _registry->get_components<Weapon>();
    auto &shields = _registry->get_components<Shield>();
    auto &players = _registry->get_components<PlayerComponent>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    std::vector<entity> powerups_to_remove;

    for (entity powerup : _powerups) {
        auto &pu_pos_opt = positions[powerup];
        auto &pu_hitbox_opt = hitboxes[powerup];
        auto &pu_opt = powerup_comps[powerup];

        if (!pu_pos_opt || !pu_hitbox_opt || !pu_opt)
            continue;

        Position &pu_pos = pu_pos_opt.value();
        Hitbox &pu_hitbox = pu_hitbox_opt.value();
        PowerUp &pu = pu_opt.value();

        for (auto &[client_id, player_ent] : _client_to_entity) {
            auto &player_pos_opt = positions[player_ent];
            auto &player_hitbox_opt = hitboxes[player_ent];
            auto &player_opt = players[player_ent];

            if (!player_pos_opt || !player_hitbox_opt || !player_opt)
                continue;
            if (!player_opt.value().is_active)
                continue;

            Position &player_pos = player_pos_opt.value();
            Hitbox &player_hitbox = player_hitbox_opt.value();

            float pu_w = pu_hitbox.width / 800.0f;
            float pu_h = pu_hitbox.height / 600.0f;
            float pl_w = player_hitbox.width / 800.0f;
            float pl_h = player_hitbox.height / 600.0f;

            bool collision = !(pu_pos.x + pu_w < player_pos.x ||
                              pu_pos.x > player_pos.x + pl_w ||
                              pu_pos.y + pu_h < player_pos.y ||
                              pu_pos.y > player_pos.y + pl_h);

            if (collision) {
                if (pu.type == PowerUpType::SHIELD) {
                    auto &shield_opt = shields[player_ent];
                    if (shield_opt) {
                        shield_opt.value().current_shield = std::min(
                            shield_opt.value().current_shield + 50,
                            shield_opt.value().max_shield);
                    } else {
                        _registry->add_component(player_ent, Shield{50, 50});
                    }
                } else if (pu.type == PowerUpType::SPREAD) {
                    auto &weapon_opt = weapons[player_ent];
                    if (weapon_opt) {
                        weapon_opt.value().projectile_count = 3;
                        weapon_opt.value().spread_angle = 15.0f;
                    }
                }

                powerups_to_remove.push_back(powerup);
                break;
            }
        }

        if (pu_pos.x < -0.1f) {
            powerups_to_remove.push_back(powerup);
        }
    }

    for (entity pu : powerups_to_remove) {
        auto &net_opt = network_comps[pu];
        if (net_opt) {
            _destroyed_net_ids.push_back(net_opt.value().net_id);
        }
        _registry->kill_entity(pu);
        _powerups.erase(std::remove(_powerups.begin(), _powerups.end(), pu), _powerups.end());
    }
}

void GameLogic::cleanupDeadEntities() {
    auto &healths = _registry->get_components<Health>();
    auto &network_comps = _registry->get_components<NetworkComponent>();
    auto &positions = _registry->get_components<Position>();
    auto &projectile_comps = _registry->get_components<Projectile>();

    std::vector<entity> to_kill;

    for (auto it = _client_to_entity.begin(); it != _client_to_entity.end(); ) {
        entity ent = it->second;
        auto &health_opt = healths[ent];
        if (health_opt && health_opt.value().current_hp <= 0) {
            to_kill.push_back(ent);
            it = _client_to_entity.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = _enemies.begin(); it != _enemies.end(); ) {
        entity ent = *it;
        auto &health_opt = healths[ent];
        auto &pos_opt = positions[ent];

        bool should_remove = !health_opt || !pos_opt ||
                            health_opt.value().current_hp <= 0 ||
                            pos_opt.value().x <= 0.0f;

        if (should_remove) {
            to_kill.push_back(ent);
            it = _enemies.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = _projectiles.begin(); it != _projectiles.end(); ) {
        entity ent = *it;
        auto &pos_opt = positions[ent];
        auto &proj_opt = projectile_comps[ent];

        bool should_remove = !pos_opt || !proj_opt ||
                            pos_opt.value().x < -0.1f ||
                            pos_opt.value().x > 1.1f ||
                            pos_opt.value().y < -0.1f ||
                            pos_opt.value().y > 1.1f ||
                            proj_opt.value().lifetime <= 0.0f;

        if (should_remove) {
            to_kill.push_back(ent);
            it = _projectiles.erase(it);
        } else {
            ++it;
        }
    }

    if ((_level_id == 1 || _endless_mode) && _boss != entity(0) && _boss_parts.empty()) {
        auto &boss_health_opt = healths[_boss];
        if (!boss_health_opt || boss_health_opt.value().current_hp <= 0) {
            to_kill.push_back(_boss);
            _boss_active = false;
            _boss = entity(0);
        }
    }

    for (entity ent : to_kill) {
        auto &net_opt = network_comps[ent];
        if (net_opt) {
            _destroyed_net_ids.push_back(net_opt.value().net_id);
        }
        _registry->kill_entity(ent);
    }
}
