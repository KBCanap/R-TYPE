/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_game_core - Core game loop and initialization
*/

#include "mario_game.hpp"
#include "systems.hpp"
#include <chrono>
#include <cmath>
#include <iostream>

MarioGame::MarioGame(registry &reg, render::IRenderWindow &win,
                     AudioManager &audioMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr) {
    _registry.register_component<component::position>();
    _registry.register_component<component::velocity>();
    _registry.register_component<component::drawable>();
    _registry.register_component<component::hitbox>();
    _registry.register_component<component::gravity>();
    _registry.register_component<component::platform_tag>();
    _registry.register_component<component::input>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::background>();
    _registry.register_component<component::animation>();
    _registry.register_component<component::dead>();
    _registry.register_component<component::enemy_stunned>();
    _registry.register_component<component::ai_input>();
    _registry.register_component<component::pow_block>();

    loadSounds();
    playSound("game_start");

    _background = _registry.spawn_entity();
    auto &bg_component = _registry.add_component<component::background>(
        *_background, component::background(0.0f));
    bg_component->texture = _window.createTexture();
    if (!bg_component->texture->loadFromFile(
            "assets/mario/sprites/background_mario.png")) {
        std::cerr << "[MarioGame] Failed to load Mario background" << std::endl;
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(100, 150, 255));
        bg_component->texture->loadFromImage(*fallback_image);
    }
    _registry.add_component<component::position>(*_background,
                                                  component::position(0.0f, 0.0f));

    _debugFont = _window.createFont();
    if (!_debugFont->loadFromFile("assets/r-type.otf")) {
        std::cerr << "[MarioGame] Failed to load font" << std::endl;
    }

    createPlatforms();

    render::Vector2u window_size = _window.getSize();
    float scale_x = static_cast<float>(window_size.x) / 256.0f;
    float scale_y = static_cast<float>(window_size.y) / 224.0f;
    createPowBlock(scale_x, scale_y);

    createPlayer();
    setupPlayerAnimation();
    spawnEnemies();
}

void MarioGame::update(float dt) {
    _gameTime += dt;

    auto &positions = _registry.get_components<component::position>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &gravities = _registry.get_components<component::gravity>();
    auto &platforms = _registry.get_components<component::platform_tag>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &ai_inputs = _registry.get_components<component::ai_input>();
    auto &projectiles = _registry.get_components<component::projectile>();
    auto &stunneds = _registry.get_components<component::enemy_stunned>();
    auto &deads = _registry.get_components<component::dead>();

    updateEnemySpawning(dt);
    updateControls(dt);
    updatePlayerAnimation(dt);

    for (size_t i = 0; i < drawables.size(); ++i) {
        if (!drawables[i] || drawables[i]->tag != "enemy")
            continue;
        if (i >= velocities.size() || !velocities[i])
            continue;
        if (velocities[i]->vx > 1.0f) {
            drawables[i]->flip_x = true;
        } else if (velocities[i]->vx < -1.0f) {
            drawables[i]->flip_x = false;
        }
    }

    systems::ai_input_system(_registry, ai_inputs, dt);

    render::Vector2u window_size = _window.getSize();
    float screen_width = static_cast<float>(window_size.x);
    float screen_height = static_cast<float>(window_size.y);

    if (_player && *_player < positions.size() && positions[*_player]) {
        auto &player_pos = positions[*_player];
        float player_width = 0.0f;
        if (*_player < hitboxes.size() && hitboxes[*_player]) {
            player_width = hitboxes[*_player]->width;
        }
        if (player_pos->x > screen_width) {
            player_pos->x = -player_width;
        } else if (player_pos->x + player_width < 0) {
            player_pos->x = screen_width;
        }
    }

    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &drawable = drawables[i];
        auto &hitbox = hitboxes[i];
        auto &stunned = stunneds[i];

        if (!pos || !drawable || drawable->tag != "enemy")
            continue;
        if (stunned && stunned->stunned && std::abs(stunned->knockback_velocity) > 1.0f)
            continue;

        float entity_width = hitbox ? hitbox->width : 20.0f;
        if (pos->x > screen_width) {
            pos->x = -entity_width;
        } else if (pos->x + entity_width < 0) {
            pos->x = screen_width;
        }
        if (pos->y > screen_height) {
            _gameOver = true;
        }
    }

    systems::gravity_system(_registry, gravities, velocities, dt);

    auto &inputs = _registry.get_components<component::input>();
    systems::position_system(_registry, positions, velocities, inputs, _window,
                             _gameTime, dt);

    systems::platform_collision_system(_registry, positions, velocities,
                                        gravities, platforms, hitboxes);

    {
        float sw = screen_width;
        float sh = screen_height;
        float pipe_left_right = sw * (16.0f / 256.0f);
        float pipe_right_left = sw * (240.0f / 256.0f);
        float ground_y = sh * (207.0f / 224.0f);

        for (size_t i = 0; i < positions.size(); ++i) {
            if (!positions[i] || i >= drawables.size() || !drawables[i] ||
                drawables[i]->tag != "enemy")
                continue;
            if (i < stunneds.size() && stunneds[i] && stunneds[i]->stunned &&
                std::abs(stunneds[i]->knockback_velocity) > 1.0f)
                continue;

            float ey = positions[i]->y;
            float ex = positions[i]->x;
            float ew = (i < hitboxes.size() && hitboxes[i]) ? hitboxes[i]->width : 20.0f;
            float eh = (i < hitboxes.size() && hitboxes[i]) ? hitboxes[i]->height : 20.0f;
            float enemy_bottom = ey + eh;
            bool at_ground = enemy_bottom >= ground_y - 5.0f;

            if (at_ground && (ex <= pipe_left_right || ex + ew >= pipe_right_left)) {
                positions[i] = std::nullopt;
                drawables[i] = std::nullopt;
                _gameOver = true;
                break;
            }
        }
    }

    systems::collision_system(_registry, positions, drawables, projectiles, hitboxes);

    if (_player && *_player < deads.size() && deads[*_player]) {
        if (!_gameOverSoundPlayed) {
            playSound("player_down");
            _gameOver = true;
            playSound("game_over");
            _gameOverSoundPlayed = true;
        }
    }

    int stunnedCount = 0;
    for (size_t i = 0; i < stunneds.size(); ++i) {
        if (stunneds[i] && stunneds[i]->stunned &&
            std::abs(stunneds[i]->knockback_velocity) < 1.0f) {
            stunnedCount++;
        }
    }
    if (stunnedCount > _lastStunnedCount) {
        playSound("enemy_flipped");
    }
    _lastStunnedCount = stunnedCount;

    auto &pow_blocks = _registry.get_components<component::pow_block>();
    if (_powBlock && *_powBlock < pow_blocks.size() && pow_blocks[*_powBlock]) {
        int currentHits = pow_blocks[*_powBlock]->hits_remaining;
        if (currentHits < _lastPowHits && currentHits >= 0) {
            playSound("pow_hit");
        }
        _lastPowHits = currentHits;
    }

    int enemyCount = 0;
    for (size_t i = 0; i < drawables.size(); ++i) {
        if (drawables[i] && drawables[i]->tag == "enemy" &&
            i < positions.size() && positions[i]) {
            enemyCount++;
        }
    }
    if (enemyCount < _lastEnemyCount && _lastEnemyCount > 0) {
        playSound("enemy_defeated");
    }
    _lastEnemyCount = enemyCount;

    if (!_victory && _enemiesSpawned >= _totalEnemiesToSpawn) {
        bool any_enemy_alive = false;
        for (size_t i = 0; i < drawables.size(); ++i) {
            if (drawables[i] && drawables[i]->tag == "enemy" &&
                i < positions.size() && positions[i]) {
                any_enemy_alive = true;
                break;
            }
        }
        if (!any_enemy_alive) {
            _victory = true;
            playSound("victory");
        }
    }
}

