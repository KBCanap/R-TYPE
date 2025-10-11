/*
** EPITECH PROJECT, 2025
** GameServer.cpp
** File description:
** Implementation of game server
*/

#include "GameServer.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

// Player spawn positions (relative to screen width/height)
static constexpr float PLAYER_SPAWN_POSITIONS[][2] = {
    {0.125f, 0.25f},  // Player 1: top-left
    {0.125f, 0.50f},  // Player 2: middle-left
    {0.125f, 0.75f},  // Player 3: bottom-left
    {0.175f, 0.50f}   // Player 4: middle (slightly right)
};

static constexpr uint32_t PLAYER_DEFAULT_HEALTH = 100;

GameServer::GameServer(uint16_t udp_port)
    : _udp_port(udp_port), _next_net_id(1), _game_score(0),
      _enemy_spawn_timer(0.0f), _enemy_spawn_interval(2.0f),
      _game_time(0.0f), _boss_spawned(false), _boss_net_id(0), _game_started(false)
{
    try {
        _socket = std::make_unique<asio::ip::udp::socket>(
            _io_context,
            asio::ip::udp::endpoint(asio::ip::udp::v4(), udp_port)
        );

        startReceive();
        _io_thread = std::thread([this]() { _io_context.run(); });

        std::cout << "UDP Game Server started on port " << udp_port << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start UDP server: " << e.what() << std::endl;
        throw;
    }
}

GameServer::~GameServer()
{
    _io_context.stop();
    if (_io_thread.joinable()) {
        _io_thread.join();
    }
    std::cout << "Game Server stopped" << std::endl;
}

void GameServer::startGame(const GameSession& session)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_game_started) {
        std::cerr << "Game already started!" << std::endl;
        return;
    }

    auto connected_clients = session.getConnectedClients();
    std::cout << "Starting game with " << connected_clients.size() << " players" << std::endl;

    // Spawn all players
    uint8_t player_index = 0;
    for (ClientId client_id : connected_clients) {
        spawnPlayer(player_index + 1, client_id);
        player_index++;

        if (player_index >= 4) break; // Max 4 players
    }

    _game_started = true;
    std::cout << "All players spawned successfully!" << std::endl;
}

void GameServer::spawnPlayer(uint8_t player_id, ClientId /*client_id*/)
{
    uint32_t net_id = generateNetId();

    // Calculate spawn position (player_id is 1-indexed)
    int spawn_index = (player_id - 1) % 4;
    float spawn_x = PLAYER_SPAWN_POSITIONS[spawn_index][0];
    float spawn_y = PLAYER_SPAWN_POSITIONS[spawn_index][1];

    ServerEntity player;
    player.net_id = net_id;
    player.type = EntityType::PLAYER;
    player.pos_x = spawn_x;
    player.pos_y = spawn_y;
    player.vel_x = 0.0f;
    player.vel_y = 0.0f;
    player.health = PLAYER_DEFAULT_HEALTH;
    player.owner_player_id = player_id;
    player.width = 0.05f;  // 5% of screen width
    player.height = 0.05f; // 5% of screen height
    player.last_fire_time = 0.0f;

    _entities[net_id] = player;
    _player_id_to_net_id[player_id] = net_id;
}

void GameServer::update(float dt)
{
    if (!_game_started) {
        return;
    }

    _game_time += dt;

    handleUDPPackets();
    updateEnemySpawning(dt);
    updateEnemyAI(dt);
    updateProjectiles(dt);
    updateCollisions();
    cleanupDeadEntities();
    broadcastEntityUpdates();
    broadcastGameState();
}

