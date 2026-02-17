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
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <sys/types.h>
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
    uint client_id;
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
    float survival_time;
    float last_time_point_awarded;
};

struct Weapon {
    float fire_rate;
    float fire_timer;
    int weapon_type;
    int damage;
    int projectile_count = 1;   // For spread weapon
    float spread_angle = 0.0f;  // For spread weapon (degrees)
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
    float shoot_timer = 0.0f;
    float shoot_interval = 2.0f;  // Shoot every 2 seconds
};

struct Boss {
    int phase;
    float phase_timer;
    int boss_type = 1;        // 1 or 2
    float shoot_timer = 0.0f;
    int projectile_count = 5; // For spread attack
    float spread_angle = 15.0f;
};

// Power-up types
enum class PowerUpType : uint8_t { SHIELD = 0, SPREAD = 1 };

struct PowerUp {
    PowerUpType type;
    float lifetime = 30.0f;  // Power-up disappears after 30 seconds
};

struct Shield {
    int current_shield;
    int max_shield;
};

struct Hitbox {
    float width;
    float height;
    float offset_x;
    float offset_y;
};

struct NetworkComponent {
    uint net_id;
    bool needs_update;
    std::string entity_type; // "player", "enemy", "projectile", "boss"
};

// Event structure with timestamp and sequence
struct ClientEvent {
    uint client_id;
    InputEvent action;
    uint sequence_number;
    std::chrono::steady_clock::time_point timestamp;
};

// Entity state snapshot for network sync
struct EntitySnapshot {
    uint net_id;
    std::string entity_type;
    Position pos;
    Velocity vel;
    int health;
    int shield;
    int score;
    uint tick;
};

struct WorldSnapshot {
    uint tick;
    std::chrono::steady_clock::time_point timestamp;
    std::vector<EntitySnapshot> entities;
};

/**
 * @brief Class to handle the game logic for R-Type multiplayer server
 */
class GameLogic {
  public:
    GameLogic(std::shared_ptr<registry> reg, uint8_t level_id = 1);
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
    std::vector<EntitySnapshot> getDeltaSnapshot(uint last_acked_tick);
    void markEntitiesSynced();

    // Player management
    entity createPlayer(uint client_id, uint net_id, float x, float y);
    void removePlayer(uint client_id);
    entity getPlayerEntity(uint client_id);

    // Getters
    uint getCurrentTick() const { return _current_tick; }
    uint generateNetId();
    std::vector<uint> getDestroyedEntities();  // Returns and clears destroyed entity net_ids

    // Game state checks
    bool isGameOver() const { return _client_to_entity.empty() && _running; }
    bool hasPlayers() const { return !_client_to_entity.empty(); }
    bool isLevelComplete() const { return _level_complete; }

    // New entity info for network broadcasting
    struct NewEntityInfo {
        uint net_id;
        std::string entity_type;
        float x, y;
        int health;
        int shield;
    };

    // Returns newly created entities for broadcasting and clears the list
    std::vector<NewEntityInfo> getNewEntities();

  private:
    std::shared_ptr<registry> _registry;
    std::queue<ClientEvent> _event_queue;
    std::mutex _event_mutex;
    std::unordered_map<uint, entity> _client_to_entity;
    bool _running;

    // Game tick tracking for sync
    uint _current_tick;
    std::chrono::steady_clock::time_point _last_update;

    // Snapshot history
    static const size_t MAX_SNAPSHOT_HISTORY = 128;
    std::deque<WorldSnapshot> _snapshot_history;

    // Game state
    float _game_time;
    float _enemy_spawn_timer;
    float _enemy_spawn_interval;
    float _accumulator;    // For fixed timestep
    float _debug_timer;    // For debug output
    int _total_score;
    bool _boss_spawned;
    bool _boss_active;
    entity _boss;
    std::vector<entity> _boss_parts;  // For Level 2 boss (3 parts)
    std::vector<entity> _enemies;
    std::vector<entity> _projectiles;
    std::vector<entity> _powerups;
    std::vector<uint> _destroyed_net_ids;  // Net IDs of recently destroyed entities
    std::vector<NewEntityInfo> _new_entities;  // New entities waiting to be broadcast

    // Level system
    uint8_t _level_id;
    bool _endless_mode;
    bool _level_complete;  // True when boss defeated in non-endless mode
    int _next_boss_threshold;
    int _boss_count;

    // Power-up spawning
    float _powerup_spawn_timer;
    float _powerup_spawn_interval;

    // Random number generation
    std::mt19937 _rng;
    uint _next_net_id;

    // Logic methods
    void processEvents();
    void handleEvent(const ClientEvent &evt);
    void registerSystems();
    void handlePlayerAction(entity player, InputEvent action);

    // Game logic
    void spawnEnemy();
    void spawnEnemyLevel1();
    void spawnEnemyLevel2();
    void spawnBoss();
    void spawnBoss1();
    void spawnBoss2();
    void updatePlayerScores(float dt);
    void updateTotalScore();
    void checkBossSpawn();
    void spawnProjectile(float x, float y, bool is_player_projectile, int damage);
    void spawnProjectileAtAngle(float x, float y, float angle, bool is_player_projectile, int damage);
    void spawnPowerUp();
    void processPlayerShooting(float dt);
    void processEnemyShooting(float dt);
    void processBossShooting(float dt);
    void processPowerUpCollisions();
    void cleanupDeadEntities();
    entity findEntityByNetId(uint net_id);

    // Helper methods for collision system
    static void processProjectileCollisions(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
        sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
        sparse_array<Health> &healths, sparse_array<Score> &scores,
        sparse_array<NetworkComponent> &network_comps);

    static void processContactCollisions(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<PlayerComponent> &players,
        sparse_array<Enemy> &enemies, sparse_array<Health> &healths,
        sparse_array<NetworkComponent> &network_comps);

    static int applyDamageWithShield(registry &reg, size_t entity_idx, int damage);

    // Helper methods for boss shooting
    void processBossLevel2Shooting(float dt);
    void processBossLevel1Shooting(float dt);
    void updateBossPartsMovement(float dt);
    void cleanupDeadBossParts();
    void processSingleBossPartShooting(entity part, float dt);
    void processFullBossPartsShooting(float dt);

    // Helper methods for powerup collisions
    bool checkPowerUpPlayerCollision(const Position &pu_pos, const Hitbox &pu_hitbox,
                                     const Position &player_pos, const Hitbox &player_hitbox);
    void applyPowerUpToPlayer(entity player_ent, PowerUpType type);

    // Helper methods for cleanup
    void cleanupDeadPlayers();
    void cleanupDeadEnemies();
    void cleanupOutOfBoundsProjectiles();
    void cleanupDeadBoss();

    // Helper methods for boss spawning
    void spawnBossPart1(float boss_x, float boss_y, int part_hp);
    void spawnBossPart2(float boss_x, float boss_y, int part_hp);
    void spawnBossPart3(float boss_x, float boss_y, int part_hp);

    // Helper methods for player shooting
    void fireSpreadWeapon(const Position &pos, const Weapon &weapon);
    void fireSingleWeapon(const Position &pos, int damage);

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