/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** game
*/

#include "../include/game.hpp"
#include "../include/accessibility_menu.hpp"
#include "../include/keybindings_menu.hpp"
#include "../include/network/NetworkManager.hpp"
#include "../include/options_menu.hpp"
#include "../include/settings.hpp"
#include "network/NetworkComponents.hpp"
#include "network/NetworkSystem.hpp"
#include "systems.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

Game::Game(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
           KeyBindings &keyBindings, network::NetworkManager *netMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr),
      _keyBindings(keyBindings), _networkManager(netMgr),
      _isMultiplayer(netMgr != nullptr), _playerManager(reg, win),
      _enemyManager(reg, win), _bossManager(reg, win),
      _powerupManager(reg, win), _gameOverMenu(win, audioMgr),
      _victoryMenu(win, audioMgr), _tickSystem(60.0) {

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (_isMultiplayer) {
        _registry.register_component<component::network_entity>();
        _registry.register_component<component::network_state>();

        systems::initialize_network_system(_networkManager);

        _networkCommandHandler = std::make_unique<NetworkCommandHandler>(
            _registry, _window, _playerManager, _enemyManager, _bossManager);

        systems::set_network_command_handler(_networkCommandHandler.get());
    }

    _background = _registry.spawn_entity();
    auto &bg_component = _registry.add_component<component::background>(
        *_background, component::background(30.f));
    bg_component->texture = _window.createTexture();
    if (!bg_component->texture->loadFromFile("assets/background.jpg")) {
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(20, 20, 80));
        bg_component->texture->loadFromImage(*fallback_image);
    }

    _scoreFont = _window.createFont();
    if (!_scoreFont->loadFromFile("assets/r-type.otf")) {
    }

    if (!_isMultiplayer) {
        _player = _playerManager.createPlayer(_playerRelativeX,
                                              _playerRelativeY, _playerSpeed);
    }
}

void Game::handleEvents(bool &running, float /*dt*/) {
    render::Event event;
    while (_window.pollEvent(event)) {
        systems::update_key_state(event);

        if (event.type == render::EventType::Closed) {
            running = false;
            _shouldExit = true;
            return;
        }

        if (event.type == render::EventType::Resized) {
            _playerManager.updatePlayerPosition(_player, _playerRelativeX,
                                                _playerRelativeY);
            _enemyManager.updateEnemyPositions(_enemies);
            _gameOverMenu.onWindowResize();
            _victoryMenu.onWindowResize();
        }

        if (_gameOver) {
            MenuAction action = _gameOverMenu.handleEvents(event);
            if (action == MenuAction::REPLAY) {
                if (_isMultiplayer) {
                    running = false;
                    _shouldExit = true;
                } else {
                    resetGame();
                }
            } else if (action == MenuAction::QUIT) {
                running = false;
                _shouldExit = true;
            }
        }

        if (_victory) {
            MenuAction action = _victoryMenu.handleEvents(event);
            if (action == MenuAction::REPLAY) {
                if (_isMultiplayer) {
                    running = false;
                    _shouldExit = true;
                } else {
                    // Advance to next level
                    _currentLevel++;
                    resetGame();
                }
            } else if (action == MenuAction::QUIT) {
                running = false;
                _shouldExit = true;
            }
        }

        if (event.type == render::EventType::KeyPressed) {
            if (event.key.code == render::Key::Escape) {
                running = false;
                _shouldExit = true;
                return;
            }

            if (!_gameOver && !_victory && !_isMultiplayer) {
                if (event.key.code == render::Key::Num1) {
                    _playerManager.changePlayerWeaponToSingle(_player);
                }
                if (event.key.code == render::Key::Num2) {
                    _playerManager.changePlayerWeaponToRapid(_player);
                }
                if (event.key.code == render::Key::Num3) {
                    _playerManager.changePlayerWeaponToBurst(_player);
                }
                if (event.key.code == render::Key::Num4) {
                    _playerManager.changePlayerWeaponToSpread(_player);
                }
                if (event.key.code == render::Key::P) {
                    OptionsMenu optionsMenu(_window, _audioManager,
                                            _keyBindings);
                    OptionsResult result = optionsMenu.run();
                    if (result == OptionsResult::Accessibility) {
                        AccessibilityMenu accessibilityMenu(_window,
                                                            _audioManager);
                        accessibilityMenu.run();
                    }
                    if (result == OptionsResult::KeyBindings) {
                        KeyBindingsMenu keyBindingsMenu(_window, _audioManager,
                                                        _keyBindings);
                        keyBindingsMenu.run();
                    }
                }
            }
        }
    }
}

