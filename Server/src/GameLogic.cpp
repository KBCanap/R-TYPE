/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameLogic
*/

#include "GameLogic.hpp"
#include "components.hpp"
#include "systems.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sys/types.h>

GameLogic::GameLogic(std::shared_ptr<registry> reg)
    : _registry(reg), _running(false), _current_tick(0), _game_time(0.0f),
      _enemy_spawn_timer(0.0f), _enemy_spawn_interval(2.0f), _total_score(0),
      _boss_spawned(false), _boss(0), _next_net_id(1000) {
    _last_update = std::chrono::steady_clock::now();
    _rng.seed(std::random_device{}());
    registerSystems();
}

GameLogic::~GameLogic() {
    stop();
}

void GameLogic::start() {
    _running = true;
    _last_update = std::chrono::steady_clock::now();

    std::cout << "[GameLogic] Started" << std::endl;
}

void GameLogic::stop() {
    _running = false;
    std::cout << "[GameLogic] Stopped" << std::endl;
}

void GameLogic::registerSystems() {
    using namespace systems;

    _registry->add_system<component::controllable, component::velocity, component::input>(control_system);
    _registry->add_system<component::health>(health_system);
    _registry->add_system<component::ai_input>(ai_input_system);
    _registry->add_system<component::score>(score_system);
}

void GameLogic::updatePlayerInputState(uint client_id, uint8_t direction) {
    auto it = _client_to_entity.find(client_id);
    if (it == _client_to_entity.end())
        return;
    auto &inputs = _registry->get_components<component::input>();
    _last_input_time[client_id] = std::chrono::steady_clock::now();
    component::input &i = inputs[it->second].value();
    i.up = direction & 0x01;
    i.down = direction & 0x02;
    i.left = direction & 0x04;
    i.right = direction & 0x08;
    i.fire = direction & 0x10;
}

void GameLogic::update(float deltaTime) {
    if (!_running)
        return;

    static const float FIXED_TIMESTEP = 1.0f / 60.0f;
    static float accumulator = 0.0f;
    static float debug_timer = 0.0f;
    static const float DEBUG_INTERVAL = 1.0f;

    accumulator += deltaTime;
    debug_timer += deltaTime;

    while (accumulator >= FIXED_TIMESTEP) {
        _current_tick++;
        _game_time += FIXED_TIMESTEP;

        DummyRenderWindow window(800, 600);
        systems::position_system(*_registry,
                         _registry->get_components<component::position>(),
                         _registry->get_components<component::velocity>(),
                         _registry->get_components<component::input>(),
                         window, _game_time, FIXED_TIMESTEP);

        systems::weapon_system(*_registry,
                       _registry->get_components<component::weapon>(),
                       _registry->get_components<component::position>(),
                       _registry->get_components<component::input>(),
                       _registry->get_components<component::ai_input>(),
                       _game_time);

        systems::projectile_system(*_registry,
                           _registry->get_components<component::projectile>(),
                           _registry->get_components<component::position>(),
                           window, FIXED_TIMESTEP);

        processEvents();
        _registry->run_systems(FIXED_TIMESTEP);

        if (!_boss_spawned && _total_score >= 5000) {
            spawnBoss();
        }

        if (!_boss_spawned) {
            _enemy_spawn_timer += FIXED_TIMESTEP;
            if (_enemy_spawn_timer >= _enemy_spawn_interval) {
                _enemy_spawn_timer = 0.0f;
                spawnEnemy();
            }
        }

        cleanupDeadEntities();

        accumulator -= FIXED_TIMESTEP;
    }

    if (debug_timer >= DEBUG_INTERVAL) {
        debug_timer = 0.0f;
        printEntityPositions();
    }

    _last_update = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::milliseconds(1); // adjust as needed

        for (const auto &[client_id, entity] : _client_to_entity) {
            if (_last_input_time.count(client_id) &&
                now - _last_input_time[client_id] > timeout) {

                auto &inputs = _registry->get_components<component::input>();
                if (inputs[entity]) {
                    component::input &i = inputs[entity].value();
                    i.up = false;
                    i.down = false;
                    i.left = false;
                    i.right = false;
                    i.fire = false;
                }
            }
        }
}

void GameLogic::printEntityPositions() {
    auto &positions = _registry->get_components<component::position>();
    auto &network_comps = _registry->get_components<component::network_entity>();

    std::cout << "\n========== ENTITY POSITIONS (Tick: " << _current_tick << ") ==========" << std::endl;
    int total_entities = 0;

    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        auto pos_opt = positions[i];
        if (net_opt && pos_opt) {
            std::cout << "[ENTITY] NET_ID=" << std::dec << net_opt.value().network_id
                      << " Type= \"" << net_opt.value().entity_type
                      << "\" Pos=(" << std::fixed << std::setprecision(3)
                      << pos_opt.value().x << ", " << pos_opt.value().y << ")"
                      << std::endl;
            total_entities++;
        }
    }

    std::cout << "Total entities: " << total_entities << std::endl;
    std::cout << "======================================================\n" << std::endl;
}