void GameServer::handleUDPPackets()
{
    std::vector<UDPPacketReceived> packets;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        packets = std::move(_incoming_packets);
        _incoming_packets.clear();
    }

    for (const auto& packet : packets) {
        if (packet.data.size() < 5) {
            continue; // Invalid packet
        }

        uint8_t msg_type = packet.data[0];

        if (msg_type == static_cast<uint8_t>(UDPMessageType::CLIENT_PING)) {
            // Handle client ping - extract player_id and send PLAYER_ASSIGNMENT
            if (packet.data.size() >= 8 + 5) { // header + 5 bytes payload (4 timestamp + 1 player_id)
                std::lock_guard<std::mutex> lock(_mutex);

                // Extract player_id from payload (last byte)
                uint8_t player_id = packet.data[8 + 4]; // offset 8 (header) + 4 (timestamp)


                // Find this player's NET_ID using the player_id (1-4)
                auto it = _player_id_to_net_id.find(player_id);
                if (it != _player_id_to_net_id.end()) {
                    uint32_t net_id = it->second;

                    // Check if we already registered this endpoint
                    bool is_new_endpoint = true;
                    for (const auto& [cid, endpoint] : _client_endpoints) {
                        if (endpoint == packet.client_endpoint) {
                            is_new_endpoint = false;
                            break;
                        }
                    }

                    if (is_new_endpoint) {
                        // Store endpoint for this player
                        ClientId temp_client_id = static_cast<ClientId>(player_id);
                        _client_endpoints[temp_client_id] = packet.client_endpoint;

                        sendPlayerAssignment(net_id, packet.client_endpoint);

                        // Send all existing entities to this new client
                        for (const auto& [eid, entity] : _entities) {
                            sendEntityCreate(entity, packet.client_endpoint);
                        }

                    }
                } else {
                    std::cerr << "[GameServer] Unknown player_id: " << static_cast<int>(player_id) << std::endl;
                }
            }
        }
        else if (msg_type == static_cast<uint8_t>(UDPMessageType::PLAYER_INPUT)) {
            // Handle player input - update player position
            if (packet.data.size() >= 8 + 2) { // header + 2 bytes payload
                uint8_t direction = packet.data[9];

                // Find which player this is from (by endpoint)
                std::lock_guard<std::mutex> lock(_mutex);
                for (const auto& [temp_client_id, endpoint] : _client_endpoints) {
                    if (endpoint == packet.client_endpoint) {
                        // temp_client_id is actually player_id (1-4)
                        uint8_t player_id = static_cast<uint8_t>(temp_client_id);

                        // Get NET_ID from player_id
                        auto it = _player_id_to_net_id.find(player_id);
                        if (it != _player_id_to_net_id.end()) {
                            uint32_t net_id = it->second;
                            auto entity_it = _entities.find(net_id);
                            if (entity_it != _entities.end()) {
                                // Update player position based on input
                                float move_speed = 0.005f; // Increased movement speed
                                if (direction & 0x01) entity_it->second.pos_y -= move_speed; // UP
                                if (direction & 0x02) entity_it->second.pos_y += move_speed; // DOWN
                                if (direction & 0x04) entity_it->second.pos_x -= move_speed; // LEFT
                                if (direction & 0x08) entity_it->second.pos_x += move_speed; // RIGHT

                                // Check if player wants to shoot (SPACE = 0x10)
                                if (direction & 0x10) {
                                    float cooldown = 0.3f; // 300ms between shots
                                    if (_game_time - entity_it->second.last_fire_time >= cooldown) {
                                        spawnProjectile(net_id, false);
                                        entity_it->second.last_fire_time = _game_time;
                                    }
                                }

                                // Clamp positions to 0.0 - 1.0 (relative coordinates)
                                entity_it->second.pos_x = std::max(0.0f, std::min(1.0f, entity_it->second.pos_x));
                                entity_it->second.pos_y = std::max(0.0f, std::min(1.0f, entity_it->second.pos_y));

                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

void GameServer::broadcastEntityUpdates()
{
    if (!_game_started || _entities.empty()) {
        return;
    }

    std::vector<ServerEntity> entities_to_sync;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& [net_id, entity] : _entities) {
            entities_to_sync.push_back(entity);
        }
    }

    auto update_packet = serializeEntityUpdate(entities_to_sync);
    broadcastUDPMessage(update_packet);
}

void GameServer::broadcastGameState()
{
    if (!_game_started) {
        return;
    }

    auto game_state_packet = serializeGameState();
    broadcastUDPMessage(game_state_packet);
}

void GameServer::sendEntityCreate(const ServerEntity& entity, const std::string& endpoint)
{
    auto packet = serializeEntityCreate(entity);
    sendUDPMessage(packet, endpoint);
}

void GameServer::sendPlayerAssignment(uint32_t net_id, const std::string& endpoint)
{
    auto packet = serializePlayerAssignment(net_id);
    sendUDPMessage(packet, endpoint);
}

uint32_t GameServer::generateNetId()
{
    return _next_net_id++;
}

// UDP Communication
void GameServer::startReceive()
{
    _socket->async_receive_from(
        asio::buffer(_recv_buffer), _remote_endpoint,
        [this](std::error_code ec, std::size_t bytes_received) {
            if (!ec && bytes_received > 0) {
                UDPPacketReceived packet;
                packet.client_endpoint = _remote_endpoint.address().to_string() + ":" +
                                        std::to_string(_remote_endpoint.port());
                packet.data.assign(_recv_buffer.begin(), _recv_buffer.begin() + bytes_received);

                std::lock_guard<std::mutex> lock(_mutex);
                _incoming_packets.push_back(packet);
            }
            startReceive();
        }
    );
}

void GameServer::sendUDPMessage(const std::vector<uint8_t>& data, const std::string& endpoint)
{
    try {
        // Parse endpoint (format: "ip:port")
        size_t colon_pos = endpoint.find(':');
        if (colon_pos == std::string::npos) {
            return;
        }

        std::string ip = endpoint.substr(0, colon_pos);
        uint16_t port = static_cast<uint16_t>(std::stoi(endpoint.substr(colon_pos + 1)));

        asio::ip::udp::endpoint remote(asio::ip::make_address(ip), port);
        _socket->send_to(asio::buffer(data), remote);
    } catch (const std::exception& e) {
        std::cerr << "Failed to send UDP message: " << e.what() << std::endl;
    }
}

void GameServer::broadcastUDPMessage(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (const auto& [client_id, endpoint] : _client_endpoints) {
        sendUDPMessage(data, endpoint);
    }
}

// Serialization
std::vector<uint8_t> GameServer::serializeEntityCreate(const ServerEntity& entity)
{
    std::vector<uint8_t> packet;

    // Message type (1 byte)
    packet.push_back(static_cast<uint8_t>(UDPMessageType::ENTITY_CREATE));

    // Data length (3 bytes, 24-bit) - always 17 for ENTITY_CREATE payload
    packet.push_back(0);           // High byte
    packet.push_back(0);           // Middle byte
    packet.push_back(17);          // Low byte

    // Sequence number (4 bytes) - can be 0 for now
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);

    // Payload (17 bytes):
    // NET_ID (4 bytes)
    packet.push_back((entity.net_id >> 24) & 0xFF);
    packet.push_back((entity.net_id >> 16) & 0xFF);
    packet.push_back((entity.net_id >> 8) & 0xFF);
    packet.push_back(entity.net_id & 0xFF);

    // Entity type (1 byte)
    packet.push_back(static_cast<uint8_t>(entity.type));

    // Health (4 bytes)
    packet.push_back((entity.health >> 24) & 0xFF);
    packet.push_back((entity.health >> 16) & 0xFF);
    packet.push_back((entity.health >> 8) & 0xFF);
    packet.push_back(entity.health & 0xFF);

    // Position X (4 bytes as float)
    uint32_t pos_x_bits;
    std::memcpy(&pos_x_bits, &entity.pos_x, sizeof(float));
    packet.push_back((pos_x_bits >> 24) & 0xFF);
    packet.push_back((pos_x_bits >> 16) & 0xFF);
    packet.push_back((pos_x_bits >> 8) & 0xFF);
    packet.push_back(pos_x_bits & 0xFF);

    // Position Y (4 bytes as float)
    uint32_t pos_y_bits;
    std::memcpy(&pos_y_bits, &entity.pos_y, sizeof(float));
    packet.push_back((pos_y_bits >> 24) & 0xFF);
    packet.push_back((pos_y_bits >> 16) & 0xFF);
    packet.push_back((pos_y_bits >> 8) & 0xFF);
    packet.push_back(pos_y_bits & 0xFF);

    return packet;
}

std::vector<uint8_t> GameServer::serializePlayerAssignment(uint32_t net_id)
{
    std::vector<uint8_t> packet;

    // Message type (1 byte)
    packet.push_back(static_cast<uint8_t>(UDPMessageType::PLAYER_ASSIGNMENT));

    // Data length (3 bytes, 24-bit) - always 4 for PLAYER_ASSIGNMENT
    packet.push_back(0);           // High byte
    packet.push_back(0);           // Middle byte
    packet.push_back(4);           // Low byte

    // Sequence number (4 bytes)
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);

    // Payload: NET_ID (4 bytes)
    packet.push_back((net_id >> 24) & 0xFF);
    packet.push_back((net_id >> 16) & 0xFF);
    packet.push_back((net_id >> 8) & 0xFF);
    packet.push_back(net_id & 0xFF);

    return packet;
}

std::vector<uint8_t> GameServer::serializeEntityUpdate(const std::vector<ServerEntity>& entities)
{
    std::vector<uint8_t> packet;

    // Message type (1 byte)
    packet.push_back(static_cast<uint8_t>(UDPMessageType::ENTITY_UPDATE));

    // Data length (3 bytes, 24-bit) - 16 bytes per entity
    uint32_t data_length = static_cast<uint32_t>(entities.size() * 16);
    packet.push_back((data_length >> 16) & 0xFF);  // High byte
    packet.push_back((data_length >> 8) & 0xFF);   // Middle byte
    packet.push_back(data_length & 0xFF);          // Low byte

    // Sequence number (4 bytes)
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);

    // Payload: for each entity (16 bytes)
    for (const auto& entity : entities) {
        // NET_ID (4 bytes)
        packet.push_back((entity.net_id >> 24) & 0xFF);
        packet.push_back((entity.net_id >> 16) & 0xFF);
        packet.push_back((entity.net_id >> 8) & 0xFF);
        packet.push_back(entity.net_id & 0xFF);

        // Health (4 bytes)
        packet.push_back((entity.health >> 24) & 0xFF);
        packet.push_back((entity.health >> 16) & 0xFF);
        packet.push_back((entity.health >> 8) & 0xFF);
        packet.push_back(entity.health & 0xFF);

        // Position X (4 bytes)
        uint32_t pos_x_bits;
        std::memcpy(&pos_x_bits, &entity.pos_x, sizeof(float));
        packet.push_back((pos_x_bits >> 24) & 0xFF);
        packet.push_back((pos_x_bits >> 16) & 0xFF);
        packet.push_back((pos_x_bits >> 8) & 0xFF);
        packet.push_back(pos_x_bits & 0xFF);

        // Position Y (4 bytes)
        uint32_t pos_y_bits;
        std::memcpy(&pos_y_bits, &entity.pos_y, sizeof(float));
        packet.push_back((pos_y_bits >> 24) & 0xFF);
        packet.push_back((pos_y_bits >> 16) & 0xFF);
        packet.push_back((pos_y_bits >> 8) & 0xFF);
        packet.push_back(pos_y_bits & 0xFF);
    }

    return packet;
}

std::vector<uint8_t> GameServer::serializeEntityDestroy(uint32_t net_id)
{
    std::vector<uint8_t> packet;

    // Message type (1 byte)
    packet.push_back(static_cast<uint8_t>(UDPMessageType::ENTITY_DESTROY));

    // Data length (3 bytes, 24-bit) - always 4 for ENTITY_DESTROY
    packet.push_back(0);           // High byte
    packet.push_back(0);           // Middle byte
    packet.push_back(4);           // Low byte

    // Sequence number (4 bytes)
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);

    // Payload: NET_ID (4 bytes)
    packet.push_back((net_id >> 24) & 0xFF);
    packet.push_back((net_id >> 16) & 0xFF);
    packet.push_back((net_id >> 8) & 0xFF);
    packet.push_back(net_id & 0xFF);

    return packet;
}

std::vector<uint8_t> GameServer::serializeGameState()
{
    std::vector<uint8_t> packet;

    // Message type (1 byte)
    packet.push_back(static_cast<uint8_t>(UDPMessageType::GAME_STATE));

    // Data length (3 bytes, 24-bit) - 4 bytes for game score
    packet.push_back(0);           // High byte
    packet.push_back(0);           // Middle byte
    packet.push_back(4);           // Low byte (4 bytes payload)

    // Sequence number (4 bytes)
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);

    // Payload: game_score (4 bytes)
    packet.push_back((_game_score >> 24) & 0xFF);
    packet.push_back((_game_score >> 16) & 0xFF);
    packet.push_back((_game_score >> 8) & 0xFF);
    packet.push_back(_game_score & 0xFF);

    return packet;
}