void MarioGame::resetForNextLevel() {
    _level++;
    _totalEnemiesToSpawn = static_cast<int>(_totalEnemiesToSpawn * 1.5f);
    _enemyBaseSpeed *= 1.1f;
    _enemySpawnInterval *= 0.67f;
    _enemiesSpawned = 0;
    _enemySpawnTimer = 0.0f;
    _victory = false;
    _gameOver = false;
    _gameOverSoundPlayed = false;

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
    _lastPowHits = 3;
    _lastStunnedCount = 0;
    _lastEnemyCount = 0;

    if (_player && *_player < positions.size() && positions[*_player]) {
        render::Vector2u window_size = _window.getSize();
        float scale_x = static_cast<float>(window_size.x) / 256.0f;
        float scale_y = static_cast<float>(window_size.y) / 224.0f;
        positions[*_player]->x = 50.0f * scale_x;
        positions[*_player]->y = 100.0f * scale_y;
    }

    auto &deads = _registry.get_components<component::dead>();
    if (_player && *_player < deads.size() && deads[*_player]) {
        deads[*_player] = std::nullopt;
    }

    playSound("game_start");
    std::cout << "[MarioGame] Starting level " << _level << " with "
              << _totalEnemiesToSpawn << " enemies" << std::endl;
}

void MarioGame::run() {
    auto last_time = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running && _window.isOpen() && !_shouldExit) {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float dt = elapsed.count();
        last_time = current_time;

        if (dt > 0.1f)
            dt = 0.1f;

        handleEvents(running, dt);
        if (!_gameOver && !_victory)
            update(dt);
        render(dt);
    }
}

void MarioGame::loadSounds() {
    render::IRenderAudio &audio = _audioManager.getAudioSystem();

    std::vector<std::pair<std::string, std::string>> soundFiles = {
        {"game_start", "assets/mario/audio/06. Game Start (Phase 2 Onward).mp3"},
        {"walk", "assets/mario/audio/24. Mario - Luigi Skidding.mp3"},
        {"jump", "assets/mario/audio/23. Mario - Luigi Jump.mp3"},
        {"enemy_flipped", "assets/mario/audio/21. Enemy Flipped.mp3"},
        {"enemy_defeated", "assets/mario/audio/22. Enemy Defeated.mp3"},
        {"pow_hit", "assets/mario/audio/20. POW Hit.mp3"},
        {"enemy_spawn", "assets/mario/audio/16. Shellcreeper Enters.mp3"},
        {"player_down", "assets/mario/audio/03. Player Down.mp3"},
        {"game_over", "assets/mario/audio/11. Game Over.mp3"},
        {"victory", "assets/mario/audio/10. Perfect Score.mp3"},
    };

    for (const auto &snd : soundFiles) {
        auto *buffer = audio.createSoundBuffer();
        if (buffer->loadFromFile(snd.second)) {
            _soundBuffers[snd.first] = buffer;
            auto *sound = audio.createSound();
            sound->setBuffer(*buffer);
            _sounds[snd.first] = sound;
        }
    }
}

void MarioGame::playSound(const std::string &name) {
    auto it = _sounds.find(name);
    if (it != _sounds.end() && it->second) {
        it->second->play();
    }
}