void GameLogic::pushClientEvent(const ClientEvent &evt) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    std::cout << "[EVENT QUEUE] Pushing event type=" << evt.action << " for client " << evt.client_id << std::endl;
    _event_queue.push(evt);
}


void GameLogic::processEvents() {
    std::queue<ClientEvent> local_queue;
    {
        std::lock_guard<std::mutex> lock(_event_mutex);
        std::swap(local_queue, _event_queue);
    }

    std::cout << "[EVENT QUEUE] Processing " << local_queue.size() << " events" << std::endl;

    while (!local_queue.empty()) {
        std::cout << "[EVENT QUEUE] Handling event type=" << local_queue.front().action
                  << " for client " << local_queue.front().client_id << std::endl;
        handleEvent(local_queue.front());
        local_queue.pop();
    }
}


void GameLogic::handleEvent(const ClientEvent &evt) {
    auto it = _client_to_entity.find(evt.client_id);
    if (it == _client_to_entity.end()) {
        std::cerr << "[GameLogic] Event from unknown client: " << evt.client_id << std::endl;
        return;
    }
    handlePlayerAction(it->second, evt.action);
}

void GameLogic::handlePlayerAction(entity player, InputEvent action) {
    auto &inputs = _registry->get_components<component::input>();
    auto input_opt = inputs[player];
    if (!input_opt)
        return;
    component::input &i = input_opt.value();
    switch (action) {
        case KEY_UP_PRESS: i.up = true; break;
        case KEY_UP_RELEASE: i.up = false; break;
        case KEY_DOWN_PRESS: i.down = true; break;
        case KEY_DOWN_RELEASE: i.down = false; break;
        case KEY_LEFT_PRESS: i.left = true; break;
        case KEY_LEFT_RELEASE: i.left = false; break;
        case KEY_RIGHT_PRESS: i.right = true; break;
        case KEY_RIGHT_RELEASE: i.right = false; break;
        case KEY_SHOOT_PRESS: i.fire = true; break;
        case KEY_SHOOT_RELEASE: i.fire = false; break;
        default: break;
    }
    std::cout << "[INPUT] Setting fire=" << (action == KEY_SHOOT_PRESS ? "true" : "false")
          << " for entity " << player << std::endl;

}

entity GameLogic::createPlayer(uint client_id, uint net_id, float x, float y) {
    using namespace component;
    entity player = _registry->spawn_entity();

    _registry->add_component(player, position{x, y});
    _registry->add_component(player, velocity{0.0f, 0.0f});
    _registry->add_component(player, component::controllable{0.5f});
    _registry->add_component(player, input{});
    _registry->add_component(player, health{100});
    _registry->add_component(player, score{0});
    _registry->add_component(player, weapon{});
    _registry->add_component(player, hitbox{32.0f, 32.0f, 0.0f, 0.0f});
    _registry->add_component(player, network_entity{net_id, static_cast<uint8_t>(client_id), false, "player"});

    _client_to_entity.emplace(client_id, player);
    return player;
}

void GameLogic::removePlayer(uint client_id) {
    auto it = _client_to_entity.find(client_id);
    if (it != _client_to_entity.end()) {
        _registry->kill_entity(it->second);
        _client_to_entity.erase(it);
        std::cout << "[GameLogic] Removed player for client " << client_id
                  << std::endl;
    }
}

entity GameLogic::getPlayerEntity(uint client_id) {
    auto it = _client_to_entity.find(client_id);
    if (it != _client_to_entity.end())
        return it->second;
    return entity(static_cast<size_t>(-1));
}

uint GameLogic::generateNetId() { return _next_net_id++; }

void GameLogic::spawnEnemy() {
    using namespace component;
    entity enemy = _registry->spawn_entity();

    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);

    float spawn_y = y_dist(_rng);

    _registry->add_component(enemy, position{0.95f, spawn_y});
    _registry->add_component(enemy, velocity{-0.01f, 0.0f});
    _registry->add_component(enemy, health{30});
    _registry->add_component(enemy, hitbox{40.0f, 40.0f, 0.0f, 0.0f});
    _registry->add_component(enemy, network_entity{generateNetId(), 0, false, "enemy"});

    _enemies.push_back(enemy);
}

void GameLogic::spawnBoss() {
    using namespace component;
    if (_boss_spawned)
        return;

    _boss = _registry->spawn_entity();

    _registry->add_component(_boss, component::position{700.0f, 300.0f});
    _registry->add_component(_boss, component::velocity{0.0f, 0.0f});
    _registry->add_component(_boss, health{1000});
    _registry->add_component(_boss, hitbox{120.0f, 120.0f, 0.0f, 0.0f});
    _registry->add_component(_boss, network_entity{generateNetId(), 0, false, "boss"});

    _boss_spawned = true;

    std::cout << "[GameLogic] Boss spawned!" << std::endl;
}

