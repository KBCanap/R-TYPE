#include "gamelogic/GameLogic.hpp"
#include <algorithm>

entity GameLogic::createPlayer(uint client_id, uint net_id, float x, float y) {
    entity player = _registry->spawn_entity();

    _registry->add_component(player, Position{x, y});
    _registry->add_component(player, Velocity{0.0f, 0.0f});
    _registry->add_component(player, InputState{});
    _registry->add_component(player, PlayerComponent{client_id, true, 0.0f});
    _registry->add_component(player, Health{100, 100, 0.0f});
    _registry->add_component(player, Score{0, 0.0f, 0.0f});
    _registry->add_component(player, Weapon{8.0f, 0.0f, 0, 25});
    _registry->add_component(player, Hitbox{66.0f, 34.0f, 15.0f, 0.0f});
    _registry->add_component(player, NetworkComponent{net_id, true, "player"});

    _client_to_entity.insert({client_id, player});

    return player;
}

void GameLogic::removePlayer(uint client_id) {
    // Clean up companion if this player had one
    auto comp_it = _player_companions.find(client_id);
    if (comp_it != _player_companions.end()) {
        auto &net_comps = _registry->get_components<NetworkComponent>();
        if (net_comps[comp_it->second])
            _destroyed_net_ids.push_back(net_comps[comp_it->second].value().net_id);
        _registry->kill_entity(comp_it->second);
        _player_companions.erase(comp_it);
    }

    auto it = _client_to_entity.find(client_id);
    if (it != _client_to_entity.end()) {
        _registry->kill_entity(it->second);
        _client_to_entity.erase(it);
    }
}

entity GameLogic::getPlayerEntity(uint client_id) {
    auto it = _client_to_entity.find(client_id);
    if (it != _client_to_entity.end())
        return it->second;
    return entity(static_cast<size_t>(-1));
}

uint GameLogic::generateNetId() { return _next_net_id++; }

std::vector<uint> GameLogic::getDestroyedEntities() {
    std::vector<uint> result = std::move(_destroyed_net_ids);
    _destroyed_net_ids.clear();
    return result;
}

std::vector<GameLogic::NewEntityInfo> GameLogic::getNewEntities() {
    std::vector<NewEntityInfo> result = std::move(_new_entities);
    _new_entities.clear();
    return result;
}

void GameLogic::updatePlayerScores(float dt) {
    const float SCORE_INTERVAL = 1.0f;

    auto &scores = _registry->get_components<Score>();
    auto &players = _registry->get_components<PlayerComponent>();

    for (size_t i = 0; i < players.size(); ++i) {
        auto &player_opt = players[i];

        if (!player_opt || !player_opt.value().is_active)
            continue;

        entity ent = _registry->entity_from_index(i);
        auto &score_opt = scores[ent];

        if (!score_opt)
            continue;

        score_opt.value().survival_time += dt;

        float time_since_award = score_opt.value().survival_time - score_opt.value().last_time_point_awarded;

        if (time_since_award >= SCORE_INTERVAL) {
            score_opt.value().current_score += 1;
            score_opt.value().last_time_point_awarded = score_opt.value().survival_time;
        }
    }
}

void GameLogic::updateTotalScore() {
    auto &scores = _registry->get_components<Score>();
    auto &players = _registry->get_components<PlayerComponent>();

    _total_score = 0;

    for (size_t i = 0; i < scores.size(); ++i) {
        auto &score_opt = scores[i];
        auto &player_opt = players[i];

        if (score_opt && player_opt && player_opt.value().is_active) {
            int player_score = score_opt.value().current_score;
            if (player_score > _total_score) {
                _total_score = player_score;
            }
        }
    }
}

void GameLogic::fireSpreadWeapon(const Position &pos, const Weapon &weapon) {
    float angle_step = weapon.spread_angle / (weapon.projectile_count - 1);
    float start_angle = -weapon.spread_angle / 2.0f;

    for (int j = 0; j < weapon.projectile_count; ++j) {
        float angle = start_angle + j * angle_step;
        spawnProjectileAtAngle(pos.x + 0.05f, pos.y, angle, true, weapon.damage);
    }
}

void GameLogic::fireSingleWeapon(const Position &pos, int damage) {
    spawnProjectile(pos.x + 0.05f, pos.y, true, damage);
}

void GameLogic::processPlayerShooting(float dt) {
    auto &inputs = _registry->get_components<InputState>();
    auto &positions = _registry->get_components<Position>();
    auto &weapons = _registry->get_components<Weapon>();
    auto &players = _registry->get_components<PlayerComponent>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    for (size_t i = 0; i < players.size(); ++i) {
        auto &player_opt = players[i];
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto &input_opt = inputs[i];
        auto &pos_opt = positions[i];
        auto &weapon_opt = weapons[i];

        if (!input_opt || !pos_opt || !weapon_opt)
            continue;

        InputState &input = input_opt.value();
        Weapon &weapon = weapon_opt.value();

        if (weapon.fire_timer > 0.0f) {
            weapon.fire_timer -= dt;
        }

        if (input.shoot && weapon.fire_timer <= 0.0f) {
            Position &pos = pos_opt.value();

            if (weapon.projectile_count > 1 && weapon.spread_angle > 0.0f) {
                fireSpreadWeapon(pos, weapon);
            } else {
                fireSingleWeapon(pos, weapon.damage);
            }

            weapon.fire_timer = 1.0f / weapon.fire_rate;
            input.shoot = false;

            auto &net_opt = network_comps[i];
            if (net_opt) {
                net_opt.value().needs_update = true;
            }
        }
    }
}
