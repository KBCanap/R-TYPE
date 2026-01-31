#include "GameLogic.hpp"
#include "../../ecs/include/GameConstants.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

GameLogic::GameLogic(std::shared_ptr<registry> reg, uint8_t level_id)
    : _registry(reg), _running(false), _current_tick(0), _game_time(0.0f),
      _enemy_spawn_timer(0.0f), _enemy_spawn_interval(2.0f), _accumulator(0.0f),
      _debug_timer(0.0f), _total_score(0), _boss_spawned(false), _boss_active(false),
      _boss(0), _level_id(level_id), _endless_mode(level_id == 99),
      _level_complete(false), _next_boss_threshold(300), _boss_count(0),
      _powerup_spawn_timer(0.0f), _powerup_spawn_interval(10.0f),
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
    _enemy_spawn_timer = -3.0f;
    _current_tick = 0;
    _accumulator = 0.0f;
    _debug_timer = 0.0f;
}

void GameLogic::stop() {
    _running = false;
}

void GameLogic::registerSystems() {
    _registry->add_system<InputState, Velocity, PlayerComponent>(inputSystem);
    _registry->add_system<Position, Velocity>(movementSystem);
    _registry->add_system<Weapon, Position, InputState, PlayerComponent>(weaponSystem);
    _registry->add_system<Projectile, Position>(projectileSystem);
    _registry->add_system<Position, Hitbox, Projectile, PlayerComponent, Enemy,
                          Health, Score, NetworkComponent>(collisionSystem);
    _registry->add_system<Health, NetworkComponent>(healthSystem);
    _registry->add_system<Enemy, Position, Velocity>(enemyAISystem);
    _registry->add_system<Boss, Position, Velocity, Health>(bossAISystem);
}

void GameLogic::printEntityPositions() {}

void GameLogic::update(float deltaTime) {
    if (!_running)
        return;

    static const float FIXED_TIMESTEP = 1.0f / 60.0f;

    _accumulator += deltaTime;

    while (_accumulator >= FIXED_TIMESTEP) {
        _current_tick++;
        _game_time += FIXED_TIMESTEP;

        processEvents();
        updatePlayerScores(FIXED_TIMESTEP);
        _registry->run_systems(FIXED_TIMESTEP);
        processPlayerShooting(FIXED_TIMESTEP);
        processEnemyShooting(FIXED_TIMESTEP);
        processBossShooting(FIXED_TIMESTEP);
        processPowerUpCollisions();

        updateTotalScore();

        if (!_level_complete) {
            checkBossSpawn();

            if (!_boss_active) {
                _enemy_spawn_timer += FIXED_TIMESTEP;
                if (_enemy_spawn_timer >= _enemy_spawn_interval) {
                    _enemy_spawn_timer = 0.0f;
                    spawnEnemy();
                }
            }
        }

        _powerup_spawn_timer += FIXED_TIMESTEP;
        if (_powerup_spawn_timer >= _powerup_spawn_interval) {
            _powerup_spawn_timer = 0.0f;
            spawnPowerUp();
        }

        cleanupDeadEntities();
        _accumulator -= FIXED_TIMESTEP;
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
        std::cerr << "[GameLogic] Event from unknown client: " << evt.client_id << std::endl;
        return;
    }

    entity player = it->second;
    handlePlayerAction(player, evt.action);
}

void GameLogic::handlePlayerAction(entity player, InputEvent action) {
    auto &input_states = _registry->get_components<InputState>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    auto &input_opt = input_states[player];
    if (!input_opt)
        return;

    InputState &input = input_opt.value();

    switch (action) {
    case KEY_UP_PRESS:    input.up = true; break;
    case KEY_UP_RELEASE:  input.up = false; break;
    case KEY_DOWN_PRESS:  input.down = true; break;
    case KEY_DOWN_RELEASE: input.down = false; break;
    case KEY_LEFT_PRESS:  input.left = true; break;
    case KEY_LEFT_RELEASE: input.left = false; break;
    case KEY_RIGHT_PRESS: input.right = true; break;
    case KEY_RIGHT_RELEASE: input.right = false; break;
    case KEY_SHOOT_PRESS: input.shoot = true; break;
    case KEY_SHOOT_RELEASE: input.shoot = false; break;
    default: break;
    }

    auto &net_opt = network_comps[player];
    if (net_opt) {
        net_opt.value().needs_update = true;
    }
}