// Game Logic Systems
void GameServer::updateEnemySpawning(float dt)
{
    // Check if boss should spawn (at 100 points)
    if (!_boss_spawned && _game_score >= 100) {
        std::lock_guard<std::mutex> lock(_mutex);
        spawnBoss();
        _boss_spawned = true;
        std::cout << "[GAME] Boss spawned at score " << _game_score << std::endl;
        return; // Don't spawn regular enemies when boss spawns
    }

    _enemy_spawn_timer += dt;

    if (_enemy_spawn_timer >= _enemy_spawn_interval) {
        _enemy_spawn_timer = 0.0f;

        // Spawn enemy at random Y position on the right side
        float spawn_y = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; // Random between 0.2 and 0.8

        std::lock_guard<std::mutex> lock(_mutex);

        // Randomly choose enemy type (50% ENEMY, 50% ENEMY_SPREAD) - exactly like solo
        int enemy_type = rand() % 2;  // 0 or 1
        if (enemy_type == 0) {
            spawnEnemy(0.95f, spawn_y);  // Enemy with wave movement (weapon_type 0 or 1)
        } else {
            spawnEnemySpread(0.95f, spawn_y);  // Spread enemy with zigzag movement (weapon_type 2)
        }

        // Gradually increase spawn rate
        if (_enemy_spawn_interval > 0.8f) {
            _enemy_spawn_interval -= 0.01f;
        }
    }
}

