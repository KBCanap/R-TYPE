#include "../include/game.hpp"
#include "../include/settings.hpp"
#include "../include/options_menu.hpp"
#include "../include/accessibility_menu.hpp"
#include "systems.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>

Game::Game(registry& reg, render::IRenderWindow& win, AudioManager& audioMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr),
      _playerManager(reg, win),
      _enemyManager(reg, win),
      _bossManager(reg, win),
      _gameOverMenu(win, audioMgr),
      _victoryMenu(win, audioMgr),
      _tickSystem(60.0)
{

    std::srand(static_cast<unsigned>(std::time(nullptr)));

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
    _player = _playerManager.createPlayer(_playerRelativeX, _playerRelativeY, _playerSpeed);
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

        // Handle key presses
        if (event.type == render::EventType::KeyPressed) {
            // Escape to exit
            if (event.key.code == render::Key::Escape) {
                running = false;
                _shouldExit = true;
                return;
            }

            // Handle weapon switching
            if (!_gameOver && !_victory) {
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
                    OptionsMenu optionsMenu(_window, _audioManager);
                    OptionsResult result = optionsMenu.run();
                    if (result == OptionsResult::Accessibility) {
                        AccessibilityMenu accessibilityMenu(_window, _audioManager);
                        accessibilityMenu.run();
                    }
                }
            }

            // Handle game over menu
            if (_gameOver) {
                if (event.key.code == render::Key::Enter) {
                    resetGame();
                } else if (event.key.code == render::Key::Q) {
                    running = false;
                    _shouldExit = true;
                }
            }

            // Handle victory menu
            if (_victory) {
                if (event.key.code == render::Key::Enter) {
                    resetGame();
                } else if (event.key.code == render::Key::Q) {
                    running = false;
                    _shouldExit = true;
                }
            }
        }
    }
}

void Game::update(float dt) {
    _gameTime += dt;

    auto& positions = _registry.get_components<component::position>();
    auto& velocities = _registry.get_components<component::velocity>();
    auto& controllables = _registry.get_components<component::controllable>();
    auto& projectiles = _registry.get_components<component::projectile>();
    auto& drawables = _registry.get_components<component::drawable>();
    auto& inputs = _registry.get_components<component::input>();

    // Input system - always runs to capture input state
    systems::input_system(_registry, inputs, _window);

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

    if (_player) {
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