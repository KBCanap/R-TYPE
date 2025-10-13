/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** Fixed GameServerLoop Implementation
*/

#include "GameServerLoop.hpp"
#include "GameLogic.hpp"
#include <iostream>
#include <chrono>
#include <memory>

GameServerLoop* GameServerLoop::instance = nullptr;

GameServerLoop::GameServerLoop(uint16_t port, uint32_t max_clients)
    : _port(port),
      _max_clients(max_clients),
      _in_game(false),
      _sequence_num(0),
      _running(false),
      _udp_server(nullptr),
      _loop_thread(nullptr),
      _protocol()
{
    instance = this;
    setupSignalHandlers();
    std::cout << "GameServerLoop initialized on port " << _port
              << " (max clients: " << _max_clients << ")" << std::endl;
}

GameServerLoop::~GameServerLoop()
{
    stop();
    instance = nullptr;
}

void GameServerLoop::setupSignalHandlers()
{
    std::signal(SIGINT, GameServerLoop::signalHandler);
    std::signal(SIGTERM, GameServerLoop::signalHandler);
}

void GameServerLoop::signalHandler(int signal)
{
    std::cout << "\n[SIGNAL] Received signal " << signal << std::endl;
    if (instance) {
        std::cout << "[SIGNAL] Stopping server gracefully..." << std::endl;
        instance->stop();
    }
    std::exit(0);
}

void GameServerLoop::start()
{
    if (_running) {
        std::cout << "GameServerLoop is already running!" << std::endl;
        return;
    }

    try {
        _udp_server = std::make_unique<UDPServer>(_port, _max_clients);
        std::cout << "UDP Server started on port " << _port << std::endl;
        _running = true;
        _loop_thread = std::make_unique<std::thread>(&GameServerLoop::run, this);
        std::cout << "GameServerLoop started successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start GameServerLoop: " << e.what() << std::endl;
        _running = false;
    }
}

void GameServerLoop::stop()
{
    if (!_running) {
        return;
    }

    std::cout << "Stopping GameServerLoop..." << std::endl;
    _running = false;

    if (_loop_thread && _loop_thread->joinable()) {
        _loop_thread->join();
    }

    _udp_server.reset();
    _game_logic.reset();
    std::cout << "GameServerLoop stopped" << std::endl;
}