void Game::update(float dt) {
    if (_isMultiplayer && _networkManager) {
        updateMultiplayer(dt);
        return;
    }

    _gameTime += dt;

    auto &positions = _registry.get_components<component::position>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &controllables = _registry.get_components<component::controllable>();
    auto &projectiles = _registry.get_components<component::projectile>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &inputs = _registry.get_components<component::input>();

    systems::input_system(_registry, inputs, _window, &_keyBindings);

    auto &ai_inputs = _registry.get_components<component::ai_input>();
    systems::ai_input_system(_registry, ai_inputs, dt);

    if (!_gameOver && !_victory) {
        systems::control_system(_registry, controllables, velocities, inputs,
                                dt);
    }

    if (!_gameOver && !_victory) {
        auto &weapons = _registry.get_components<component::weapon>();
        systems::weapon_system(_registry, weapons, positions, inputs, ai_inputs,
                               _gameTime);
    }

    systems::position_system(_registry, positions, velocities, inputs, _window,
                             _gameTime, dt);

    systems::projectile_system(_registry, projectiles, positions, _window, dt);

    if (_player && !_gameOver && !_victory) {
        auto &player_pos = positions[*_player];
        if (player_pos) {
            render::Vector2u window_size = _window.getSize();

            player_pos->x = std::max(
                0.f, std::min(player_pos->x,
                              static_cast<float>(window_size.x) - 40.f));
            player_pos->y = std::max(
                0.f, std::min(player_pos->y,
                              static_cast<float>(window_size.y) - 40.f));

            _playerRelativeX =
                player_pos->x / static_cast<float>(window_size.x);
            _playerRelativeY =
                player_pos->y / static_cast<float>(window_size.y);
        }
    }

    if (!_gameOver && !_victory) {
        auto &hitboxes = _registry.get_components<component::hitbox>();
        systems::collision_system(_registry, positions, drawables, projectiles,
                                  hitboxes);
    }

    auto &healths = _registry.get_components<component::health>();
    systems::health_system(_registry, healths, dt);

    // Update shield visual
    if (_player) {
        _playerManager.updateShieldVisual(_player, _playerShield);
    }

    // For level 1 boss: check if it's still alive
    if (!_gameOver && !_victory && _boss && _currentLevel == 1) {
        auto &boss_pos = positions[*_boss];
        auto &boss_drawable = drawables[*_boss];

        if (!boss_pos || !boss_drawable) {
            _victory = true;
            _victoryMenu.setButtonText("Next Stage");
            _victoryMenu.setVisible(true);
            _boss.reset();
        }
    }

    // Check if all boss parts are destroyed (phase 2 victory)
    if (!_gameOver && !_victory && _bossPhase2 && !_bossParts.empty()) {
        bool all_parts_dead = true;
        int alive_parts = 0;
        std::optional<entity> last_part;

        for (auto it = _bossParts.begin(); it != _bossParts.end();) {
            auto &part_pos = positions[*it];
            if (part_pos) {
                all_parts_dead = false;
                alive_parts++;
                last_part = *it;
                ++it;
            } else {
                it = _bossParts.erase(it);
            }
        }

        // If only one part remains (part3), give it a spread weapon
        if (alive_parts == 1 && last_part) {
            auto &weapons = _registry.get_components<component::weapon>();
            if (*last_part < weapons.size() && !weapons[*last_part]) {
                // Add spread weapon to the last part
                _registry.add_component<component::weapon>(
                    *last_part, component::weapon(0.8f, false, 5, 30.0f,
                                    component::projectile_pattern::spread(30.0f),
                                    20.0f, 300.0f, 5.0f, false, 1, false, 1, 0.1f,
                                    render::IntRect(249, 103, 16, 12)));

                // Enable firing on AI
                auto &ai_inputs = _registry.get_components<component::ai_input>();
                if (*last_part < ai_inputs.size() && ai_inputs[*last_part]) {
                    ai_inputs[*last_part]->fire = true;
                    ai_inputs[*last_part]->fire_interval = 0.8f;
                }
            }
        }

        if (all_parts_dead) {
            _victory = true;
            _victoryMenu.setButtonText("Next Stage");
            _victoryMenu.setVisible(true);
            _bossPhase2 = false;
        }
    }

    if (!_gameOver && !_victory) {
        auto &scores = _registry.get_components<component::score>();
        systems::score_system(_registry, scores, dt);
    }

    auto &sound_effects = _registry.get_components<component::sound_effect>();
    auto &musics = _registry.get_components<component::music>();
    auto &triggers = _registry.get_components<component::audio_trigger>();
    systems::audio_system(_registry, sound_effects, musics, triggers,
                          _audioManager.getAudioSystem());

    if (!_gameOver && !isPlayerAlive()) {
        _gameOver = true;
        _gameOverMenu.setVisible(true);
        _audioManager.loadMusic(MusicType::GAME_OVER,
                                "assets/audio/game_over.ogg");
        _audioManager.playMusic(MusicType::GAME_OVER, false);
    }

    if (!_gameOver && !_victory &&
        _bossManager.shouldSpawnBoss(_player, _boss)) {
        // Spawn appropriate boss based on level
        if (_currentLevel == 1) {
            _boss = _bossManager.spawnBoss();
        } else {
            // Level 2: spawn dummy boss entity
            _boss = _bossManager.spawnBossLevel2();

            // Immediately spawn the 3 parts
            render::Vector2u window_size = _window.getSize();
            float boss_x = static_cast<float>(window_size.x) - 250.0f;
            float boss_y = static_cast<float>(window_size.y) / 2.0f;
            _bossManager.spawnBossLevel2Phase2Parts(boss_x, boss_y, _bossParts);
            _bossPhase2 = true;

            std::cout << "[Game] Level 2 boss spawned with 3 parts" << std::endl;
        }
    }

    // Enemy spawning logic
    if (!_gameOver && !_victory) {
        if (_boss && _currentLevel == 2) {
            // Level 2 Boss is present: spawn enemies in waves from boss position
            _bossWaveTimer += dt;
            if (_bossWaveTimer >= _bossWaveInterval) {
                _bossWaveTimer = 0.f;

                // Get boss position from central part
                auto &positions = _registry.get_components<component::position>();
                float boss_x = 0.0f;
                float boss_y = 0.0f;

                if (!_bossParts.empty() && _bossParts.size() >= 2) {
                    // Level 2: use central boss part position (part2)
                    entity center_part = _bossParts[1]; // part2 is the center
                    if (center_part < positions.size() && positions[center_part]) {
                        boss_x = positions[center_part]->x;
                        boss_y = positions[center_part]->y;
                    }
                }

                // Spawn a wave of enemies from boss position
                float wave_spacing = 80.0f;
                float start_y = boss_y - (wave_spacing * (_bossWaveEnemyCount - 1) / 2.0f);

                for (int i = 0; i < _bossWaveEnemyCount; ++i) {
                    entity enemy = _enemyManager.spawnEnemyLevel2();

                    // Position enemies starting from boss position in a vertical wave pattern
                    if (enemy < positions.size() && positions[enemy]) {
                        positions[enemy]->x = boss_x - 50.0f; // Spawn slightly to the left of boss
                        positions[enemy]->y = start_y + (i * wave_spacing);
                    }

                    _enemies.push_back(enemy);
                }
            }
        } else if (!_boss) {
            // No boss: normal enemy spawning
            _enemySpawnTimer += dt;
            if (_enemySpawnTimer >= _enemySpawnInterval) {
                _enemySpawnTimer = 0.f;
                // Spawn appropriate enemy based on level
                if (_currentLevel == 1) {
                    _enemies.push_back(_enemyManager.spawnEnemy());
                } else {
                    // Level 2: randomly spawn either normal or spread enemy
                    if (std::rand() % 3 == 0) { // 33% chance for spread enemy
                        _enemies.push_back(_enemyManager.spawnEnemyLevel2Spread());
                    } else {
                        _enemies.push_back(_enemyManager.spawnEnemyLevel2());
                    }
                }
            }
        }
    }

    // Powerup spawning
    if (!_gameOver && !_victory) {
        _powerupSpawnTimer += dt;
        if (_powerupSpawnTimer >= _powerupSpawnInterval) {
            _powerupSpawnTimer = 0.f;
            // Spawn powerup at random Y position on right side of screen
            render::Vector2u window_size = _window.getSize();
            float random_y = static_cast<float>(std::rand() % (window_size.y - 100) + 50);

            // Randomly choose between shield and spread powerup
            if (std::rand() % 2 == 0) {
                _powerupManager.spawnShieldPowerup(static_cast<float>(window_size.x) - 50.0f, random_y);
            } else {
                _powerupManager.spawnSpreadPowerup(static_cast<float>(window_size.x) - 50.0f, random_y);
            }
        }
    }

    _enemyManager.cleanupOffscreenEnemies(_enemies);
}

