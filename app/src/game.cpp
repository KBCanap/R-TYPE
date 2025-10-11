#include "../include/game.hpp"
#include "../include/settings.hpp"
#include "../include/options_menu.hpp"
#include "../include/accessibility_menu.hpp"
#include "../include/keybindings_menu.hpp"
#include "../include/network/NetworkManager.hpp"
#include "systems.hpp"
#include "network/NetworkSystem.hpp"
#include "network/NetworkComponents.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>

Game::Game(registry& reg, render::IRenderWindow& win, AudioManager& audioMgr, KeyBindings& keyBindings, network::NetworkManager* netMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr), _keyBindings(keyBindings),
      _networkManager(netMgr), _isMultiplayer(netMgr != nullptr),
      _playerManager(reg, win),
      _enemyManager(reg, win),
      _bossManager(reg, win),
      _gameOverMenu(win, audioMgr),
      _victoryMenu(win, audioMgr),
      _tickSystem(60.0)
{

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Register network components if in multiplayer mode
    if (_isMultiplayer) {
        _registry.register_component<component::network_entity>();
    }

    // Create background
    _background = _registry.spawn_entity();
    auto& bg_component = _registry.add_component<component::background>(*_background, component::background(30.f));
    bg_component->texture = _window.createTexture();
    if (!bg_component->texture->loadFromFile("assets/background.jpg")) {
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(20, 20, 80));
        bg_component->texture->loadFromImage(*fallback_image);
    }

    // Load r-type font for score display
    _scoreFont = _window.createFont();
    if (!_scoreFont->loadFromFile("assets/r-type.otf")) {
    }

    // In multiplayer, don't create local player yet - wait for server assignment
    if (!_isMultiplayer) {
        _player = _playerManager.createPlayer(_playerRelativeX, _playerRelativeY, _playerSpeed);
    }
}