void GameServer::spawnEnemy(float pos_x, float pos_y)
{
    // NOTE: Caller must already hold _mutex lock
    uint32_t net_id = generateNetId();

    ServerEntity enemy;
    enemy.net_id = net_id;
    enemy.type = EntityType::ENEMY;
    enemy.pos_x = pos_x;
    enemy.pos_y = pos_y;
    enemy.vel_x = 0.0f;  // Velocity will be calculated by pattern
    enemy.vel_y = 0.0f;
    enemy.health = 10;   // 10 HP for 1-hit kill
    enemy.owner_player_id = 0;
    enemy.width = 0.05f;
    enemy.height = 0.05f;
    enemy.last_fire_time = _game_time;

    // Wave movement pattern (exactly like solo mode)
    enemy.movement_pattern = MovementPattern::WAVE;
    enemy.pattern_amplitude = 50.0f;      // 50 pixels amplitude
    enemy.pattern_frequency = 0.01f;      // Frequency from solo mode
    enemy.pattern_base_speed = 120.0f;    // 120 pixels/sec
    enemy.pattern_time = 0.0f;

    // Randomly choose weapon: 50% single shot (weapon_type 0), 50% burst (weapon_type 1)
    int weapon_variant = rand() % 2;
    if (weapon_variant == 0) {
        // Single shot weapon (weapon_type 0 in solo)
        enemy.projectile_count = 1;
        enemy.projectile_angle_spread = 0.0f;
        enemy.fire_cooldown = 1.0f + (static_cast<float>(rand()) / RAND_MAX);  // 1-2 seconds
    } else {
        // Burst weapon (weapon_type 1 in solo) - Note: burst not implemented yet, use single for now
        enemy.projectile_count = 1;
        enemy.projectile_angle_spread = 0.0f;
        enemy.fire_cooldown = 1.0f + (static_cast<float>(rand()) / RAND_MAX);  // 1-2 seconds
        // TODO: Implement burst firing mechanism
    }

    _entities[net_id] = enemy;

    // Broadcast entity creation to all clients
    for (const auto& [client_id, endpoint] : _client_endpoints) {
        sendEntityCreate(enemy, endpoint);
    }
}

