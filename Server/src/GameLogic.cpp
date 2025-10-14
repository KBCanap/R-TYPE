#include "GameLogic.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

GameLogic::GameLogic(std::shared_ptr<registry> reg)
    : _registry(reg), _running(false), _current_tick(0), _game_time(0.0f),
      _enemy_spawn_timer(0.0f), _enemy_spawn_interval(2.0f), _total_score(0),
      _boss_spawned(false), _boss(0), _next_net_id(1000) {
    _last_update = std::chrono::steady_clock::now();
    _rng.seed(std::random_device{}());
    registerSystems();
}

GameLogic::~GameLogic() { stop(); }

void GameLogic::start() {
    _running = true;
    _last_update = std::chrono::steady_clock::now();

    spawnEnemy();
    spawnEnemy();
    spawnEnemy();
    spawnEnemy();
    spawnEnemy();
    spawnEnemy();
    spawnEnemy();

    std::cout << "[GameLogic] Started" << std::endl;
}

void GameLogic::stop() {
    _running = false;
    std::cout << "[GameLogic] Stopped" << std::endl;
}

void GameLogic::registerSystems() {
    _registry->add_system<InputState, Velocity, PlayerComponent>(inputSystem);
    _registry->add_system<Position, Velocity>(movementSystem);
    _registry->add_system<Weapon, Position, InputState, PlayerComponent>(
        weaponSystem);
    _registry->add_system<Projectile, Position>(projectileSystem);
    _registry->add_system<Position, Hitbox, Projectile, PlayerComponent, Enemy,
                          Health, Score, NetworkComponent>(collisionSystem);
    _registry->add_system<Health, NetworkComponent>(healthSystem);
    _registry->add_system<Enemy, Position, Velocity>(enemyAISystem);
    _registry->add_system<Boss, Position, Velocity, Health>(bossAISystem);
}

void GameLogic::printEntityPositions() {
    auto &positions = _registry->get_components<Position>();
    auto &network_comps = _registry->get_components<NetworkComponent>();
    auto &players = _registry->get_components<PlayerComponent>();
    auto &enemies = _registry->get_components<Enemy>();

    std::cout << "\n========== ENTITY POSITIONS (Tick: " << _current_tick
              << ") ==========" << std::endl;

    int total_entities = 0;

    // Print players
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (!net_opt)
            continue;

        auto pos_opt = positions[i];
        auto player_opt = players[i];

        if (pos_opt && player_opt) {
            std::cout << "[PLAYER] NET_ID=" << net_opt.value().net_id
                      << " Client=" << player_opt.value().client_id << " Pos=("
                      << std::fixed << std::setprecision(3) << pos_opt.value().x
                      << ", " << pos_opt.value().y << ")"
                      << " Type=" << net_opt.value().entity_type << std::endl;
            total_entities++;
        }
    }

    // Print enemies
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (!net_opt)
            continue;

        auto pos_opt = positions[i];
        auto enemy_opt = enemies[i];

        if (pos_opt && enemy_opt) {
            std::cout << "[ENEMY]  NET_ID=" << net_opt.value().net_id
                      << " Type=" << enemy_opt.value().enemy_type << " Pos=("
                      << std::fixed << std::setprecision(3) << pos_opt.value().x
                      << ", " << pos_opt.value().y << ")" << std::endl;
            total_entities++;
        }
    }

    std::cout << "Total entities: " << total_entities << std::endl;
    std::cout << "======================================================\n"
              << std::endl;
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

    // *** ADD DEBUG OUTPUT HERE ***
    if (debug_timer >= DEBUG_INTERVAL) {
        debug_timer = 0.0f;
        printEntityPositions();
    }

    _last_update = std::chrono::steady_clock::now();
}

void GameLogic::pushClientEvent(const ClientEvent &evt) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    _event_queue.push(evt);
}

void GameLogic::processEvents() {
    std::queue<ClientEvent> local_queue;
    {
        std::lock_guard<std::mutex> lock(_event_mutex);
        std::swap(local_queue, _event_queue);
    }

    while (!local_queue.empty()) {
        const ClientEvent &evt = local_queue.front();
        handleEvent(evt);
        local_queue.pop();
    }
}

void GameLogic::handleEvent(const ClientEvent &evt) {
    auto it = _client_to_entity.find(evt.client_id);
    if (it == _client_to_entity.end()) {
        std::cerr << "[GameLogic] Event from unknown client: " << evt.client_id
                  << std::endl;
        return;
    }

    entity player = it->second;
    handlePlayerAction(player, evt.action);
}