void Game::render(float dt) {
    _window.clear();

    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();
    systems::render_system(_registry, positions, drawables, _window, dt);

    if (_isMultiplayer && _networkCommandHandler) {
        uint32_t my_net_id = _networkCommandHandler->getAssignedPlayerNetId();
        auto my_entity = _networkCommandHandler->findEntityByNetId(my_net_id);

        if (my_entity) {
            renderPlayerInfo(*my_entity);
        }

    } else if (_player) {
        renderPlayerInfo(*_player);

        auto &scores = _registry.get_components<component::score>();
        if (*_player < scores.size() && scores[*_player]) {
            renderScore(scores[*_player]->current_score);
        }
    }

#ifdef DEBUG_MODE
    {
        auto &hitboxes = _registry.get_components<component::hitbox>();

        for (size_t i = 0; i < positions.size() && i < hitboxes.size(); ++i) {
            if (positions[i] && hitboxes[i]) {
                auto hitbox_outline = _window.createRectangleShape(
                    render::Vector2f(hitboxes[i]->width, hitboxes[i]->height));
                hitbox_outline->setPosition(
                    positions[i]->x + hitboxes[i]->offset_x,
                    positions[i]->y + hitboxes[i]->offset_y);
                hitbox_outline->setFillColor(render::Color(0, 0, 0, 0));

                if (drawables[i] && drawables[i]->tag == "player") {
                    hitbox_outline->setOutlineColor(render::Color::Green());
                } else if (drawables[i] &&
                           (drawables[i]->tag == "enemy" ||
                            drawables[i]->tag == "enemy_zigzag" ||
                            drawables[i]->tag == "boss")) {
                    hitbox_outline->setOutlineColor(render::Color::Red());
                } else {
                    hitbox_outline->setOutlineColor(render::Color::Yellow());
                }
                hitbox_outline->setOutlineThickness(2.0f);

                _window.draw(*hitbox_outline);
            }
        }
    }
#endif

    _gameOverMenu.render();

    _victoryMenu.render();

    _window.display();
}