void GameServer::spawnEnemySpread(float pos_x, float pos_y)
{
    // NOTE: Caller must already hold _mutex lock
    uint32_t net_id = generateNetId();

    ServerEntity enemy;
    enemy.net_id = net_id;
    enemy.type = EntityType::ENEMY_SPREAD;
    enemy.pos_x = pos_x;
    enemy.pos_y = pos_y;
    enemy.vel_x = 0.0f;  // Velocity will be calculated by pattern
    enemy.vel_y = 0.0f;
    enemy.health = 10;   // 10 HP for 1-hit kill
    enemy.owner_player_id = 0;
    enemy.width = 0.05f;  // Slightly larger hitbox
    enemy.height = 0.05f;
    enemy.last_fire_time = _game_time;

    // Zigzag movement pattern (exactly like solo mode)
    enemy.movement_pattern = MovementPattern::ZIGZAG;
    enemy.pattern_amplitude = 60.0f;     // 60 pixels amplitude
    enemy.pattern_frequency = 0.015f;    // Frequency from solo mode
    enemy.pattern_base_speed = 130.0f;   // 130 pixels/sec
    enemy.pattern_time = 0.0f;

    // Spread weapon (3 projectiles with 20 degree spread, exactly like solo)
    enemy.projectile_count = 3;
    enemy.projectile_angle_spread = 20.0f;
    enemy.fire_cooldown = 1.0f / 0.8f;  // Weapon fire_rate = 0.8, so interval = 1/0.8 = 1.25s

    _entities[net_id] = enemy;

    // Broadcast entity creation to all clients
    for (const auto& [client_id, endpoint] : _client_endpoints) {
        sendEntityCreate(enemy, endpoint);
    }
}

void GameServer::spawnBoss()
{
    // NOTE: Caller must already hold _mutex lock
    uint32_t net_id = generateNetId();

    ServerEntity boss;
    boss.net_id = net_id;
    boss.type = EntityType::BOSS;
    boss.pos_x = 0.85f;  // Near right side but not at edge
    boss.pos_y = 0.5f;   // Start in middle
    boss.vel_x = 0.0f;
    boss.vel_y = 100.0f;  // Vertical movement speed 100 pixels/sec (exactly like solo)
    boss.health = 1000;   // Boss has 1000 HP (exactly like solo)
    boss.owner_player_id = 0;
    boss.width = 0.08f;   // Larger hitbox for boss
    boss.height = 0.14f;
    boss.last_fire_time = _game_time;

    // Boss movement - straight up/down (bounce handled in AI, exactly like solo)
    boss.movement_pattern = MovementPattern::STRAIGHT;
    boss.pattern_amplitude = 0.0f;
    boss.pattern_frequency = 0.0f;
    boss.pattern_base_speed = 0.0f;
    boss.pattern_time = 0.0f;

    // Boss weapon: spread pattern with high fire rate (exactly like solo)
    // fire_rate = 2.0 means 1/2.0 = 0.5 seconds between shots
    boss.projectile_count = 5;
    boss.projectile_angle_spread = 15.0f;
    boss.fire_cooldown = 1.0f / 2.0f;  // fire_rate = 2.0, so interval = 0.5 seconds

    _entities[net_id] = boss;
    _boss_net_id = net_id;

    // Broadcast entity creation to all clients
    for (const auto& [client_id, endpoint] : _client_endpoints) {
        sendEntityCreate(boss, endpoint);
    }

    std::cout << "[GAME] Boss created with NET_ID=" << net_id << std::endl;
}

