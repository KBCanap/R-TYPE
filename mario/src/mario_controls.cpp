/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_controls - Input handling and controls
*/

#include "mario_game.hpp"
#include "systems.hpp"
#include <iostream>

void MarioGame::handleEvents(bool &running, float /*dt*/) {
    render::Event event;
    while (_window.pollEvent(event)) {
        systems::update_key_state(event);

        if (event.type == render::EventType::Closed) {
            running = false;
            _shouldExit = true;
            return;
        }

        if (event.type == render::EventType::KeyPressed) {
            if (event.key.code == render::Key::Escape) {
                running = false;
                _shouldExit = true;
                return;
            }

            if (event.key.code == render::Key::Enter) {
                if (_gameOver) {
                    _level = 1;
                    _enemySpawnInterval = 4.0f;
                    _totalEnemiesToSpawn = 3;
                    _enemyBaseSpeed = 75.0f;
                    _enemiesSpawned = 0;
                    _enemySpawnTimer = 0.0f;
                    _victory = false;
                    _gameOver = false;
                    _gameOverSoundPlayed = false;
                    _lastStunnedCount = 0;
                    _lastPowHits = 3;
                    _lastEnemyCount = 0;

                    auto &positions = _registry.get_components<component::position>();
                    auto &drawables = _registry.get_components<component::drawable>();
                    for (size_t i = 0; i < drawables.size(); ++i) {
                        if (drawables[i] && drawables[i]->tag == "enemy") {
                            positions[i] = std::nullopt;
                            drawables[i] = std::nullopt;
                        }
                    }

                    auto &pow_blocks = _registry.get_components<component::pow_block>();
                    if (_powBlock && *_powBlock < pow_blocks.size() && pow_blocks[*_powBlock]) {
                        if (pow_blocks[*_powBlock]->hits_remaining <= 0) {
                            pow_blocks[*_powBlock]->hits_remaining = 3;
                            _registry.add_component<component::platform_tag>(
                                *_powBlock, component::platform_tag());
                        }
                    }

                    if (_player) {
                        auto &deads = _registry.get_components<component::dead>();
                        if (*_player < deads.size() && deads[*_player]) {
                            deads[*_player] = std::nullopt;
                        }
                        if (*_player < positions.size() && positions[*_player]) {
                            render::Vector2u window_size = _window.getSize();
                            float scale_x = static_cast<float>(window_size.x) / 256.0f;
                            float scale_y = static_cast<float>(window_size.y) / 224.0f;
                            positions[*_player]->x = 50.0f * scale_x;
                            positions[*_player]->y = 100.0f * scale_y;
                        }
                    }

                    playSound("game_start");
                } else if (_victory) {
                    resetForNextLevel();
                }
            }
        }
    }
}

void MarioGame::updateControls(float dt) {
    if (_gameOver || _victory)
        return;

    auto &inputs = _registry.get_components<component::input>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &controllables = _registry.get_components<component::controllable>();
    auto &gravities = _registry.get_components<component::gravity>();

    systems::input_system(_registry, inputs, _window, nullptr);

    bool isWalking = false;

    for (size_t i = 0; i < inputs.size() && i < velocities.size() &&
                       i < controllables.size(); ++i) {
        auto &input = inputs[i];
        auto &vel = velocities[i];
        auto &ctrl = controllables[i];

        if (!input || !vel || !ctrl)
            continue;

        vel->vx = 0.0f;
        if (input->left) {
            vel->vx = -ctrl->speed;
            if (_player && i == *_player)
                isWalking = true;
        } else if (input->right) {
            vel->vx = ctrl->speed;
            if (_player && i == *_player)
                isWalking = true;
        }
    }

    if (isWalking && _player) {
        auto &player_gravity = gravities[*_player];
        if (player_gravity && player_gravity->on_ground) {
            _walkSoundTimer += dt;
            if (_walkSoundTimer >= _walkSoundInterval) {
                playSound("walk");
                _walkSoundTimer = 0.0f;
            }
        }
    } else {
        _walkSoundTimer = 0.0f;
    }

    if (_player) {
        auto &player_input = inputs[*_player];
        auto &player_gravity = gravities[*_player];
        auto &player_vel = velocities[*_player];

        if (player_input && player_gravity && player_vel) {
            if (player_input->up && player_gravity->on_ground) {
                player_vel->vy = -player_gravity->jump_strength;
                player_gravity->on_ground = false;
                playSound("jump");
            }
        }
    }
}