void Game::renderPlayerInfo(entity player_entity) {
    auto &healths = _registry.get_components<component::health>();
    auto &shields = _registry.get_components<component::shield>();

    if (player_entity < healths.size() && healths[player_entity]) {
        // Display HP
        auto hp_text = _window.createText();
        hp_text->setFont(*_scoreFont);
        hp_text->setString("HP " + std::to_string(healths[player_entity]->current_hp) + "/" +
                           std::to_string(healths[player_entity]->max_hp));
        hp_text->setCharacterSize(24);
        hp_text->setFillColor(render::Color::White());
        hp_text->setPosition(20, 20);
        _window.draw(*hp_text);

        // Display shield if player has shield component
        if (player_entity < shields.size() && shields[player_entity]) {
            if (shields[player_entity]->current_shield > 0) {
                auto sh_text = _window.createText();
                sh_text->setFont(*_scoreFont);
                sh_text->setString("Shield " + std::to_string(shields[player_entity]->current_shield) +
                                   "/" + std::to_string(shields[player_entity]->max_shield));
                sh_text->setCharacterSize(24);
                sh_text->setFillColor(render::Color(100, 150, 255)); // Light blue for shield
                sh_text->setPosition(20, 50);
                _window.draw(*sh_text);
            }
        }
    }
}