void GameServer::updateEnemyAI(float dt)
{
    std::lock_guard<std::mutex> lock(_mutex);

    // We assume a reference screen size of 1920x1080 for consistent velocity calculations
    const float REFERENCE_WIDTH = 1920.0f;
    const float REFERENCE_HEIGHT = 1080.0f;

    for (auto& [net_id, entity] : _entities) {
        if (entity.type == EntityType::ENEMY || entity.type == EntityType::ENEMY_SPREAD) {
            // Update movement pattern time
            entity.pattern_time += dt;

            // Calculate movement based on pattern (EXACTLY like solo mode)
            float vel_x_pixels = 0.0f;
            float vel_y_pixels = 0.0f;

            if (entity.movement_pattern == MovementPattern::WAVE) {
                // Wave pattern: sinusoidal vertical movement based on X position
                // Exactly like solo: vy = amplitude * sin(frequency * pos_x + phase_offset)
                vel_x_pixels = -entity.pattern_base_speed;
                float pos_x_pixels = entity.pos_x * REFERENCE_WIDTH;
                vel_y_pixels = entity.pattern_amplitude * std::sin(entity.pattern_frequency * pos_x_pixels);
            }
            else if (entity.movement_pattern == MovementPattern::ZIGZAG) {
                // Zigzag pattern: triangular wave based on time
                // Exactly like solo: triangle wave using time
                vel_x_pixels = -entity.pattern_base_speed;
                float triangle_wave = std::abs(std::fmod(entity.pattern_frequency * entity.pattern_time, 2.0f) - 1.0f) * 2.0f - 1.0f;
                vel_y_pixels = entity.pattern_amplitude * triangle_wave;
            }
            else {
                // Straight movement
                vel_x_pixels = -entity.pattern_base_speed;
                vel_y_pixels = 0.0f;
            }

            // Convert pixel velocities to relative velocities and apply dt
            entity.vel_x = vel_x_pixels / REFERENCE_WIDTH;
            entity.vel_y = vel_y_pixels / REFERENCE_HEIGHT;

            // Move enemy
            entity.pos_x += entity.vel_x * dt;
            entity.pos_y += entity.vel_y * dt;

            // Clamp Y position to screen bounds
            entity.pos_y = std::max(0.0f, std::min(1.0f, entity.pos_y));

            // Remove enemies that went off-screen (left side)
            if (entity.pos_x < -0.1f) {
                _entities_to_destroy.push_back(net_id);
                continue;
            }

            // Enemy shooting logic
            if (_game_time - entity.last_fire_time >= entity.fire_cooldown) {
                if (entity.projectile_count > 1) {
                    std::cout << "[SHOOT] " << (entity.type == EntityType::ENEMY_SPREAD ? "ENEMY_SPREAD" : "ENEMY")
                              << " NET_ID=" << net_id << " firing " << entity.projectile_count
                              << " projectiles with " << entity.projectile_angle_spread << "° spread" << std::endl;
                    spawnMultipleProjectiles(net_id, true, entity.projectile_count, entity.projectile_angle_spread);
                } else {
                    spawnProjectile(net_id, true);
                }
                entity.last_fire_time = _game_time;
            }
        }
        else if (entity.type == EntityType::BOSS) {
            // Boss vertical movement with bounce (EXACTLY like solo mode)
            // Convert pixel velocity to relative velocity
            float vel_y_relative = entity.vel_y / REFERENCE_HEIGHT;
            entity.pos_y += vel_y_relative * dt;

            // Bounce at screen edges (exactly like solo: 50px top, 100px bottom margin)
            float top_margin = 50.0f / REFERENCE_HEIGHT;
            float bottom_margin = (REFERENCE_HEIGHT - 100.0f) / REFERENCE_HEIGHT;

            if (entity.pos_y <= top_margin) {
                entity.pos_y = top_margin;
                entity.vel_y = std::abs(entity.vel_y);  // Bounce down
            } else if (entity.pos_y >= bottom_margin) {
                entity.pos_y = bottom_margin;
                entity.vel_y = -std::abs(entity.vel_y);  // Bounce up
            }

            // Boss shooting logic
            if (_game_time - entity.last_fire_time >= entity.fire_cooldown) {
                spawnMultipleProjectiles(net_id, true, entity.projectile_count, entity.projectile_angle_spread);
                entity.last_fire_time = _game_time;
            }
        }
    }
}