void GameLogic::cleanupDeadEntities() {
    auto &healths = _registry->get_components<component::health>();
    //auto &network_comps = _registry->get_components<component::network_entity>();

    _enemies.erase(std::remove_if(_enemies.begin(), _enemies.end(),
                                  [&](entity ent) {
                                      auto health_opt = healths[ent];
                                      return !health_opt ||
                                             health_opt.value().current_hp <= 0;
                                  }),
                   _enemies.end());

    auto &positions = _registry->get_components<component::position>();
    auto &projectiles = _registry->get_components<component::projectile>();

    _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(),
                                      [&](entity ent) {
                                          auto pos_opt = positions[ent];
                                          auto proj_opt = projectiles[ent];
                                          if (!pos_opt || !proj_opt)
                                              return true;
                                          return pos_opt.value().x < -50.0f ||
                                                 pos_opt.value().x > 850.0f ||
                                                 pos_opt.value().y < -50.0f ||
                                                 pos_opt.value().y > 650.0f ||
                                                 proj_opt.value().lifetime <=
                                                     0.0f;
                                      }),
                       _projectiles.end());
}

// Snapshot management
WorldSnapshot GameLogic::generateSnapshot() {
    WorldSnapshot snapshot;
    snapshot.tick = _current_tick;
    snapshot.timestamp = std::chrono::steady_clock::now();

    auto &positions = _registry->get_components<component::position>();
    auto &velocities = _registry->get_components<component::velocity>();
    auto &healths = _registry->get_components<component::health>();
    auto &scores = _registry->get_components<component::score>();
    auto &network_comps = _registry->get_components<component::network_entity>();

    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (!net_opt)
            continue;

        entity ent = _registry->entity_from_index(i);
        auto pos_opt = positions[ent];
        auto vel_opt = velocities[ent];

        if (pos_opt && vel_opt) {
            EntitySnapshot entity_snap;
            entity_snap.net_id = net_opt.value().network_id;
            entity_snap.entity_type = net_opt.value().entity_type;
            entity_snap.pos = pos_opt.value();
            entity_snap.vel = vel_opt.value();
            entity_snap.health =
                healths[ent] ? healths[ent].value().current_hp : 0;
            entity_snap.score =
                scores[ent] ? scores[ent].value().current_score : 0;
            entity_snap.tick = _current_tick;
            snapshot.entities.push_back(entity_snap);
        }
    }

    _snapshot_history.push_back(snapshot);
    if (_snapshot_history.size() > MAX_SNAPSHOT_HISTORY) {
        _snapshot_history.pop_front();
    }

    return snapshot;
}

std::vector<EntitySnapshot> GameLogic::getDeltaSnapshot(uint last_acked_tick) {
    auto acked_snapshot =
        std::find_if(_snapshot_history.begin(), _snapshot_history.end(),
                     [last_acked_tick](const WorldSnapshot &s) {
                         return s.tick == last_acked_tick;
                     });

    if (acked_snapshot == _snapshot_history.end()) {
        auto current = generateSnapshot();
        return current.entities;
    }

    std::unordered_map<uint, EntitySnapshot> old_entities;
    for (const auto &entity : acked_snapshot->entities) {
        old_entities[entity.net_id] = entity;
    }

    std::vector<EntitySnapshot> delta;
    auto current_snapshot = generateSnapshot();

    const float POS_THRESHOLD = 5.0f;

    for (const auto &entity : current_snapshot.entities) {
        auto old_it = old_entities.find(entity.net_id);
        bool should_send = false;

        if (old_it == old_entities.end()) {
            should_send = true;
        } else {
            const EntitySnapshot &old = old_it->second;
            float pos_delta = std::sqrt(std::pow(entity.pos.x - old.pos.x, 2) +
                                        std::pow(entity.pos.y - old.pos.y, 2));

            should_send =
                (pos_delta > POS_THRESHOLD || entity.health != old.health ||
                 entity.score != old.score);
        }

        if (should_send) {
            delta.push_back(entity);
        }
    }

    return delta;
}

void GameLogic::markEntitiesSynced(const std::vector<uint32_t> &net_ids) {
    auto &network_comps = _registry->get_components<component::network_entity>();
    for (uint32_t net_id : net_ids) {
        entity ent = findEntityByNetId(net_id);
        if (ent != entity(static_cast<size_t>(-1)) &&
            ent < network_comps.size() && network_comps[ent]) {
            network_comps[ent]->synced = true;
        }
    }
}


entity GameLogic::findEntityByNetId(uint net_id) {
    auto &network_comps = _registry->get_components<component::network_entity>();
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (net_opt && net_opt.value().network_id == net_id) {
            return _registry->entity_from_index(i);
        }
    }
    return entity(static_cast<size_t>(-1));
}

bool GameLogic::isEntitySynced(uint net_id) {
    auto ent = findEntityByNetId(net_id);
    if (ent == entity(static_cast<size_t>(-1))) return false;

    auto &network_comps = _registry->get_components<component::network_entity>();
    if (ent < network_comps.size() && network_comps[ent]) {
        return network_comps[ent]->synced;
    }
    return false;
}