void Game::renderScore(uint32_t score) {
    auto score_text = _window.createText();
    score_text->setFont(*_scoreFont);
    score_text->setString("Score " + std::to_string(score));
    score_text->setCharacterSize(24);
    score_text->setFillColor(render::Color::White());

    render::Vector2u window_size = _window.getSize();
    render::FloatRect text_bounds = score_text->getLocalBounds();
    score_text->setPosition(window_size.x - text_bounds.width - 20, 20);

    _window.draw(*score_text);
}

void Game::run() {
    _audioManager.loadMusic(MusicType::IN_GAME, "assets/audio/in_game.ogg");
    _audioManager.playMusic(MusicType::IN_GAME, true);

    _shouldExit = false;
    _tickSystem.reset();

    auto should_continue = [this]() -> bool { return !_shouldExit; };

    auto update_func = [this](double dt) -> void {
        bool dummy_running = true;
        handleEvents(dummy_running, static_cast<float>(dt));
        update(static_cast<float>(dt));
    };

    auto render_func = [this](double /*interpolation*/) -> void {
        render(static_cast<float>(_tickSystem.getTickDelta()));
    };

    _tickSystem.run(should_continue, update_func, render_func);
}

bool Game::isPlayerAlive() const {
    return _playerManager.isPlayerAlive(_player);
}

void Game::setLevel(int level) {
    _currentLevel = level;
    std::cout << "[Game] Level set to " << level << std::endl;

    // Reload background for the new level
    if (_background) {
        auto &backgrounds = _registry.get_components<component::background>();
        if (_background.value() < backgrounds.size() && backgrounds[*_background]) {
            auto &bg_component = backgrounds[*_background];
            bg_component->texture = _window.createTexture();

            std::string backgroundFile;
            if (_currentLevel == 1) {
                backgroundFile = "assets/background.jpg";
            } else {
                // Level 2+ uses r-typesheet-background.png
                backgroundFile = "assets/sprites/r-typesheet-background.png";
            }

            if (!bg_component->texture->loadFromFile(backgroundFile)) {
                auto fallback_image = _window.createImage();
                fallback_image->create(800, 600, render::Color(20, 20, 80));
                bg_component->texture->loadFromImage(*fallback_image);
            }
            std::cout << "[Game] Background loaded: " << backgroundFile << std::endl;
        }
    }
}

void Game::cleanup() {
    std::cout << "[Game] Cleaning up game session..." << std::endl;

    // Kill player
    if (_player) {
        _registry.kill_entity(*_player);
        _player.reset();
    }

    // Kill player shield if exists
    if (_playerShield) {
        _registry.kill_entity(*_playerShield);
        _playerShield.reset();
    }

    // Kill background
    if (_background) {
        _registry.kill_entity(*_background);
        _background.reset();
    }

    // Kill boss
    if (_boss) {
        _registry.kill_entity(*_boss);
        _boss.reset();
    }

    // Kill all enemies
    for (const auto &enemy : _enemies) {
        _registry.kill_entity(enemy);
    }
    _enemies.clear();

    // Kill all remaining entities
    auto &positions = _registry.get_components<component::position>();
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i]) {
            _registry.kill_entity(entity(i));
        }
    }

    std::cout << "[Game] Cleanup complete" << std::endl;
}

void Game::resetGame() {
    _gameOver = false;
    _victory = false;
    _gameOverMenu.setVisible(false);
    _victoryMenu.setVisible(false);
    _gameTime = 0.f;
    _enemySpawnTimer = 0.f;
    _bossWaveTimer = 0.f;
    _audioManager.playMusic(MusicType::IN_GAME, true);

    if (_player) {
        _registry.kill_entity(*_player);
        _player.reset();
    }
    if (_background) {
        _registry.kill_entity(*_background);
        _background.reset();
    }
    if (_boss) {
        _registry.kill_entity(*_boss);
        _boss.reset();
    }
    for (const auto &part : _bossParts) {
        _registry.kill_entity(part);
    }
    _bossParts.clear();
    _bossPhase2 = false;
    for (const auto &enemy : _enemies) {
        _registry.kill_entity(enemy);
    }
    _enemies.clear();

    auto &positions = _registry.get_components<component::position>();
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i]) {
            _registry.kill_entity(entity(i));
        }
    }

    // Load background based on current level
    _background = _registry.spawn_entity();
    auto &bg_component = _registry.add_component<component::background>(
        *_background, component::background(30.f));
    bg_component->texture = _window.createTexture();

    std::string backgroundFile;
    if (_currentLevel == 1) {
        backgroundFile = "assets/background.jpg";
    } else {
        // Level 2+ uses r-typesheet-background.png
        backgroundFile = "assets/sprites/r-typesheet-background.png";
    }

    if (!bg_component->texture->loadFromFile(backgroundFile)) {
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(20, 20, 80));
        bg_component->texture->loadFromImage(*fallback_image);
    }

    _player = _playerManager.createPlayer(_playerRelativeX, _playerRelativeY,
                                          _playerSpeed);
}

