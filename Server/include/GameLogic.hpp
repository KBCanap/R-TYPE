/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameLogic
*/

#ifndef GAMELOGIC_COMPLETE_HPP_
#define GAMELOGIC_COMPLETE_HPP_

#include "../../ecs/include/registery.hpp"
#include <chrono>
#include <cstdint>  // Required for uint32_t
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <thread>      // Required for std::this_thread
#include <unordered_map>
#include <vector>

// Input events
enum InputEvent {
    KEY_UP_PRESS,
    KEY_UP_RELEASE,
    KEY_DOWN_PRESS,
    KEY_DOWN_RELEASE,
    KEY_LEFT_PRESS,
    KEY_LEFT_RELEASE,
    KEY_RIGHT_PRESS,
    KEY_RIGHT_RELEASE,
    KEY_SHOOT_PRESS,
    KEY_SHOOT_RELEASE
};

// Component structures for ECS
struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
};

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
    int weapon_type = 0; // 0=single, 1=rapid, 2=burst, 3=spread
};

struct PlayerComponent {
    uint32_t client_id;   // Changed from uint to uint32_t
    bool is_active;
    float respawn_timer = 0.0f;
};

struct Health {
    int current_hp;
    int max_hp;
    float invulnerability_timer = 0.0f;
};

struct Score {
    int current_score;
};

struct Weapon {
    float fire_rate;
    float fire_timer;
    int weapon_type;
    int damage;
};

struct Projectile {
    bool is_player_projectile;
    int damage;
    float lifetime;
};

struct Enemy {
    int enemy_type; // 0=straight, 1=zigzag, 2=boss
    float pattern_timer;
    int score_value;
};

struct Boss {
    int phase;
    float phase_timer;
};

struct Hitbox {
    float width;
    float height;
    float offset_x;
    float offset_y;
};

struct NetworkComponent {
    uint32_t net_id;   // Changed from uint to uint32_t
    bool needs_update;
    std::string entity_type; // "player", "enemy", "projectile", "boss"
};

// Event structure with timestamp and sequence
struct ClientEvent {
    uint32_t client_id;   // Changed from uint to uint32_t
    InputEvent action;
    uint32_t sequence_number;   // Changed from uint to uint32_t
    std::chrono::steady_clock::time_point timestamp;
};

// Entity state snapshot for network sync
struct EntitySnapshot {
    uint32_t net_id;   // Changed from uint to uint32_t
    std::string entity_type;
    Position pos;
    Velocity vel;
    int health;
    int score;
    uint32_t tick;   // Changed from uint to uint32_t
};

struct WorldSnapshot {
    uint32_t tick;   // Changed from uint to uint32_t
    std::chrono::steady_clock::time_point timestamp;
    std::vector<EntitySnapshot> entities;
};

/**
 * @brief Class to handle the game logic for R-Type multiplayer server
 */
class GameLogic {
  public:
    GameLogic(std::shared_ptr<registry> reg);
    ~GameLogic();

    // Game loop control
    void start();
    void stop();
    void update(float deltaTime);
    void printEntityPositions();

    // Event handling from network thread
    void pushClientEvent(const ClientEvent &evt);

    // Snapshot management
    WorldSnapshot generateSnapshot();
    std::vector<EntitySnapshot> getDeltaSnapshot(uint32_t last_acked_tick);
    void markEntitiesSynced();

    // Player management
    entity createPlayer(uint32_t client_id, uint32_t net_id, float x, float y);
    void removePlayer(uint32_t client_id);
    entity getPlayerEntity(uint32_t client_id);

    // Getters
    uint32_t getCurrentTick() const { return _current_tick; }
    uint32_t generateNetId();

  private:
    std::shared_ptr<registry> _registry;
    std::queue<ClientEvent> _event_queue;
    std::mutex _event_mutex;
    std::unordered_map<uint32_t, entity> _client_to_entity;
    bool _running;

    // Game tick tracking for sync
    uint32_t _current_tick;
    std::chrono::steady_clock::time_point _last_update;

    // Snapshot history
    static const size_t MAX_SNAPSHOT_HISTORY = 128;
    std::deque<WorldSnapshot> _snapshot_history;

    // Game state
    float _game_time;
    float _enemy_spawn_timer;
    float _enemy_spawn_interval;
    int _total_score;
    bool _boss_spawned;
    entity _boss;
    std::vector<entity> _enemies;
    std::vector<entity> _projectiles;

    // Random number generation
    std::mt19937 _rng;
    uint32_t _next_net_id;

    // Logic methods
    void processEvents();
    void handleEvent(const ClientEvent &evt);
    void registerSystems();
    void handlePlayerAction(entity player, InputEvent action);

    // Game logic
    void spawnEnemy();
    void spawnBoss();
    void cleanupDeadEntities();
    entity findEntityByNetId(uint32_t net_id);

    // Systems
    static void inputSystem(registry &reg, sparse_array<InputState> &inputs,
                            sparse_array<Velocity> &velocities,
                            sparse_array<PlayerComponent> &players, float dt);

    static void movementSystem(registry &reg, sparse_array<Position> &positions,
                               sparse_array<Velocity> &velocities, float dt);

    static void weaponSystem(registry &reg, sparse_array<Weapon> &weapons,
                             sparse_array<Position> &positions,
                             sparse_array<InputState> &inputs,
                             sparse_array<PlayerComponent> &players,
                             float game_time);

    static void projectileSystem(registry &reg,
                                 sparse_array<Projectile> &projectiles,
                                 sparse_array<Position> &positions, float dt);

    static void collisionSystem(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
        sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
        sparse_array<Health> &healths, sparse_array<Score> &scores,
        sparse_array<NetworkComponent> &network_comps, float dt);

    static void healthSystem(registry &reg, sparse_array<Health> &healths,
                             sparse_array<NetworkComponent> &network_comps,
                             float dt);

    static void enemyAISystem(registry &reg, sparse_array<Enemy> &enemies,
                              sparse_array<Position> &positions,
                              sparse_array<Velocity> &velocities, float dt);

    static void bossAISystem(registry &reg, sparse_array<Boss> &bosses,
                             sparse_array<Position> &positions,
                             sparse_array<Velocity> &velocities,
                             sparse_array<Health> &healths, float dt);
};

#endif /* !GAMELOGIC_COMPLETE_HPP_ */