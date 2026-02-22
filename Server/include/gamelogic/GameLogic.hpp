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
enum class PowerUpType : uint8_t { SHIELD = 0, SPREAD = 1, COMPANION = 2 };

struct PowerUp {
    PowerUpType type;
    float lifetime = 30.0f;  // Power-up disappears after 30 seconds
};

struct CompanionComponent {
    uint client_id;        // Owner player's client_id
    float shoot_timer;     // Countdown to next shot
    float shoot_interval;  // Seconds between shots (3x slower than player)
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
    /** @brief Starts the game loop */
    void start();

    /** @brief Stops the game loop */
    void stop();

    /** @brief Updates game state with fixed timestep
     * @param deltaTime Time elapsed since last frame in seconds */
    void update(float deltaTime);

    /** @brief Debug helper to print all entity positions */
    void printEntityPositions();

    // Event handling from network thread
    /** @brief Thread-safe event queue for client input
     * @param evt Client input event to process */
    void pushClientEvent(const ClientEvent &evt);

    // Snapshot management
    /** @brief Creates a snapshot of current world state
     * @return Full world snapshot with all entities */
    WorldSnapshot generateSnapshot();

    /** @brief Generates delta snapshot since last acknowledged tick
     * @param last_acked_tick Last tick client confirmed receiving
     * @return List of entities that changed since last_acked_tick */
    std::vector<EntitySnapshot> getDeltaSnapshot(uint last_acked_tick);

    /** @brief Clears needs_update flags on all networked entities */
    void markEntitiesSynced();

    // Player management
    /** @brief Spawns a new player entity
     * @param client_id Unique client identifier
     * @param net_id Network ID for this entity
     * @param x Initial X position (0.0-1.0)
     * @param y Initial Y position (0.0-1.0)
     * @return Entity handle for the spawned player */
    entity createPlayer(uint client_id, uint net_id, float x, float y);

    /** @brief Removes player and destroys their entity
     * @param client_id Client to remove */
    void removePlayer(uint client_id);

    /** @brief Retrieves entity handle for a client
     * @param client_id Client to lookup
     * @return Entity handle or invalid entity if not found */
    entity getPlayerEntity(uint client_id);

    // Getters
    /** @brief Current server tick counter */
    uint getCurrentTick() const { return _current_tick; }

    /** @brief Generates unique network ID for new entities
     * @return Monotonically increasing network ID */
    uint generateNetId();

    /** @brief Returns and clears list of destroyed entity net IDs
     * @return List of net IDs destroyed since last call */
    std::vector<uint> getDestroyedEntities();

    // Game state checks
    /** @brief Checks if game is over (no players left) */
    bool isGameOver() const { return _client_to_entity.empty() && _running; }

    /** @brief Checks if any players are connected */
    bool hasPlayers() const { return !_client_to_entity.empty(); }

    /** @brief Checks if level is complete (boss defeated) */
    bool isLevelComplete() const { return _level_complete; }

    /** @brief Information about newly spawned entities for network sync */
    struct NewEntityInfo {
        uint net_id;           ///< Network identifier
        std::string entity_type; ///< Entity type ("player", "enemy", "boss", etc.)
        float x, y;            ///< Spawn position (normalized 0.0-1.0)
        int health;            ///< Initial health points
        int shield;            ///< Initial shield points
    };

    /** @brief Returns and clears list of newly spawned entities
     * @return List of entities created since last call */
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

    // Companions (one per player, keyed by client_id)
    std::unordered_map<uint, entity> _player_companions;

    // Random number generation
    std::mt19937 _rng;
    uint _next_net_id;

    // Logic methods
    /** @brief Processes queued client input events */
    void processEvents();

    /** @brief Handles a single client event
     * @param evt Event to process */
    void handleEvent(const ClientEvent &evt);

    /** @brief Registers all ECS systems with the registry */
    void registerSystems();

    /** @brief Applies player input action to entity
     * @param player Player entity to control
     * @param action Input action to apply */
    void handlePlayerAction(entity player, InputEvent action);

