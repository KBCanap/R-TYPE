#include "GameLogic.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

GameLogic::GameLogic(std::shared_ptr<registry> reg)
    : _registry(reg), _running(false), _current_tick(0), _game_time(0.0f),
      _enemy_spawn_timer(0.0f), _enemy_spawn_interval(2.0f), _accumulator(0.0f),
      _debug_timer(0.0f), _total_score(0), _boss_spawned(false), _boss(0),
      _next_net_id(1000) {
    _last_update = std::chrono::steady_clock::now();
    _rng.seed(std::random_device{}());
    registerSystems();
}

GameLogic::~GameLogic() { stop(); }

void GameLogic::start() {
    _running = true;
    _last_update = std::chrono::steady_clock::now();
    _game_time = 0.0f;
    _enemy_spawn_timer = 0.0f;
    _current_tick = 0;
    _accumulator = 0.0f;   // Reset accumulator to avoid initial burst
    _debug_timer = 0.0f;

    // Don't spawn enemies at start - let the spawn timer handle it like solo mode
    // Solo mode uses _enemySpawnTimer which starts at 0, so first enemy spawns after interval

    std::cout << "[GameLogic] Started - enemies will spawn via timer like solo mode" << std::endl;
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
    static const float DEBUG_INTERVAL = 1.0f;

    // Use member variables instead of static to properly reset between games
    _accumulator += deltaTime;
    _debug_timer += deltaTime;

    while (_accumulator >= FIXED_TIMESTEP) {
        _current_tick++;
        _game_time += FIXED_TIMESTEP;

        processEvents();
        _registry->run_systems(FIXED_TIMESTEP);
        processPlayerShooting(FIXED_TIMESTEP);
        processEnemyShooting(FIXED_TIMESTEP);

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
        _accumulator -= FIXED_TIMESTEP;
    }

    // Debug output every second
    if (_debug_timer >= DEBUG_INTERVAL) {
        _debug_timer = 0.0f;
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

    auto &input_opt = input_states[player];  // Reference to avoid copy
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

    auto &net_opt = network_comps[player];  // Reference to avoid copy
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
    // fire_rate = 8.0 means 8 shots/sec like solo mode (fire_timer resets to 1/fire_rate = 0.125s)
    _registry->add_component(player, Weapon{8.0f, 0.0f, 0, 20});
    _registry->add_component(player, Hitbox{66.0f, 34.0f, 15.0f, 0.0f});  // Same as solo mode
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

void GameLogic::spawnEnemy() {
    entity enemy = _registry->spawn_entity();

    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);

    int enemy_type = type_dist(_rng);
    float spawn_y = y_dist(_rng);
    uint net_id = generateNetId();

    _registry->add_component(enemy, Position{0.95f, spawn_y});
    // Speed: 150 pixels/sec on 800px screen = 0.1875 normalized/sec (like solo mode)
    _registry->add_component(enemy, Velocity{-0.1875f, 0.0f});
    _registry->add_component(enemy, Enemy{enemy_type, 0.0f, 100});
    _registry->add_component(enemy, Health{25, 25, 0.0f});  // 25 HP like solo mode
    _registry->add_component(enemy, Hitbox{50.0f, 58.0f, 0.0f, 0.0f});  // Same as solo (50x58)
    _registry->add_component(enemy, NetworkComponent{net_id, true, "enemy"});

    _enemies.push_back(enemy);

    // Store new entity info for broadcasting ENTITY_CREATE
    _new_entities.push_back({net_id, "enemy", 0.95f, spawn_y, 25});
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

void GameLogic::spawnProjectile(float x, float y, bool is_player_projectile, int damage) {
    entity proj = _registry->spawn_entity();

    // Projectile velocity: player shoots right, enemies shoot left
    float proj_vx = is_player_projectile ? 1.0f : -0.8f;  // Normalized speed
    uint net_id = generateNetId();

    _registry->add_component(proj, Position{x, y});
    _registry->add_component(proj, Velocity{proj_vx, 0.0f});
    _registry->add_component(proj, Projectile{is_player_projectile, damage, 3.0f});  // 3 sec lifetime
    _registry->add_component(proj, Hitbox{8.0f, 8.0f, 0.0f, 0.0f});

    // Use different entity_type string based on projectile owner
    std::string proj_type = is_player_projectile ? "allied_projectile" : "projectile";
    _registry->add_component(proj, NetworkComponent{net_id, true, proj_type});

    _projectiles.push_back(proj);

    // Store new entity info for broadcasting ENTITY_CREATE
    _new_entities.push_back({net_id, proj_type, x, y, 0});
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

        // Decrease weapon timer
        if (weapon.fire_timer > 0.0f) {
            weapon.fire_timer -= dt;
        }

        // Check if player wants to shoot and weapon is ready
        if (input.shoot && weapon.fire_timer <= 0.0f) {
            Position &pos = pos_opt.value();

            // Spawn projectile slightly in front of player (20 damage like solo)
            spawnProjectile(pos.x + 0.05f, pos.y, true, 20);

            // Reset fire timer based on fire rate
            weapon.fire_timer = 1.0f / weapon.fire_rate;

            // Reset shoot flag - client must send another fire event to shoot again
            input.shoot = false;

            // Mark for network update
            auto &net_opt = network_comps[i];
            if (net_opt) {
                net_opt.value().needs_update = true;
            }
        }
    }
}

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

        // Update shoot timer
        enemy.shoot_timer += dt;

        // Only shoot if visible on screen (x < 0.9) and timer expired
        if (pos.x < 0.9f && enemy.shoot_timer >= enemy.shoot_interval) {
            enemy.shoot_timer = 0.0f;

            // Spawn enemy projectile (shoots left, 25 damage like solo mode)
            spawnProjectile(pos.x - 0.02f, pos.y, false, 25);
        }
    }
}