void Game::updateMultiplayer(float dt) {
    _gameTime += dt;

    if (_networkManager) {
        systems::network_system(dt);

        // Vérifier l'état de connexion
        auto connectionState = _networkManager->getConnectionState();
        if (connectionState == network::ConnectionState::ERROR ||
            connectionState == network::ConnectionState::DISCONNECTED) {
            _shouldExit = true;
            return;
        }
    }

    auto &positions = _registry.get_components<component::position>();
    auto &inputs = _registry.get_components<component::input>();

    systems::input_system(_registry, inputs, _window, &_keyBindings);

    sendPlayerInput(dt);

    auto &backgrounds = _registry.get_components<component::background>();
    for (size_t i = 0; i < backgrounds.size() && i < positions.size(); ++i) {
        if (backgrounds[i] && positions[i]) {
            backgrounds[i]->offset_x -= backgrounds[i]->scroll_speed * dt;
            if (backgrounds[i]->offset_x <=
                -backgrounds[i]->texture->getSize().x) {
                backgrounds[i]->offset_x = 0;
            }
        }
    }

    checkGameEndConditions();
}

void Game::checkGameEndConditions() {
    if (_networkCommandHandler) {
        uint32_t my_net_id = _networkCommandHandler->getAssignedPlayerNetId();

        if (my_net_id != 0) {
            auto my_entity =
                _networkCommandHandler->findEntityByNetId(my_net_id);

            if (!my_entity) {
                if (!_gameOver) {
                    _gameOver = true;
                    _gameOverMenu.setVisible(true);
                    _audioManager.loadMusic(MusicType::GAME_OVER,
                                            "assets/audio/game_over.ogg");
                    _audioManager.playMusic(MusicType::GAME_OVER, false);
                }
            } else {
                _player = my_entity;

                auto &inputs = _registry.get_components<component::input>();
                if (*my_entity >= inputs.size() || !inputs[*my_entity]) {
                    _registry.add_component<component::input>(
                        *my_entity, component::input());
                }
            }
        }
    }
}

void Game::sendPlayerInput(float dt) {
    _inputSendTimer += dt;

    if (_inputSendTimer >= _inputSendInterval) {
        _inputSendTimer = 0.0f;

        if (!_networkManager)
            return;

        uint32_t my_net_id = _networkManager->getAssignedPlayerNetId();
        std::cout << "[Game] My NET_ID from NetworkManager: " << my_net_id
                  << std::endl;

        if (my_net_id == 0 || !_networkCommandHandler)
            return;

        auto my_entity = _networkCommandHandler->findEntityByNetId(my_net_id);

        if (!my_entity) {
            std::cout << "[Game] Entity not found for NET_ID: " << my_net_id
                      << std::endl;
            return;
        }

        auto &inputs = _registry.get_components<component::input>();
        if (*my_entity >= inputs.size() || !inputs[*my_entity])
            return;
        std::cout << "MAIS PKKKKKKKKKKK\n";

        auto &playerInput = inputs[*my_entity];

        uint8_t direction = 0;
        if (playerInput->up)
            direction |= 0x01;
        if (playerInput->down)
            direction |= 0x02;
        if (playerInput->left)
            direction |= 0x04;
        if (playerInput->right)
            direction |= 0x08;

        if (direction != 0) {
            _networkManager->sendPlayerInput(direction);
        }

        if (playerInput->fire) {
            _networkManager->sendPlayerFire();
        }
    }
}