    // Game logic
    /** @brief Spawns enemy based on current level */
    void spawnEnemy();

    /** @brief Spawns level 1 enemy (4 types) */
    void spawnEnemyLevel1();

    /** @brief Spawns level 2 enemy (spread or normal) */
    void spawnEnemyLevel2();

    /** @brief Spawns boss based on current level */
    void spawnBoss();

    /** @brief Spawns level 1 boss (single entity) */
    void spawnBoss1();

    /** @brief Spawns level 2 boss (3 parts) */
    void spawnBoss2();

    /** @brief Updates player survival time and score awards
     * @param dt Delta time in seconds */
    void updatePlayerScores(float dt);

    /** @brief Updates total score from all active players */
    void updateTotalScore();

    /** @brief Checks if boss should spawn based on score threshold */
    void checkBossSpawn();

    /** @brief Spawns a straight projectile
     * @param x X position
     * @param y Y position
     * @param is_player_projectile True if fired by player
     * @param damage Damage dealt on hit */
    void spawnProjectile(float x, float y, bool is_player_projectile, int damage);

    /** @brief Spawns an angled projectile
     * @param x X position
     * @param y Y position
     * @param angle Firing angle in degrees
     * @param is_player_projectile True if fired by player
     * @param damage Damage dealt on hit */
    void spawnProjectileAtAngle(float x, float y, float angle, bool is_player_projectile, int damage);

    /** @brief Spawns a power-up (shield, spread, or companion) */
    void spawnPowerUp();

    /** @brief Spawns a companion entity for the given player
     * @param player_ent Player entity that collected the companion power-up
     * @param client_id  Client ID of the owning player */
    void spawnCompanionForPlayer(entity player_ent, uint client_id);

    /** @brief Updates all active companions: follow player, auto-fire
     * @param dt Delta time in seconds */
    void updateCompanions(float dt);

    /** @brief Processes player weapon firing
     * @param dt Delta time in seconds */
    void processPlayerShooting(float dt);

    /** @brief Processes enemy shooting behavior
     * @param dt Delta time in seconds */
    void processEnemyShooting(float dt);

    /** @brief Processes boss shooting patterns
     * @param dt Delta time in seconds */
    void processBossShooting(float dt);

    /** @brief Detects and applies power-up pickups */
    void processPowerUpCollisions();

    /** @brief Removes dead entities and out-of-bounds projectiles */
    void cleanupDeadEntities();

    /** @brief Finds entity by network ID
     * @param net_id Network ID to search for
     * @return Entity handle or invalid entity if not found */
    entity findEntityByNetId(uint net_id);

    // Helper methods for collision system
    /** @brief Handles projectile vs entity collisions and damage */
    static void processProjectileCollisions(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
        sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
        sparse_array<Health> &healths, sparse_array<Score> &scores,
        sparse_array<NetworkComponent> &network_comps);

    /** @brief Handles player-enemy contact damage */
    static void processContactCollisions(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<PlayerComponent> &players,
        sparse_array<Enemy> &enemies, sparse_array<Health> &healths,
        sparse_array<NetworkComponent> &network_comps);

    /** @brief Applies damage with shield absorption
     * @param reg Registry reference
     * @param entity_idx Entity index to damage
     * @param damage Incoming damage amount
     * @return Remaining damage after shield absorption */
    static int applyDamageWithShield(registry &reg, size_t entity_idx, int damage);

    // Helper methods for boss shooting
    /** @brief Handles level 2 boss (3-part) shooting patterns */
    void processBossLevel2Shooting(float dt);

    /** @brief Handles level 1 boss (single) shooting pattern */
    void processBossLevel1Shooting(float dt);

    /** @brief Updates sine wave movement for all boss parts */
    void updateBossPartsMovement(float dt);

    /** @brief Removes destroyed boss parts from _boss_parts vector */
    void cleanupDeadBossParts();

    /** @brief Handles shooting for last remaining boss part */
    void processSingleBossPartShooting(entity part, float dt);

