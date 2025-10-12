/*
** EPITECH PROJECT, 2025
** GameServer.hpp
** File description:
** Game server managing UDP communication and game entities
*/

#ifndef GAMESERVER_HPP_
#define GAMESERVER_HPP_

#include "GameSession.hpp"
#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

// Entity types matching the network protocol
enum class EntityType : uint8_t {
    PLAYER = 0x01,
    ENEMY = 0x02, // Enemy with wave movement (single OR burst shot - random)
    ENEMY_SPREAD = 0x05, // Spread shooting enemy (spread shot, zigzag movement)
    BOSS = 0x06,         // Boss enemy
    PROJECTILE = 0x03,
    ALLIED_PROJECTILE = 0x04
};

// UDP Message types
enum class UDPMessageType : uint8_t {
    CLIENT_PING = 0x00,
    PLAYER_ASSIGNMENT = 0x01,
    ENTITY_CREATE = 0x10,
    ENTITY_UPDATE = 0x11,
    ENTITY_DESTROY = 0x12,
    GAME_STATE = 0x13,
    PLAYER_INPUT = 0x20
};

// Movement pattern types for enemies
enum class MovementPattern {
    STRAIGHT, // Move straight left
    WAVE,     // Sinusoidal wave movement
    ZIGZAG    // Zigzag movement
};

// Entity structure on server
struct ServerEntity {
    uint32_t net_id;
    EntityType type;
    float pos_x;
    float pos_y;
    float vel_x; // Velocity for movement
    float vel_y;
    uint32_t health;
    uint8_t owner_player_id; // For players and their projectiles
    float width;             // Hitbox for collision
    float height;
    float last_fire_time; // For firing cooldown

    // Movement pattern data
    MovementPattern movement_pattern = MovementPattern::STRAIGHT;
    float pattern_amplitude = 0.0f;  // Amplitude for wave/zigzag
    float pattern_frequency = 0.0f;  // Frequency for wave/zigzag
    float pattern_base_speed = 0.0f; // Base horizontal speed
    float pattern_time = 0.0f;       // Time accumulator for pattern

    // Weapon data
    int projectile_count = 1; // Number of projectiles per shot
    float projectile_angle_spread =
        0.0f;                   // Angle spread for multi-projectile shots
    float fire_cooldown = 2.0f; // Time between shots
};

// UDP Packet received from client
struct UDPPacketReceived {
    std::string client_endpoint;
    std::vector<uint8_t> data;
};

/**
 * @class GameServer
 * @brief Manages UDP game server and entity synchronization
 */
class GameServer {
  public:
    GameServer(uint16_t udp_port);
    ~GameServer();

    // Start game and spawn all players
    void startGame(const GameSession &session);

    // Update game state
    void update(float dt);

    // Send updates to all clients
    void broadcastEntityUpdates();
    void broadcastGameState();

    // Handle incoming UDP packets
    void handleUDPPackets();

    // Game logic systems
    void updateEnemySpawning(float dt);
    void updateEnemyAI(float dt);
    void updateProjectiles(float dt);
    void updateCollisions();
    void cleanupDeadEntities();

    // Get UDP port
    uint16_t getPort() const { return _udp_port; }

  private:
    // Entity management
    uint32_t generateNetId();
    void spawnPlayer(uint8_t player_id, ClientId client_id);
    void spawnEnemy(float pos_x, float pos_y);
    void spawnEnemySpread(float pos_x, float pos_y);
    void spawnBoss();
    void spawnProjectile(uint32_t owner_net_id, bool is_enemy);
    void spawnMultipleProjectiles(uint32_t owner_net_id, bool is_enemy,
                                  int count, float angle_spread);
    void sendEntityCreate(const ServerEntity &entity,
                          const std::string &endpoint);
    void sendEntityDestroy(uint32_t net_id);
    void sendPlayerAssignment(uint32_t net_id, const std::string &endpoint);

    // Collision detection
    bool checkCollision(const ServerEntity &a, const ServerEntity &b);
    void damageEntity(uint32_t net_id, uint32_t damage);

    // UDP communication
    void startReceive();
    void sendUDPMessage(const std::vector<uint8_t> &data,
                        const std::string &endpoint);
    void broadcastUDPMessage(const std::vector<uint8_t> &data);

    // Serialization helpers
    std::vector<uint8_t> serializeEntityCreate(const ServerEntity &entity);
    std::vector<uint8_t> serializeEntityDestroy(uint32_t net_id);
    std::vector<uint8_t> serializePlayerAssignment(uint32_t net_id);
    std::vector<uint8_t>
    serializeEntityUpdate(const std::vector<ServerEntity> &entities);
    std::vector<uint8_t> serializeGameState();

    uint16_t _udp_port;
    std::unordered_map<uint32_t, ServerEntity> _entities;
    std::unordered_map<ClientId, std::string> _client_endpoints;
    std::unordered_map<uint8_t, uint32_t>
        _player_id_to_net_id; // player_id (1-4) -> NET_ID
    std::vector<uint32_t> _entities_to_destroy;
    uint32_t _next_net_id;
    uint32_t _game_score;

    // Game timing
    float _enemy_spawn_timer;
    float _enemy_spawn_interval;
    float _game_time;

    // Boss tracking
    bool _boss_spawned;
    uint32_t _boss_net_id;

    // UDP server
    asio::io_context _io_context;
    std::unique_ptr<asio::ip::udp::socket> _socket;
    asio::ip::udp::endpoint _remote_endpoint;
    std::thread _io_thread;
    std::mutex _mutex;
    std::array<uint8_t, 1024> _recv_buffer;
    std::vector<UDPPacketReceived> _incoming_packets;

    bool _game_started;
};

#endif /* !GAMESERVER_HPP_ */