void Game::handleEvents(bool& running, float /*dt*/) {
    render::Event event;
    while (_window.pollEvent(event)) {
        // Mettre à jour l'état des touches pour input_system
        systems::update_key_state(event);

        // Handle window close
        if (event.type == render::EventType::Closed) {
            running = false;
            _shouldExit = true;
            return;
        }

        // Handle window resize
        if (event.type == render::EventType::Resized) {
            _playerManager.updatePlayerPosition(_player, _playerRelativeX, _playerRelativeY);
            _enemyManager.updateEnemyPositions(_enemies);
            _gameOverMenu.onWindowResize();
            _victoryMenu.onWindowResize();
        }

        // Handle game over menu
        if (_gameOver) {
            MenuAction action = _gameOverMenu.handleEvents(event);
            if (action == MenuAction::REPLAY) {
                // In multiplayer, REPLAY means return to menu (exit game)
                // In solo, REPLAY means restart the game
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

        // Handle victory menu
        if (_victory) {
            MenuAction action = _victoryMenu.handleEvents(event);
            if (action == MenuAction::REPLAY) {
                // In multiplayer, REPLAY means return to menu (exit game)
                // In solo, REPLAY means restart the game
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

        // Handle key presses
        if (event.type == render::EventType::KeyPressed) {
            // Escape to exit
            if (event.key.code == render::Key::Escape) {
                running = false;
                _shouldExit = true;
                return;
            }

            // Handle weapon switching (solo mode only)
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
                    OptionsMenu optionsMenu(_window, _audioManager, _keyBindings);
                    OptionsResult result = optionsMenu.run();
                    if (result == OptionsResult::Accessibility) {
                        AccessibilityMenu accessibilityMenu(_window, _audioManager);
                        accessibilityMenu.run();
                    }
                    if (result == OptionsResult::KeyBindings) {
                        KeyBindingsMenu keyBindingsMenu(_window, _audioManager, _keyBindings);
                        keyBindingsMenu.run();
                    }
                }
            }
        }
    }
}

void Game::update(float dt) {
    // Use multiplayer logic if network manager is present
    if (_isMultiplayer && _networkManager) {
        updateMultiplayer(dt);
        return;
    }

    _gameTime += dt;

    auto& positions = _registry.get_components<component::position>();
    auto& velocities = _registry.get_components<component::velocity>();
    auto& controllables = _registry.get_components<component::controllable>();
    auto& projectiles = _registry.get_components<component::projectile>();
    auto& drawables = _registry.get_components<component::drawable>();
    auto& inputs = _registry.get_components<component::input>();

    // Input system - always runs to capture input state
    systems::input_system(_registry, inputs, _window, &_keyBindings);

    // AI input system for enemies - always runs
    auto& ai_inputs = _registry.get_components<component::ai_input>();
    systems::ai_input_system(_registry, ai_inputs, dt);

    // Only allow player control if game is not over and not victory
    if (!_gameOver && !_victory) {
        systems::control_system(_registry, controllables, velocities, inputs, dt);
    }

    // Weapon system - handle weapon firing and projectile creation
    if (!_gameOver && !_victory) {
        auto& weapons = _registry.get_components<component::weapon>();
        systems::weapon_system(_registry, weapons, positions, inputs, ai_inputs, _gameTime);
    }

    // These systems should continue running even during game over
    systems::position_system(_registry, positions, velocities, inputs, _window, _gameTime, dt);

    // Update projectile lifetimes and cleanup
    systems::projectile_system(_registry, projectiles, positions, _window, dt);

    // Update player relative position and constrain to screen bounds
    if (_player && !_gameOver && !_victory) {
        auto& player_pos = positions[*_player];
        if (player_pos) {
            render::Vector2u window_size = _window.getSize();

            // Constrain player to screen bounds
            player_pos->x = std::max(0.f, std::min(player_pos->x, static_cast<float>(window_size.x) - 40.f));
            player_pos->y = std::max(0.f, std::min(player_pos->y, static_cast<float>(window_size.y) - 40.f));

            // Update relative position
            _playerRelativeX = player_pos->x / static_cast<float>(window_size.x);
            _playerRelativeY = player_pos->y / static_cast<float>(window_size.y);
        }
    }

    if (!_gameOver && !_victory) {
        auto& hitboxes = _registry.get_components<component::hitbox>();
        systems::collision_system(_registry, positions, drawables, projectiles, hitboxes);
    }

    // Health system - process damage and handle entity death
    auto& healths = _registry.get_components<component::health>();
    systems::health_system(_registry, healths, dt);

    // Check for victory (boss defeated)
    if (!_gameOver && !_victory && _boss) {
        auto& boss_pos = positions[*_boss];
        auto& boss_drawable = drawables[*_boss];
        if (!boss_pos || !boss_drawable) {
            _victory = true;
            _victoryMenu.setVisible(true);
            _boss.reset();
        }
    }

    if (!_gameOver && !_victory) {
        auto& scores = _registry.get_components<component::score>();
        systems::score_system(_registry, scores, dt);
    }

    // Audio system
    auto& sound_effects = _registry.get_components<component::sound_effect>();
    auto& musics = _registry.get_components<component::music>();
    auto& triggers = _registry.get_components<component::audio_trigger>();
    systems::audio_system(_registry, sound_effects, musics, triggers, _audioManager.getAudioSystem());

    // Check if player is still alive
    if (!_gameOver && !isPlayerAlive()) {
        _gameOver = true;
        _gameOverMenu.setVisible(true);
        _audioManager.loadMusic(MusicType::GAME_OVER, "assets/audio/game_over.ogg");
        _audioManager.playMusic(MusicType::GAME_OVER, false);
    }

    // Check score and spawn boss using BossManager
    if (!_gameOver && !_victory && _bossManager.shouldSpawnBoss(_player, _boss)) {
        _boss = _bossManager.spawnBoss();
    }

    // Spawn enemies - only if game is not over, not victory AND boss hasn't spawned yet
    if (!_gameOver && !_victory && !_boss) {
        _enemySpawnTimer += dt;
        if (_enemySpawnTimer >= _enemySpawnInterval) {
            _enemySpawnTimer = 0.f;
            entity newEnemy = _enemyManager.spawnEnemy();
            _enemies.push_back(newEnemy);
        }
    }

    // Remove enemies that are off-screen
    _enemyManager.cleanupOffscreenEnemies(_enemies);
}

void Game::render(float dt) {
    _window.clear();

    auto& positions = _registry.get_components<component::position>();
    auto& drawables = _registry.get_components<component::drawable>();
    systems::render_system(_registry, positions, drawables, _window, dt);

    // In multiplayer mode, find the correct player entity by NET_ID
    if (_isMultiplayer && _networkManager) {
        uint32_t my_net_id = _networkManager->getAssignedNetId();
        auto it = _networkEntities.find(my_net_id);

        if (it != _networkEntities.end() && it->second) {
            entity local_player = *(it->second);

            auto& healths = _registry.get_components<component::health>();

            if (local_player < healths.size() && healths[local_player]) {
                auto& player_health = healths[local_player];

                auto hp_text = _window.createText();
                hp_text->setFont(*_scoreFont);
                hp_text->setString("HP " + std::to_string(player_health->current_hp) + "/" + std::to_string(player_health->max_hp));
                hp_text->setCharacterSize(24);
                hp_text->setFillColor(render::Color::White());
                hp_text->setPosition(20, 20);

                _window.draw(*hp_text);
            }
        }

        // Display score in multiplayer mode
        uint32_t game_score = _networkManager->getGameScore();
        auto score_text = _window.createText();
        score_text->setFont(*_scoreFont);
        score_text->setString("Score " + std::to_string(game_score));
        score_text->setCharacterSize(24);
        score_text->setFillColor(render::Color::White());

        render::Vector2u window_size = _window.getSize();
        render::FloatRect text_bounds = score_text->getLocalBounds();
        score_text->setPosition(window_size.x - text_bounds.width - 20, 20);

        _window.draw(*score_text);
    } else if (_player) {
        // Solo mode - use _player directly
        auto& healths = _registry.get_components<component::health>();
        auto& scores = _registry.get_components<component::score>();

        if (*_player < healths.size() && healths[*_player]) {
            auto& player_health = healths[*_player];

            auto hp_text = _window.createText();
            hp_text->setFont(*_scoreFont);
            hp_text->setString("HP " + std::to_string(player_health->current_hp) + "/" + std::to_string(player_health->max_hp));
            hp_text->setCharacterSize(24);
            hp_text->setFillColor(render::Color::White());
            hp_text->setPosition(20, 20);

            _window.draw(*hp_text);
        }

        if (*_player < scores.size() && scores[*_player]) {
            auto& player_score = scores[*_player];

            auto score_text = _window.createText();
            score_text->setFont(*_scoreFont);
            score_text->setString("Score " + std::to_string(player_score->current_score));
            score_text->setCharacterSize(24);
            score_text->setFillColor(render::Color::White());

            render::Vector2u window_size = _window.getSize();
            render::FloatRect text_bounds = score_text->getLocalBounds();
            score_text->setPosition(window_size.x - text_bounds.width - 20, 20);

            _window.draw(*score_text);
        }
    }

#ifdef DEBUG_MODE
    // Draw debug hitboxes
    {
        auto& hitboxes = _registry.get_components<component::hitbox>();

        for (size_t i = 0; i < positions.size() && i < hitboxes.size(); ++i) {
            if (positions[i] && hitboxes[i]) {
                auto hitbox_outline = _window.createRectangleShape(
                    render::Vector2f(hitboxes[i]->width, hitboxes[i]->height)
                );
                hitbox_outline->setPosition(
                    positions[i]->x + hitboxes[i]->offset_x,
                    positions[i]->y + hitboxes[i]->offset_y
                );
                hitbox_outline->setFillColor(render::Color(0, 0, 0, 0)); // Transparent

                // Different colors for different entity types
                if (drawables[i] && drawables[i]->tag == "player") {
                    hitbox_outline->setOutlineColor(render::Color::Green());
                } else if (drawables[i] && (drawables[i]->tag == "enemy" || drawables[i]->tag == "enemy_zigzag" || drawables[i]->tag == "boss")) {
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

void Game::run() {
    _audioManager.loadMusic(MusicType::IN_GAME, "assets/audio/in_game.ogg");
    _audioManager.playMusic(MusicType::IN_GAME, true);

    _shouldExit = false;
    _tickSystem.reset();

    auto should_continue = [this]() -> bool {
        return !_shouldExit;
    };

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

void Game::resetGame() {
    _gameOver = false;
    _victory = false;
    _gameOverMenu.setVisible(false);
    _victoryMenu.setVisible(false);
    _gameTime = 0.f;
    _enemySpawnTimer = 0.f;
    _audioManager.playMusic(MusicType::IN_GAME, true);

    // Kill all existing entities
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
    for (const auto& enemy : _enemies) {
        _registry.kill_entity(enemy);
    }
    _enemies.clear();

    // Kill all projectiles and other entities by checking all components
    auto& positions = _registry.get_components<component::position>();
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i]) {
            _registry.kill_entity(entity(i));
        }
    }

    // Recreate background
    _background = _registry.spawn_entity();
    auto& bg_component = _registry.add_component<component::background>(*_background, component::background(30.f));
    bg_component->texture = _window.createTexture();
    if (!bg_component->texture->loadFromFile("assets/background.jpg")) {
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(20, 20, 80));
        bg_component->texture->loadFromImage(*fallback_image);
    }

    // Recreate player using PlayerManager
    _player = _playerManager.createPlayer(_playerRelativeX, _playerRelativeY, _playerSpeed);
}
// Multiplayer-specific update logic
void Game::updateMultiplayer(float dt) {
    _gameTime += dt;

    // Process network messages and update entities
    if (_networkManager) {
        _networkManager->processMessages();
        processNetworkEntities();
    }

    auto& positions = _registry.get_components<component::position>();
    auto& inputs = _registry.get_components<component::input>();

    // Input system - capture local player input
    systems::input_system(_registry, inputs, _window, &_keyBindings);

    // Send player input to server
    sendPlayerInput(dt);

    // Update background scrolling
    auto& backgrounds = _registry.get_components<component::background>();
    for (size_t i = 0; i < backgrounds.size() && i < positions.size(); ++i) {
        if (backgrounds[i] && positions[i]) {
            backgrounds[i]->offset_x -= backgrounds[i]->scroll_speed * dt;
            if (backgrounds[i]->offset_x <= -backgrounds[i]->texture->getSize().x) {
                backgrounds[i]->offset_x = 0;
            }
        }
    }

    // Note: In multiplayer, the server handles all game logic
    // Clients only render what the server tells them
}

void Game::sendPlayerInput(float dt) {
    _inputSendTimer += dt;

    if (_inputSendTimer >= _inputSendInterval) {
        _inputSendTimer = 0.0f;

        if (!_networkManager || !_player) {
            return;
        }

        auto& inputs = _registry.get_components<component::input>();
        if (*_player >= inputs.size() || !inputs[*_player]) {
            return;
        }

        auto& playerInput = inputs[*_player];
        uint8_t direction = 0;

        // Encode movement
        if (playerInput->up) direction |= 0x01;     // UP
        if (playerInput->down) direction |= 0x02;   // DOWN
        if (playerInput->left) direction |= 0x04;   // LEFT
        if (playerInput->right) direction |= 0x08;  // RIGHT
        if (playerInput->fire) direction |= 0x10;   // FIRE (SPACE)

        // Send PLAYER_INPUT message
        _networkManager->sendPlayerInput(direction);
    }
}

void Game::processNetworkEntities() {
    if (!_networkManager) return;

    auto& networkEntities = _networkManager->getNetworkEntities();
    auto& positions = _registry.get_components<component::position>();
    auto& healths = _registry.get_components<component::health>();

    render::Vector2u window_size = _window.getSize();

    // Process all network entities
    for (const auto& [net_id, netEntity] : networkEntities) {
        // Check if we already have this entity
        auto it = _networkEntities.find(net_id);

        if (it == _networkEntities.end()) {
            // Create new entity
            entity ent = _registry.spawn_entity();
            _networkEntities[net_id] = std::optional<entity>(ent);

            // Add position
            _registry.add_component<component::position>(ent, component::position(
                netEntity.pos_x * window_size.x,
                netEntity.pos_y * window_size.y
            ));

            // Add drawable with appropriate texture using R-TYPE sprites
            std::string texturePath;
            std::string tag;
            render::IntRect textureRect;
            float scale;

            switch (netEntity.type) {
                case 0x01: // PLAYER
                    texturePath = "assets/sprites/r-typesheet42.gif";
                    tag = "player";
                    textureRect = render::IntRect(66, 0, 33, 17);  // Middle frame of player
                    scale = 2.0f;
                    break;
                case 0x02: // ENEMY
                    texturePath = "assets/sprites/r-typesheet9.gif";
                    tag = "enemy";
                    textureRect = render::IntRect();  // Full sprite
                    scale = 1.0f;
                    break;
                case 0x05: // ENEMY_SPREAD
                    texturePath = "assets/sprites/r-typesheet3.gif";
                    tag = "enemy_spread";
                    textureRect = render::IntRect();  // Full sprite
                    scale = 3.0f;
                    break;
                case 0x06: // BOSS
                    texturePath = "assets/sprites/r-typesheet17.gif";
                    tag = "boss";
                    textureRect = render::IntRect();  // Full sprite
                    scale = 2.0f;
                    break;
                case 0x03: // PROJECTILE (enemy)
                    texturePath = "assets/sprites/r-typesheet1.gif";
                    tag = "projectile_enemy";
                    textureRect = render::IntRect(249, 103, 16, 12);
                    scale = 1.0f;
                    break;
                case 0x04: // ALLIED_PROJECTILE
                    texturePath = "assets/sprites/r-typesheet1.gif";
                    tag = "projectile";
                    textureRect = render::IntRect(60, 353, 12, 12);
                    scale = 1.0f;
                    break;
                default:
                    texturePath = "assets/sprites/r-typesheet42.gif";
                    tag = "unknown";
                    textureRect = render::IntRect(66, 0, 33, 17);
                    scale = 2.0f;
                    break;
            }

            _registry.add_component<component::drawable>(ent,
                component::drawable(texturePath, textureRect, scale, tag));

            // Add animation component based on entity type
            if (netEntity.type == 0x01) { // PLAYER
                auto& player_anim = _registry.add_component<component::animation>(ent, component::animation(0.2f, true));
                player_anim->frames.push_back(render::IntRect(0, 0, 33, 17));     // Phase 1: neutral up
                player_anim->frames.push_back(render::IntRect(33, 0, 33, 17));    // Phase 2: slight up
                player_anim->frames.push_back(render::IntRect(66, 0, 33, 17));    // Phase 3: middle/neutral
                player_anim->frames.push_back(render::IntRect(99, 0, 33, 17));    // Phase 4: slight down
                player_anim->frames.push_back(render::IntRect(132, 0, 33, 17));   // Phase 5: full down
                player_anim->current_frame = 2; // Start at middle frame
                player_anim->playing = true; // Animation playing
            } else if (netEntity.type == 0x02) { // ENEMY
                // Regular enemy animation (3 frames from r-typesheet9.gif)
                auto& enemy_anim = _registry.add_component<component::animation>(ent, component::animation(0.5f, true));
                enemy_anim->frames.push_back(render::IntRect(0, 0, 50, 58));     // Frame 1
                enemy_anim->frames.push_back(render::IntRect(51, 0, 57, 58));    // Frame 2
                enemy_anim->frames.push_back(render::IntRect(116, 0, 49, 58));   // Frame 3
            } else if (netEntity.type == 0x05) { // ENEMY_SPREAD
                // Spread enemy animation (12 frames from r-typesheet3.gif)
                auto& enemy_anim = _registry.add_component<component::animation>(ent, component::animation(0.5f, true));
                enemy_anim->frames.push_back(render::IntRect(0, 0, 17, 18));      // Frame 1
                enemy_anim->frames.push_back(render::IntRect(17, 0, 17, 18));     // Frame 2
                enemy_anim->frames.push_back(render::IntRect(34, 0, 17, 18));     // Frame 3
                enemy_anim->frames.push_back(render::IntRect(51, 0, 17, 18));     // Frame 4
                enemy_anim->frames.push_back(render::IntRect(68, 0, 17, 18));     // Frame 5
                enemy_anim->frames.push_back(render::IntRect(85, 0, 17, 18));     // Frame 6
                enemy_anim->frames.push_back(render::IntRect(102, 0, 17, 18));    // Frame 7
                enemy_anim->frames.push_back(render::IntRect(119, 0, 17, 18));    // Frame 8
                enemy_anim->frames.push_back(render::IntRect(136, 0, 17, 18));    // Frame 9
                enemy_anim->frames.push_back(render::IntRect(153, 0, 17, 18));    // Frame 10
                enemy_anim->frames.push_back(render::IntRect(170, 0, 17, 18));    // Frame 11
                enemy_anim->frames.push_back(render::IntRect(187, 0, 17, 18));    // Frame 12
            } else if (netEntity.type == 0x06) { // BOSS
                // Boss animation (8 frames from r-typesheet17.gif)
                auto& boss_anim = _registry.add_component<component::animation>(ent, component::animation(0.1f, true));
                for (int i = 0; i < 8; ++i) {
                    boss_anim->frames.push_back(render::IntRect(i * 65, 0, 65, 132));
                }
            }

            // Add health component
            _registry.add_component<component::health>(ent, component::health(netEntity.health));

            // Add network component
            _registry.add_component<component::network_entity>(ent, component::network_entity(net_id));

            // Mark as local player if this is our assigned entity
            if (netEntity.type == 0x01 && net_id == _networkManager->getAssignedNetId()) {
                _player = ent;
                _registry.add_component<component::input>(ent, component::input());
            }
        } else {
            // Update existing entity
            if (!it->second) continue;
            entity ent = *(it->second);

            // Track old position for animation
            float old_y = 0.0f;
            if (ent < positions.size() && positions[ent]) {
                old_y = positions[ent]->y;
                positions[ent]->x = netEntity.pos_x * window_size.x;
                positions[ent]->y = netEntity.pos_y * window_size.y;

                // Update player animation based on vertical movement
                if (netEntity.type == 0x01) { // PLAYER
                    auto& animations = _registry.get_components<component::animation>();
                    if (ent < animations.size() && animations[ent]) {
                        auto& anim = animations[ent];
                        float delta_y = positions[ent]->y - old_y;

                        if (delta_y < -0.5f) {  // Moving up
                            anim->playing = false;
                            anim->current_frame = 4;  // Frame 4: full up
                        } else if (delta_y > 0.5f) {  // Moving down
                            anim->playing = false;
                            anim->current_frame = 0;  // Frame 0: full down
                        } else {  // Not moving much vertically
                            anim->playing = false;
                            anim->current_frame = 2;  // Frame 2: middle/neutral
                        }
                    }
                }
            }

            if (ent < healths.size() && healths[ent]) {
                healths[ent]->current_hp = netEntity.health;
            }
        }
    }

    // Remove entities that no longer exist on server
    std::vector<uint32_t> toRemove;
    for (const auto& [net_id, opt_ent] : _networkEntities) {
        if (networkEntities.find(net_id) == networkEntities.end()) {

            if (opt_ent) {
                entity ent = *opt_ent;

                // Check if this is a boss being destroyed (victory condition)
                auto& drawables = _registry.get_components<component::drawable>();
                if (ent < drawables.size() && drawables[ent] &&
                    drawables[ent]->tag == "boss" && !_victory) {
                    _victory = true;
                    _victoryMenu.setVisible(true);
                    // Keep the current music playing for victory
                }

                // Create explosion effect if entity has position
                if (ent < positions.size() && positions[ent]) {
                    auto explosion_entity = _registry.spawn_entity();
                    _registry.add_component<component::position>(explosion_entity,
                        component::position(positions[ent]->x, positions[ent]->y));
                    _registry.add_component<component::drawable>(explosion_entity,
                        component::drawable("assets/sprites/r-typesheet1.gif",
                                          render::IntRect(70, 290, 36, 32), 2.0f, "explosion"));
                    auto& anim = _registry.add_component<component::animation>(explosion_entity,
                        component::animation(0.1f, false, true));
                    anim->frames.push_back(render::IntRect(70, 290, 36, 32));
                    anim->frames.push_back(render::IntRect(106, 290, 36, 32));
                    anim->frames.push_back(render::IntRect(142, 290, 36, 32));
                    anim->frames.push_back(render::IntRect(178, 290, 35, 32));
                    anim->playing = true;
                }

                // Check if this is the local player dying
                if (_player && ent == *_player) {
                    _gameOver = true;
                    _gameOverMenu.setVisible(true);
                    _audioManager.loadMusic(MusicType::GAME_OVER, "assets/audio/game_over.ogg");
                    _audioManager.playMusic(MusicType::GAME_OVER, false);
                }

                _registry.kill_entity(ent);
            }
            toRemove.push_back(net_id);
        }
    }

    for (uint32_t net_id : toRemove) {
        _networkEntities.erase(net_id);
    }
}