    /** @brief Handles shooting when all 3 boss parts are alive */
    void processFullBossPartsShooting(float dt);

    // Helper methods for powerup collisions
    /** @brief Checks AABB collision between power-up and player
     * @return True if colliding */
    bool checkPowerUpPlayerCollision(const Position &pu_pos, const Hitbox &pu_hitbox,
                                     const Position &player_pos, const Hitbox &player_hitbox);

    /** @brief Applies power-up effect to player (shield or spread) */
    void applyPowerUpToPlayer(entity player_ent, PowerUpType type);

    // Helper methods for cleanup
    /** @brief Removes dead players from _client_to_entity map */
    void cleanupDeadPlayers();

    /** @brief Removes dead or off-screen enemies */
    void cleanupDeadEnemies();

    /** @brief Removes projectiles that are out of bounds or expired */
    void cleanupOutOfBoundsProjectiles();

    /** @brief Removes dead boss (level 1 only) */
    void cleanupDeadBoss();

    // Helper methods for boss spawning
    /** @brief Spawns left boss part for level 2 boss */
    void spawnBossPart1(float boss_x, float boss_y, int part_hp);

    /** @brief Spawns center boss part for level 2 boss */
    void spawnBossPart2(float boss_x, float boss_y, int part_hp);

    /** @brief Spawns right boss part for level 2 boss */
    void spawnBossPart3(float boss_x, float boss_y, int part_hp);

    // Helper methods for player shooting
    /** @brief Fires multiple projectiles in a spread pattern */
    void fireSpreadWeapon(const Position &pos, const Weapon &weapon);

    /** @brief Fires a single straight projectile */
    void fireSingleWeapon(const Position &pos, int damage);

    // Systems
    /** @brief Converts player input to velocity (WASD movement) */
    static void inputSystem(registry &reg, sparse_array<InputState> &inputs,
                            sparse_array<Velocity> &velocities,
                            sparse_array<PlayerComponent> &players, float dt);

    /** @brief Updates positions based on velocity, clamps player bounds */
    static void movementSystem(registry &reg, sparse_array<Position> &positions,
                               sparse_array<Velocity> &velocities, float dt);

    /** @brief Decrements weapon fire timers */
    static void weaponSystem(registry &reg, sparse_array<Weapon> &weapons,
                             sparse_array<Position> &positions,
                             sparse_array<InputState> &inputs,
                             sparse_array<PlayerComponent> &players,
                             float game_time);

    /** @brief Decrements projectile lifetime */
    static void projectileSystem(registry &reg,
                                 sparse_array<Projectile> &projectiles,
                                 sparse_array<Position> &positions, float dt);

    /** @brief Detects and resolves all collision types (projectiles + contact) */
    static void collisionSystem(
        registry &reg, sparse_array<Position> &positions,
        sparse_array<Hitbox> &hitboxes, sparse_array<Projectile> &projectiles,
        sparse_array<PlayerComponent> &players, sparse_array<Enemy> &enemies,
        sparse_array<Health> &healths, sparse_array<Score> &scores,
        sparse_array<NetworkComponent> &network_comps, float dt);

    /** @brief Updates health states and invulnerability timers */
    static void healthSystem(registry &reg, sparse_array<Health> &healths,
                             sparse_array<NetworkComponent> &network_comps,
                             float dt);

    /** @brief Updates enemy movement patterns (zigzag, sine wave) */
    static void enemyAISystem(registry &reg, sparse_array<Enemy> &enemies,
                              sparse_array<Position> &positions,
                              sparse_array<Velocity> &velocities, float dt);

    /** @brief Boss AI system (currently unused, logic in processBossShooting) */
    static void bossAISystem(registry &reg, sparse_array<Boss> &bosses,
                             sparse_array<Position> &positions,
                             sparse_array<Velocity> &velocities,
                             sparse_array<Health> &healths, float dt);
};

#endif /* !GAMELOGIC_COMPLETE_HPP_ */