void GameServer::spawnProjectile(uint32_t owner_net_id, bool is_enemy)
{
    // NOTE: Caller must already hold _mutex lock
    auto owner_it = _entities.find(owner_net_id);
    if (owner_it == _entities.end()) {
        return;
    }

    uint32_t net_id = generateNetId();

    ServerEntity projectile;
    projectile.net_id = net_id;
    projectile.type = is_enemy ? EntityType::PROJECTILE : EntityType::ALLIED_PROJECTILE;
    projectile.pos_x = owner_it->second.pos_x + (is_enemy ? -0.05f : 0.05f); // Spawn in front
    projectile.pos_y = owner_it->second.pos_y;
    projectile.vel_x = is_enemy ? -0.008f : 0.008f; // Fast movement
    projectile.vel_y = 0.0f;
    projectile.health = 1;
    projectile.owner_player_id = owner_it->second.owner_player_id;
    projectile.width = 0.02f;
    projectile.height = 0.01f;
    projectile.last_fire_time = 0.0f;

    _entities[net_id] = projectile;

    // Broadcast entity creation to all clients
    for (const auto& [client_id, endpoint] : _client_endpoints) {
        sendEntityCreate(projectile, endpoint);
    }
}

void GameServer::spawnMultipleProjectiles(uint32_t owner_net_id, bool is_enemy, int count, float angle_spread)
{
    // NOTE: Caller must already hold _mutex lock
    auto owner_it = _entities.find(owner_net_id);
    if (owner_it == _entities.end()) {
        return;
    }

    // Calculate angles for each projectile (EXACTLY like solo mode)
    const float PI = 3.14159265359f;
    float base_angle = 0.0f;

    if (count > 1) {
        base_angle = -angle_spread * (count - 1) / 2.0f;
    }

    for (int i = 0; i < count; ++i) {
        uint32_t net_id = generateNetId();

        ServerEntity projectile;
        projectile.net_id = net_id;
        projectile.type = is_enemy ? EntityType::PROJECTILE : EntityType::ALLIED_PROJECTILE;
        projectile.pos_x = owner_it->second.pos_x + (is_enemy ? -0.05f : 0.05f);
        projectile.pos_y = owner_it->second.pos_y;

        // Calculate angle for this projectile (in degrees)
        float angle_deg = base_angle + (angle_spread * i);
        float angle_rad = angle_deg * PI / 180.0f;

        // Calculate velocity components (base speed 0.008)
        float base_speed = 0.008f;
        float speed_x = base_speed * std::cos(angle_rad);
        float speed_y = base_speed * std::sin(angle_rad);

        // Force X direction based on team (like solo mode does with abs())
        if (is_enemy) {
            projectile.vel_x = -std::abs(speed_x);  // Enemy projectiles go left
        } else {
            projectile.vel_x = std::abs(speed_x);   // Player projectiles go right
        }
        projectile.vel_y = speed_y;
        projectile.health = 1;
        projectile.owner_player_id = owner_it->second.owner_player_id;
        projectile.width = 0.02f;
        projectile.height = 0.01f;
        projectile.last_fire_time = 0.0f;

        _entities[net_id] = projectile;

        // Broadcast entity creation to all clients
        for (const auto& [client_id, endpoint] : _client_endpoints) {
            sendEntityCreate(projectile, endpoint);
        }
    }
}

void GameServer::updateProjectiles(float /*dt*/)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& [net_id, entity] : _entities) {
        if (entity.type == EntityType::PROJECTILE || entity.type == EntityType::ALLIED_PROJECTILE) {
            // Move projectile
            entity.pos_x += entity.vel_x;

            // Remove projectiles that went off-screen
            if (entity.pos_x < -0.1f || entity.pos_x > 1.1f) {
                _entities_to_destroy.push_back(net_id);
            }
        }
    }
}

bool GameServer::checkCollision(const ServerEntity& a, const ServerEntity& b)
{
    return (a.pos_x < b.pos_x + b.width &&
            a.pos_x + a.width > b.pos_x &&
            a.pos_y < b.pos_y + b.height &&
            a.pos_y + a.height > b.pos_y);
}

