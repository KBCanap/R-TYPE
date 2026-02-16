#include "GameServerLoop.hpp"
#include "GameLogic.hpp"
#include "UdpMessageType.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>

GameServerLoop *GameServerLoop::instance = nullptr;

GameServerLoop::GameServerLoop(uint16_t port, uint32_t max_clients, uint8_t level_id)
    : _port(port), _max_clients(max_clients), _level_id(level_id), _in_game(false),
      _victory_sent(false), _sequence_num(0), _running(false), _udp_server(nullptr),
      _loop_thread(nullptr), _protocol() {
    instance = this;
    setupSignalHandlers();
}

GameServerLoop::~GameServerLoop() {
    stop();
    instance = nullptr;
}

void GameServerLoop::setupSignalHandlers() {
    std::signal(SIGINT, GameServerLoop::signalHandler);
    std::signal(SIGTERM, GameServerLoop::signalHandler);
}

void GameServerLoop::signalHandler(int signal) {
    if (instance) {
        instance->stop();
    }
    std::exit(0);
}

void GameServerLoop::start() {
    if (_running) {
        return;
    }

    try {
        _udp_server = std::make_unique<UDPServer>(_port, _max_clients);
        _running = true;
        _loop_thread = std::make_unique<std::thread>(&GameServerLoop::run, this);
    } catch (const std::exception &e) {
        std::cerr << "Failed to start GameServerLoop: " << e.what() << std::endl;
        _running = false;
    }
}

void GameServerLoop::stop() {
    if (!_running) {
        return;
    }

    _running = false;

    if (_loop_thread && _loop_thread->joinable()) {
        _loop_thread->join();
    }

    _udp_server.reset();
    _game_logic.reset();
}