void GameLogic::cleanupDeadEntities() {
    auto &healths = _registry->get_components<Health>();
    auto &network_comps = _registry->get_components<NetworkComponent>();
    auto &positions = _registry->get_components<Position>();
    auto &players = _registry->get_components<PlayerComponent>();
    auto &projectile_comps = _registry->get_components<Projectile>();

    std::vector<entity> to_kill;

    // Cleanup dead players
    for (auto it = _client_to_entity.begin(); it != _client_to_entity.end(); ) {
        entity ent = it->second;
        auto &health_opt = healths[ent];
        if (health_opt && health_opt.value().current_hp <= 0) {
            std::cout << "[GameLogic] Player " << it->first << " died!" << std::endl;
            to_kill.push_back(ent);
            it = _client_to_entity.erase(it);
        } else {
            ++it;
        }
    }

    // Cleanup enemies by HP or position (off-screen left)
    for (auto it = _enemies.begin(); it != _enemies.end(); ) {
        entity ent = *it;
        auto &health_opt = healths[ent];
        auto &pos_opt = positions[ent];

        bool should_remove = !health_opt || !pos_opt ||
                            health_opt.value().current_hp <= 0 ||
                            pos_opt.value().x <= 0.0f;  // Remove when at or past left edge

        if (should_remove) {
            to_kill.push_back(ent);
            it = _enemies.erase(it);
        } else {
            ++it;
        }
    }

    // Cleanup projectiles by position or lifetime
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

    // Store net_ids and kill all marked entities
    for (entity ent : to_kill) {
        auto &net_opt = network_comps[ent];
        if (net_opt) {
            _destroyed_net_ids.push_back(net_opt.value().net_id);
            std::cout << "[GameLogic] Destroying entity NET_ID=" << net_opt.value().net_id << std::endl;
        }
        _registry->kill_entity(ent);
    }
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
    const float MOVE_SPEED = 0.5f;  // Normalized units per second

    for (size_t i = 0; i < players.size(); ++i) {
        auto &player_opt = players[i];  // Reference to avoid copy
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto &vel_opt = velocities[i];  // Reference to avoid copy
        auto &input_opt = inputs[i];    // Reference to avoid copy

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
    auto &players = reg.get_components<PlayerComponent>();

    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto &pos_opt = positions[i];  // Reference to avoid copy
        auto &vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            Position &pos = pos_opt.value();
            Velocity &vel = vel_opt.value();

            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;

            // Only clamp players to screen bounds, not projectiles/enemies
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
    // Update weapon cooldowns
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
        auto &proj_opt = projectiles[i];  // Reference to avoid copy
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

    // Check all projectiles for collisions
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

        // Check collision with all potential targets
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

            // Determine if target is player or enemy
            bool target_is_player = players[target_idx].has_value();
            bool target_is_enemy = enemies[target_idx].has_value();

            // Player projectiles hit enemies, enemy projectiles hit players
            bool valid_hit = (proj.is_player_projectile && target_is_enemy) ||
                            (!proj.is_player_projectile && target_is_player);

            if (!valid_hit)
                continue;

            // Simple AABB collision (normalized coordinates)
            // Convert hitbox to normalized space (assume hitbox is in pixels, divide by screen size ~800x600)
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
                // Apply damage
                Health &target_health = target_health_opt.value();
                target_health.current_hp -= proj.damage;

                std::cout << "[COLLISION] Projectile hit! Target idx=" << target_idx
                          << " is_player=" << target_is_player
                          << " HP now=" << target_health.current_hp << std::endl;

                // Mark for network update
                auto &target_net_opt = network_comps[target_idx];
                if (target_net_opt) {
                    target_net_opt.value().needs_update = true;
                }

                // Award score if enemy killed by player projectile
                if (proj.is_player_projectile && target_is_enemy && target_health.current_hp <= 0) {
                    auto &enemy_opt = enemies[target_idx];
                    if (enemy_opt) {
                        // Find a player to award score to
                        for (size_t p_idx = 0; p_idx < players.size(); ++p_idx) {
                            auto &p_opt = players[p_idx];
                            auto &score_opt = scores[p_idx];
                            if (p_opt && score_opt) {
                                score_opt.value().current_score += enemy_opt.value().score_value;
                                break;
                            }
                        }
                    }
                }

                // Mark projectile for removal by setting lifetime to 0
                proj.lifetime = 0.0f;
                break;  // Projectile can only hit one target
            }
        }
    }

    // Check player-enemy contact collisions (50 damage to both, like solo mode)
    const int CONTACT_DAMAGE = 50;

    for (size_t player_idx = 0; player_idx < players.size(); ++player_idx) {
        auto &player_opt = players[player_idx];
        if (!player_opt || !player_opt.value().is_active)
            continue;

        auto &player_pos_opt = positions[player_idx];
        auto &player_hitbox_opt = hitboxes[player_idx];
        auto &player_health_opt = healths[player_idx];

        if (!player_pos_opt || !player_hitbox_opt || !player_health_opt)
            continue;

        // Skip if player has invulnerability
        if (player_health_opt.value().invulnerability_timer > 0.0f)
            continue;

        Position &player_pos = player_pos_opt.value();
        Hitbox &player_hitbox = player_hitbox_opt.value();

        // Convert hitbox to normalized space
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

            // Convert hitbox to normalized space
            float enemy_w = enemy_hitbox.width / 800.0f;
            float enemy_h = enemy_hitbox.height / 600.0f;

            // AABB collision check
            bool collision =
                player_pos.x < enemy_pos.x + enemy_w &&
                player_pos.x + player_w > enemy_pos.x &&
                player_pos.y < enemy_pos.y + enemy_h &&
                player_pos.y + player_h > enemy_pos.y;

            if (collision) {
                // Apply damage to both
                player_health_opt.value().current_hp -= CONTACT_DAMAGE;
                enemy_health_opt.value().current_hp -= CONTACT_DAMAGE;

                // Give player brief invulnerability to prevent multiple hits
                player_health_opt.value().invulnerability_timer = 0.5f;

                std::cout << "[COLLISION] Player-Enemy contact! Player HP="
                          << player_health_opt.value().current_hp
                          << " Enemy HP=" << enemy_health_opt.value().current_hp
                          << std::endl;

                // Mark for network update
                auto &player_net_opt = network_comps[player_idx];
                if (player_net_opt) {
                    player_net_opt.value().needs_update = true;
                }
                auto &enemy_net_opt = network_comps[enemy_idx];
                if (enemy_net_opt) {
                    enemy_net_opt.value().needs_update = true;
                }

                break;  // Only one contact collision per player per tick
            }
        }
    }
}