void GameLogic::handlePlayerAction(entity player, InputEvent action) {
    auto &input_states = _registry->get_components<InputState>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    auto input_opt = input_states[player];
    if (!input_opt)
        return;

    InputState &input = input_opt.value();

    switch (action) {
    case KEY_UP_PRESS:
        input.up = true;
        break;
    case KEY_UP_RELEASE:
        input.up = false;
        break;
    case KEY_DOWN_PRESS:
        input.down = true;
        break;
    case KEY_DOWN_RELEASE:
        input.down = false;
        break;
    case KEY_LEFT_PRESS:
        input.left = true;
        break;
    case KEY_LEFT_RELEASE:
        input.left = false;
        break;
    case KEY_RIGHT_PRESS:
        input.right = true;
        break;
    case KEY_RIGHT_RELEASE:
        input.right = false;
        break;
    case KEY_SHOOT_PRESS:
        input.shoot = true;
        break;
    case KEY_SHOOT_RELEASE:
        input.shoot = false;
        break;
    default:
        break;
    }

    auto net_opt = network_comps[player];
    if (net_opt) {
        net_opt.value().needs_update = true;
    }
}

entity GameLogic::createPlayer(uint client_id, uint net_id, float x, float y) {
    entity player = _registry->spawn_entity();

    _registry->add_component(player, Position{x, y});
    _registry->add_component(player, Velocity{0.0f, 0.0f});
    _registry->add_component(player, InputState{});
    _registry->add_component(player, PlayerComponent{client_id, true, 0.0f});
    _registry->add_component(player, Health{100, 100, 0.0f});
    _registry->add_component(player, Score{0});
    _registry->add_component(player, Weapon{0.25f, 0.0f, 0, 10});
    _registry->add_component(player, Hitbox{32.0f, 32.0f, 0.0f, 0.0f});
    _registry->add_component(player, NetworkComponent{net_id, true, "player"});

    _client_to_entity.insert({client_id, player});

    std::cout << "[GameLogic] Created player for client " << client_id
              << " at (" << x << ", " << y << ")" << std::endl;

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
    entity enemy = _registry->spawn_entity();

    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);

    int enemy_type = type_dist(_rng);
    float spawn_y = y_dist(_rng);

    _registry->add_component(enemy, Position{0.95f, spawn_y});
    _registry->add_component(enemy, Velocity{-0.01f, 0.0f});
    _registry->add_component(enemy, Enemy{enemy_type, 0.0f, 100});
    _registry->add_component(enemy, Health{30, 30, 0.0f});
    _registry->add_component(enemy, Hitbox{40.0f, 40.0f, 0.0f, 0.0f});
    _registry->add_component(enemy,
                             NetworkComponent{generateNetId(), true, "enemy"});

    _enemies.push_back(enemy);
}

void GameLogic::spawnBoss() {
    if (_boss_spawned)
        return;

    _boss = _registry->spawn_entity();

    _registry->add_component(_boss, Position{700.0f, 300.0f});
    _registry->add_component(_boss, Velocity{0.0f, 0.0f});
    _registry->add_component(_boss, Boss{0, 0.0f});
    _registry->add_component(_boss, Enemy{2, 0.0f, 5000});
    _registry->add_component(_boss, Health{1000, 1000, 0.0f});
    _registry->add_component(_boss, Hitbox{120.0f, 120.0f, 0.0f, 0.0f});
    _registry->add_component(_boss,
                             NetworkComponent{generateNetId(), true, "boss"});

    _boss_spawned = true;

    std::cout << "[GameLogic] Boss spawned!" << std::endl;
}

void GameLogic::cleanupDeadEntities() {
    auto &healths = _registry->get_components<Health>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    _enemies.erase(std::remove_if(_enemies.begin(), _enemies.end(),
                                  [&](entity ent) {
                                      auto health_opt = healths[ent];
                                      return !health_opt ||
                                             health_opt.value().current_hp <= 0;
                                  }),
                   _enemies.end());

    auto &positions = _registry->get_components<Position>();
    auto &projectiles = _registry->get_components<Projectile>();

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

    auto &positions = _registry->get_components<Position>();
    auto &velocities = _registry->get_components<Velocity>();
    auto &healths = _registry->get_components<Health>();
    auto &scores = _registry->get_components<Score>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (!net_opt)
            continue;

        entity ent = _registry->entity_from_index(i);
        auto pos_opt = positions[ent];
        auto vel_opt = velocities[ent];

        if (pos_opt && vel_opt) {
            EntitySnapshot entity_snap;
            entity_snap.net_id = net_opt.value().net_id;
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

void GameLogic::markEntitiesSynced() {
    auto &network_comps = _registry->get_components<NetworkComponent>();
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (net_opt) {
            net_opt.value().needs_update = false;
        }
    }
}

entity GameLogic::findEntityByNetId(uint net_id) {
    auto &network_comps = _registry->get_components<NetworkComponent>();
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (net_opt && net_opt.value().net_id == net_id) {
            return _registry->entity_from_index(i);
        }
    }
    return entity(static_cast<size_t>(-1));
}