void GameServerLoop::run() {
    _game_logic = std::make_unique<GameLogic>(std::make_shared<registry>(), _level_id);
    _last_tick = std::chrono::steady_clock::now();

    while (_running) {
        if (!_in_game && _udp_server->getCurrentClientCount() == _max_clients) {
            _in_game = true;
            _game_logic->start();
            _last_tick = std::chrono::steady_clock::now();
        }

        if (_in_game) {
            processMessages();

            auto now = std::chrono::steady_clock::now();
            float delta = std::chrono::duration<float>(now - _last_tick).count();

            if (delta > 0.1f) {
                delta = 0.1f;
            }

            _game_logic->update(delta);
            _last_tick = now;

            broadcastEntityUpdates();

            if (_game_logic->isLevelComplete() && !_victory_sent) {
                std::string victory_msg = _protocol.createVictory(_sequence_num++);
                auto clients = _udp_server->getConnectedClients();
                for (uint32_t client_id : clients) {
                    _udp_server->sendToClient(client_id, victory_msg);
                }
                _victory_sent = true;
            }
        } else {
            processMessages();

            auto elapsed = std::chrono::steady_clock::now() - _last_tick;
            if (std::chrono::duration<float>(elapsed).count() > 30.0f) {
                _running = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GameServerLoop::processMessages() {
    if (!_udp_server || !_game_logic) {
        return;
    }

    auto messages = _udp_server->poll();

    for (const auto &msg : messages) {
        ParsedUdpMessage parsed = _protocol.parseMessage(msg.message);

        if (!parsed.valid) {
            std::cerr << "Invalid UDP message from client " << msg.client_id << std::endl;
            continue;
        }

        if (parsed.type == CLIENT_PING) {
            if (_game_logic->getPlayerEntity(msg.client_id) == entity(static_cast<size_t>(-1))) {
                uint net_id = _game_logic->generateNetId();

                float spawn_x = 0.1f + (0.15f * (msg.client_id % 4));
                float spawn_y = 0.4f + (0.1f * (msg.client_id % 4));

                _game_logic->createPlayer(msg.client_id, net_id, spawn_x, spawn_y);

                std::string assign_msg = _protocol.createPlayerAssignment(net_id, _sequence_num++);
                _udp_server->sendToClient(msg.client_id, assign_msg);

                auto snapshot = _game_logic->generateSnapshot();
                std::vector<Entity> entities;

                for (const auto &snap : snapshot.entities) {
                    EntityType type = EntityType::PLAYER;

                    //change if/else order to gain performance
                    if (snap.entity_type == "allied_projectile") type = EntityType::ALLIED_PROJECTILE;
                    else if (snap.entity_type == "projectile") type = EntityType::PROJECTILE;
                    // Enemies
                    else if (snap.entity_type == "enemy") type = EntityType::ENEMY;
                    else if (snap.entity_type == "enemy_level2") type = EntityType::ENEMY_LEVEL2;
                    else if (snap.entity_type == "enemy_level2_spread") type = EntityType::ENEMY_LEVEL2_SPREAD;

                    // Players
                    else if (snap.entity_type == "player") type = EntityType::PLAYER;

                    // Bosses
                    else if (snap.entity_type == "boss") type = EntityType::BOSS;
                    else if (snap.entity_type == "boss_level2_part1") type = EntityType::BOSS_LEVEL2_PART1;
                    else if (snap.entity_type == "boss_level2_part2") type = EntityType::BOSS_LEVEL2_PART2;
                    else if (snap.entity_type == "boss_level2_part3") type = EntityType::BOSS_LEVEL2_PART3;

                    // Powerups
                    else if (snap.entity_type == "powerup_shield") type = EntityType::POWERUP_SHIELD;
                    else if (snap.entity_type == "powerup_spread") type = EntityType::POWERUP_SPREAD;

                    entities.push_back({snap.net_id, type,
                                        static_cast<uint32_t>(snap.health),
                                        static_cast<uint32_t>(snap.shield),
                                        snap.pos.x, snap.pos.y,
                                        static_cast<uint32_t>(snap.score)});
                }

                std::string game_state_msg = _protocol.createGameState(entities, _sequence_num++);
                _udp_server->sendToClient(msg.client_id, game_state_msg);

                Entity new_player_entity = {net_id, EntityType::PLAYER, 100, 0, spawn_x, spawn_y, 0};
                std::string create_msg = _protocol.createEntityCreate(new_player_entity, _sequence_num++);

                auto all_clients = _udp_server->getConnectedClients();
                for (uint32_t client_id : all_clients) {
                    if (client_id != msg.client_id) {
                        _udp_server->sendToClient(client_id, create_msg);
                    }
                }
            }
        } else if (parsed.type == PLAYER_INPUT && parsed.data.size() >= 2) {
            uint8_t event_type = parsed.data[0];
            uint8_t direction = parsed.data[1];

            if (event_type == 0x01) {
                _game_logic->pushClientEvent({msg.client_id, KEY_UP_RELEASE, parsed.sequence_num, std::chrono::steady_clock::now()});
                _game_logic->pushClientEvent({msg.client_id, KEY_DOWN_RELEASE, parsed.sequence_num, std::chrono::steady_clock::now()});
                _game_logic->pushClientEvent({msg.client_id, KEY_LEFT_RELEASE, parsed.sequence_num, std::chrono::steady_clock::now()});
                _game_logic->pushClientEvent({msg.client_id, KEY_RIGHT_RELEASE, parsed.sequence_num, std::chrono::steady_clock::now()});

                if (direction & 0x01) {
                    _game_logic->pushClientEvent({msg.client_id, KEY_UP_PRESS, parsed.sequence_num, std::chrono::steady_clock::now()});
                }
                if (direction & 0x02) {
                    _game_logic->pushClientEvent({msg.client_id, KEY_DOWN_PRESS, parsed.sequence_num, std::chrono::steady_clock::now()});
                }
                if (direction & 0x04) {
                    _game_logic->pushClientEvent({msg.client_id, KEY_LEFT_PRESS, parsed.sequence_num, std::chrono::steady_clock::now()});
                }
                if (direction & 0x08) {
                    _game_logic->pushClientEvent({msg.client_id, KEY_RIGHT_PRESS, parsed.sequence_num, std::chrono::steady_clock::now()});
                }
            } else if (event_type == 0x02) {
                _game_logic->pushClientEvent({msg.client_id, KEY_SHOOT_PRESS, parsed.sequence_num, std::chrono::steady_clock::now()});
            } else if (event_type == 0x03) {
                _game_logic->removePlayer(msg.client_id);
                _udp_server->disconnectClient(msg.client_id);
            }
        }
    }
}

void GameServerLoop::broadcastEntityUpdates() {
    if (!_in_game || !_game_logic || !_udp_server) {
        return;
    }

    auto clients = _udp_server->getConnectedClients();
    if (clients.empty()) {
        return;
    }

    auto new_entities = _game_logic->getNewEntities();
    std::set<uint> new_entity_ids;

    for (const auto &new_ent : new_entities) {
        new_entity_ids.insert(new_ent.net_id);

        //change if/else order to gain performance
        EntityType type = EntityType::ENEMY;
        if (new_ent.entity_type == "allied_projectile") type = EntityType::ALLIED_PROJECTILE;
        else if (new_ent.entity_type == "projectile") type = EntityType::PROJECTILE;
        // Enemies
        else if (new_ent.entity_type == "enemy") type = EntityType::ENEMY;
        else if (new_ent.entity_type == "enemy_level2") type = EntityType::ENEMY_LEVEL2;
        else if (new_ent.entity_type == "enemy_level2_spread") type = EntityType::ENEMY_LEVEL2_SPREAD;

        // Players
        else if (new_ent.entity_type == "player") type = EntityType::PLAYER;

        // Powerups
        else if (new_ent.entity_type == "powerup_shield") type = EntityType::POWERUP_SHIELD;
        else if (new_ent.entity_type == "powerup_spread") type = EntityType::POWERUP_SPREAD;

        // Bosses
        else if (new_ent.entity_type == "boss") type = EntityType::BOSS;
        else if (new_ent.entity_type == "boss_level2_part1") type = EntityType::BOSS_LEVEL2_PART1;
        else if (new_ent.entity_type == "boss_level2_part2") type = EntityType::BOSS_LEVEL2_PART2;
        else if (new_ent.entity_type == "boss_level2_part3") type = EntityType::BOSS_LEVEL2_PART3;



        Entity ent = {new_ent.net_id, type, static_cast<uint32_t>(new_ent.health),
                      static_cast<uint32_t>(new_ent.shield), new_ent.x, new_ent.y, 0};
        std::string create_msg = _protocol.createEntityCreate(ent, _sequence_num++);

        for (uint32_t client_id : clients) {
            _udp_server->sendToClient(client_id, create_msg);
        }
    }

    auto deltas = _game_logic->getDeltaSnapshot(0);

    std::vector<Entity> entities;
    for (const auto &snap : deltas) {
        if (new_entity_ids.count(snap.net_id) > 0) {
            continue;
        }

        EntityType type = EntityType::ENEMY;

        if (snap.entity_type == "player") {
            type = EntityType::PLAYER;
        } else if (snap.entity_type == "enemy") {
            type = EntityType::ENEMY;
        } else if (snap.entity_type == "enemy_level2") {
            type = EntityType::ENEMY_LEVEL2;
        } else if (snap.entity_type == "enemy_level2_spread") {
            type = EntityType::ENEMY_LEVEL2_SPREAD;
        } else if (snap.entity_type == "projectile") {
            type = EntityType::PROJECTILE;
        } else if (snap.entity_type == "allied_projectile") {
            type = EntityType::ALLIED_PROJECTILE;
        } else if (snap.entity_type == "boss") {
            type = EntityType::BOSS;
        } else if (snap.entity_type == "boss_level2_part1") {
            type = EntityType::BOSS_LEVEL2_PART1;
        } else if (snap.entity_type == "boss_level2_part2") {
            type = EntityType::BOSS_LEVEL2_PART2;
        } else if (snap.entity_type == "boss_level2_part3") {
            type = EntityType::BOSS_LEVEL2_PART3;
        } else if (snap.entity_type == "powerup_shield") {
            type = EntityType::POWERUP_SHIELD;
        } else if (snap.entity_type == "powerup_spread") {
            type = EntityType::POWERUP_SPREAD;
        }

        entities.push_back({snap.net_id, type,
                            static_cast<uint32_t>(snap.health),
                            static_cast<uint32_t>(snap.shield), snap.pos.x,
                            snap.pos.y, static_cast<uint32_t>(snap.score)});
    }

    if (!entities.empty()) {
        std::string update_msg = _protocol.createEntityUpdate(entities, _sequence_num++);

        for (uint32_t client_id : clients) {
            _udp_server->sendToClient(client_id, update_msg);
        }
    }

    _game_logic->markEntitiesSynced();

    auto destroyed = _game_logic->getDestroyedEntities();
    if (!destroyed.empty()) {
        std::string destroy_msg = _protocol.createEntityDestroy(destroyed, _sequence_num++);
        for (uint32_t client_id : clients) {
            _udp_server->sendToClient(client_id, destroy_msg);
        }
    }
}