void GameServer::updateCollisions()
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::vector<uint32_t> entities_to_check;
    for (const auto& [net_id, entity] : _entities) {
        entities_to_check.push_back(net_id);
    }

    // Check all entity pairs for collisions
    for (size_t i = 0; i < entities_to_check.size(); ++i) {
        for (size_t j = i + 1; j < entities_to_check.size(); ++j) {
            uint32_t id_a = entities_to_check[i];
            uint32_t id_b = entities_to_check[j];

            auto it_a = _entities.find(id_a);
            auto it_b = _entities.find(id_b);

            if (it_a == _entities.end() || it_b == _entities.end()) {
                continue;
            }

            ServerEntity& a = it_a->second;
            ServerEntity& b = it_b->second;

            if (!checkCollision(a, b)) {
                continue;
            }

            // Handle different collision types
            // Allied projectile vs Enemy (any type)
            if (a.type == EntityType::ALLIED_PROJECTILE &&
                (b.type == EntityType::ENEMY || b.type == EntityType::ENEMY_SPREAD || b.type == EntityType::BOSS)) {
                damageEntity(id_b, 10);  // 10 damage
                _entities_to_destroy.push_back(id_a);  // Destroy projectile
            }
            else if (b.type == EntityType::ALLIED_PROJECTILE &&
                     (a.type == EntityType::ENEMY || a.type == EntityType::ENEMY_SPREAD || a.type == EntityType::BOSS)) {
                damageEntity(id_a, 10);
                _entities_to_destroy.push_back(id_b);
            }
            // Enemy projectile vs Player
            else if (a.type == EntityType::PROJECTILE && b.type == EntityType::PLAYER) {
                damageEntity(id_b, 20);  // 20 damage to player
                _entities_to_destroy.push_back(id_a);
            }
            else if (b.type == EntityType::PROJECTILE && a.type == EntityType::PLAYER) {
                damageEntity(id_a, 20);
                _entities_to_destroy.push_back(id_b);
            }
            // Enemy vs Player (collision damage)
            else if ((a.type == EntityType::ENEMY || a.type == EntityType::ENEMY_SPREAD || a.type == EntityType::BOSS) &&
                     b.type == EntityType::PLAYER) {
                damageEntity(id_b, 30);  // Big damage
                damageEntity(id_a, 30);  // Enemy also takes damage
            }
            else if ((b.type == EntityType::ENEMY || b.type == EntityType::ENEMY_SPREAD || b.type == EntityType::BOSS) &&
                     a.type == EntityType::PLAYER) {
                damageEntity(id_a, 30);
                damageEntity(id_b, 30);
            }
        }
    }
}

void GameServer::damageEntity(uint32_t net_id, uint32_t damage)
{
    auto it = _entities.find(net_id);
    if (it == _entities.end()) {
        return;
    }

    uint32_t old_health = it->second.health;
    std::string entity_type_name;
    switch (it->second.type) {
        case EntityType::PLAYER: entity_type_name = "PLAYER"; break;
        case EntityType::ENEMY: entity_type_name = "ENEMY"; break;
        case EntityType::ENEMY_SPREAD: entity_type_name = "ENEMY_SPREAD"; break;
        case EntityType::BOSS: entity_type_name = "BOSS"; break;
        case EntityType::PROJECTILE: entity_type_name = "ENEMY_PROJECTILE"; break;
        case EntityType::ALLIED_PROJECTILE: entity_type_name = "ALLIED_PROJECTILE"; break;
        default: entity_type_name = "UNKNOWN"; break;
    }

    if (it->second.health <= damage) {
        it->second.health = 0;
        _entities_to_destroy.push_back(net_id);

        std::cout << "[DAMAGE] " << entity_type_name << " NET_ID=" << net_id
                  << " took " << damage << " damage (" << old_health << " -> 0) - DESTROYED!" << std::endl;

        // Add score if enemy died
        if (it->second.type == EntityType::ENEMY) {
            _game_score += 10;
        } else if (it->second.type == EntityType::ENEMY_SPREAD) {
            _game_score += 10;  // Same as regular enemy
        } else if (it->second.type == EntityType::BOSS) {
            _game_score += 1000;  // Boss worth much more
            _boss_spawned = false;  // Allow boss to respawn in future
        }
    } else {
        it->second.health -= damage;
        std::cout << "[DAMAGE] " << entity_type_name << " NET_ID=" << net_id
                  << " took " << damage << " damage (" << old_health << " -> " << it->second.health << ")" << std::endl;
    }
}

void GameServer::cleanupDeadEntities()
{
    if (_entities_to_destroy.empty()) {
        return;
    }

    std::vector<uint32_t> destroyed_ids;
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (uint32_t net_id : _entities_to_destroy) {
            auto it = _entities.find(net_id);
            if (it != _entities.end()) {
                _entities.erase(it);
                destroyed_ids.push_back(net_id);
            }
        }

        _entities_to_destroy.clear();
    }

    // Send destroy messages AFTER releasing the lock
    for (uint32_t net_id : destroyed_ids) {
        sendEntityDestroy(net_id);
    }
}

void GameServer::sendEntityDestroy(uint32_t net_id)
{
    auto packet = serializeEntityDestroy(net_id);
    broadcastUDPMessage(packet);
}