// ====== ECS SYSTEMS IMPLEMENTATION ======

void GameLogic::inputSystem(registry &reg, sparse_array<InputState> &inputs,
                            sparse_array<Velocity> &velocities,
                            sparse_array<PlayerComponent> &players, float dt) {
    const float MOVE_SPEED = 300.0f;

    for (size_t i = 0; i < players.size(); ++i) {
        auto player_opt = players[i];
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto vel_opt = velocities[i];
        auto input_opt = inputs[i];

        if (vel_opt && input_opt) {
            Velocity &vel = vel_opt.value();
            InputState &input = input_opt.value();

            vel.vx = 0.0f;
            vel.vy = 0.0f;

            if (input.up)
                vel.vy -= MOVE_SPEED;
            if (input.down)
                vel.vy += MOVE_SPEED;
            if (input.left)
                vel.vx -= MOVE_SPEED;
            if (input.right)
                vel.vx += MOVE_SPEED;

            // Normalize diagonal movement
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
    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto pos_opt = positions[i];
        auto vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            Position &pos = pos_opt.value();
            Velocity &vel = vel_opt.value();

            pos.x += 300;
            pos.y += 150;

            pos.x = std::max(0.0f, std::min(pos.x, 1.0f));
            pos.y = std::max(0.0f, std::min(pos.y, 1.0f));
        }
    }
}

void GameLogic::weaponSystem(registry &reg, sparse_array<Weapon> &weapons,
                             sparse_array<Position> &positions,
                             sparse_array<InputState> &inputs,
                             sparse_array<PlayerComponent> &players,
                             float game_time) {
    // TODO: Add projectiles logic
}

void GameLogic::projectileSystem(registry &reg,
                                 sparse_array<Projectile> &projectiles,
                                 sparse_array<Position> &positions, float dt) {
    for (size_t i = 0; i < projectiles.size(); ++i) {
        auto proj_opt = projectiles[i];
        if (proj_opt) {
            proj_opt.value().lifetime -= dt;
        }
    }
}

void GameLogic::collisionSystem(
    registry &reg, sparse_array<Position> &positions,
    sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
    sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
    sparse_array<Health> &healths, sparse_array<Score> &scores,
    sparse_array<NetworkComponent> &network_comps, float dt) {
    // TODO: Add collision logic
}

void GameLogic::healthSystem(registry &reg, sparse_array<Health> &healths,
                             sparse_array<NetworkComponent> &network_comps,
                             float dt) {
    for (size_t i = 0; i < healths.size(); ++i) {
        auto health_opt = healths[i];
        if (health_opt) {
            if (health_opt.value().invulnerability_timer > 0.0f) {
                health_opt.value().invulnerability_timer -= dt;
            }

            if (health_opt.value().current_hp <= 0) {
                // Mark for network update before death
                if (network_comps[i]) {
                    network_comps[i].value().needs_update = true;
                }
            }
        }
    }
}

void GameLogic::enemyAISystem(registry &reg, sparse_array<Enemy> &enemies,
                              sparse_array<Position> &positions,
                              sparse_array<Velocity> &velocities, float dt) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto enemy_opt = enemies[i];
        if (!enemy_opt)
            continue;

        auto pos_opt = positions[i];
        auto vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            Enemy &enemy = enemy_opt.value();
            Velocity &vel = vel_opt.value();

            enemy.pattern_timer += dt;

            if (enemy.enemy_type == 1) { // Zigzag pattern
                vel.vy = std::sin(enemy.pattern_timer * 3.0f) * 100.0f;
            }
        }
    }
}

void GameLogic::bossAISystem(registry &reg, sparse_array<Boss> &bosses,
                             sparse_array<Position> &positions,
                             sparse_array<Velocity> &velocities,
                             sparse_array<Health> &healths, float dt) {}