void GameServerLoop::run()
{
    std::cout << "Game loop started" << std::endl;

    _game_logic = std::make_unique<GameLogic>(std::make_shared<registry>());
    _last_tick = std::chrono::steady_clock::now();

    while (_running) {
        if (!_in_game && _udp_server->getCurrentClientCount() == _max_clients) {
            _in_game = true;
            _game_logic->start();
            std::cout << "[GameServerLoop] All " << _max_clients
                      << " clients connected, game started!" << std::endl;
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
        } else {
            processMessages();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "Game loop ended" << std::endl;
}

void GameServerLoop::processMessages()
{
    if (!_udp_server || !_game_logic) {
        return;
    }

    auto messages = _udp_server->poll();

    for (const auto& msg : messages) {
        ParsedUdpMessage parsed = _protocol.parseMessage(msg.message);

        if (!parsed.valid) {
            std::cerr << "Invalid UDP message from client " << msg.client_id << std::endl;
            continue;
        }

        if (parsed.type == CLIENT_PING) {
            if (_game_logic->getPlayerEntity(msg.client_id) == entity(static_cast<size_t>(-1))) {
                uint net_id = _game_logic->generateNetId();

                // Use normalized coordinates (0.0-1.0)
                float spawn_x = 0.1f + (0.15f * (msg.client_id % 4));
                float spawn_y = 0.4f + (0.1f * (msg.client_id % 4));

                _game_logic->createPlayer(msg.client_id, net_id, spawn_x, spawn_y);

                // Send PLAYER_ASSIGNMENT
                std::string assign_msg = _protocol.createPlayerAssignment(net_id, _sequence_num++);
                _udp_server->sendToClient(msg.client_id, assign_msg);

                std::cout << "[GameServerLoop] Player " << msg.client_id
                        << " assigned NET_ID " << net_id << std::endl;

                auto snapshot = _game_logic->generateSnapshot();
                std::vector<Entity> entities;

                for (const auto& snap : snapshot.entities) {
                    EntityType type = EntityType::PLAYER;

                    if (snap.entity_type == "player") {
                        type = EntityType::PLAYER;
                    } else if (snap.entity_type == "enemy") {
                        type = EntityType::ENEMY;
                    } else if (snap.entity_type == "projectile") {
                        type = EntityType::PROJECTILE;
                    }

                    entities.push_back({
                        snap.net_id,
                        type,
                        static_cast<uint32_t>(snap.health),
                        snap.pos.x,
                        snap.pos.y
                    });
                }

                std::string game_state_msg = _protocol.createGameState(entities, _sequence_num++);
                _udp_server->sendToClient(msg.client_id, game_state_msg);

                std::cout << "[GameServerLoop] Sent GAME_STATE with " << entities.size()
                        << " entities to client " << msg.client_id << std::endl;
            }
        }
        else if (parsed.type == PLAYER_INPUT && parsed.data.size() >= 2) {
            uint8_t event_type = parsed.data[0];
            uint8_t direction = parsed.data[1];

            if (event_type == 0x01) {
                _game_logic->pushClientEvent({
                    msg.client_id,
                    KEY_UP_RELEASE,
                    parsed.sequence_num,
                    std::chrono::steady_clock::now()
                });
                _game_logic->pushClientEvent({
                    msg.client_id,
                    KEY_DOWN_RELEASE,
                    parsed.sequence_num,
                    std::chrono::steady_clock::now()
                });
                _game_logic->pushClientEvent({
                    msg.client_id,
                    KEY_LEFT_RELEASE,
                    parsed.sequence_num,
                    std::chrono::steady_clock::now()
                });
                _game_logic->pushClientEvent({
                    msg.client_id,
                    KEY_RIGHT_RELEASE,
                    parsed.sequence_num,
                    std::chrono::steady_clock::now()
                });

                if (direction & 0x01) {
                    _game_logic->pushClientEvent({
                        msg.client_id,
                        KEY_UP_PRESS,
                        parsed.sequence_num,
                        std::chrono::steady_clock::now()
                    });
                }
                if (direction & 0x02) {
                    _game_logic->pushClientEvent({
                        msg.client_id,
                        KEY_DOWN_PRESS,
                        parsed.sequence_num,
                        std::chrono::steady_clock::now()
                    });
                }
                if (direction & 0x04) {
                    _game_logic->pushClientEvent({
                        msg.client_id,
                        KEY_LEFT_PRESS,
                        parsed.sequence_num,
                        std::chrono::steady_clock::now()
                    });
                }
                if (direction & 0x08) {
                    _game_logic->pushClientEvent({
                        msg.client_id,
                        KEY_RIGHT_PRESS,
                        parsed.sequence_num,
                        std::chrono::steady_clock::now()
                    });
                }
            }
            else if (event_type == 0x02) {
                _game_logic->pushClientEvent({
                    msg.client_id,
                    KEY_SHOOT_PRESS,
                    parsed.sequence_num,
                    std::chrono::steady_clock::now()
                });
            }
            else if (event_type == 0x03) {
                _game_logic->removePlayer(msg.client_id);
                _udp_server->disconnectClient(msg.client_id);
                std::cout << "[GameServerLoop] Player " << msg.client_id << " quit" << std::endl;
            }
        }
    }
}

void GameServerLoop::broadcastEntityUpdates()
{
    if (!_in_game || !_game_logic || !_udp_server) {
        return;
    }

    auto clients = _udp_server->getConnectedClients();
    if (clients.empty()) {
        return;
    }

    auto deltas = _game_logic->getDeltaSnapshot(0);

    if (deltas.empty()) {
        return;
    }

    std::vector<Entity> entities;
    for (const auto& snap : deltas) {
        EntityType type = EntityType::ENEMY;

        if (snap.entity_type == "player") {
            type = EntityType::PLAYER;
        } else if (snap.entity_type == "enemy") {
            type = EntityType::ENEMY;
        } else if (snap.entity_type == "projectile") {
            type = EntityType::PROJECTILE;
        } else if (snap.entity_type == "boss") {
            type = EntityType::ENEMY;
        }

        entities.push_back({
            snap.net_id,
            type,
            static_cast<uint32_t>(snap.health),
            snap.pos.x,
            snap.pos.y
        });
    }

    std::string update_msg = _protocol.createEntityUpdate(entities, _sequence_num++);

    for (uint32_t client_id : clients) {
        _udp_server->sendToClient(client_id, update_msg);
    }

    _game_logic->markEntitiesSynced();
}