void GameLogic::healthSystem(registry &reg, sparse_array<Health> &healths,
                             sparse_array<NetworkComponent> &network_comps,
                             float dt) {
    for (size_t i = 0; i < healths.size(); ++i) {
        auto &health_opt = healths[i];  // Reference to avoid copy
        if (health_opt) {
            if (health_opt.value().invulnerability_timer > 0.0f) {
                health_opt.value().invulnerability_timer -= dt;
            }

            if (health_opt.value().current_hp <= 0) {
                // Mark for network update before death
                auto &net_opt = network_comps[i];  // Reference to avoid copy
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
        auto &enemy_opt = enemies[i];  // Reference to avoid copy
        if (!enemy_opt)
            continue;

        auto &pos_opt = positions[i];  // Reference to avoid copy
        auto &vel_opt = velocities[i]; // Reference to avoid copy

        if (pos_opt && vel_opt) {
            Enemy &enemy = enemy_opt.value();
            Velocity &vel = vel_opt.value();

            enemy.pattern_timer += dt;

            if (enemy.enemy_type == 1) { // Zigzag pattern
                vel.vy = std::sin(enemy.pattern_timer * 3.0f) * 0.3f;  // Normalized amplitude
            }
        }
    }
}

void GameLogic::bossAISystem(registry &reg, sparse_array<Boss> &bosses,
                             sparse_array<Position> &positions,
                             sparse_array<Velocity> &velocities,
                             sparse_array<Health> &healths, float dt) {}