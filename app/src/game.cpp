#include "../include/game.hpp"
#include "systems.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>

Game::Game(registry& reg, sf::RenderWindow& win, AudioManager& audioMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr), _gameOverMenu(win, audioMgr), _tickSystem(60.0)
{
    // Reset the window view to default after menu potentially modified it
    sf::Vector2u window_size = _window.getSize();
    sf::View default_view;
    default_view.setSize(static_cast<float>(window_size.x), static_cast<float>(window_size.y));
    default_view.setCenter(static_cast<float>(window_size.x) / 2.f, static_cast<float>(window_size.y) / 2.f);
    _window.setView(default_view);

    // Initialize enemy weapon creators array
    enemyWeaponCreators[0] = &Game::createEnemySingleWeapon;
    enemyWeaponCreators[1] = &Game::createEnemyBurstWeapon;
    enemyWeaponCreators[2] = &Game::createEnemySpreadWeapon;
    enemyWeaponCreators[3] = &Game::createEnemyZigzagSpreadWeapon;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    // Crée le background
    _background = _registry.spawn_entity();
    auto& bg_component = _registry.add_component<component::background>(*_background, component::background(30.f));
    if (!bg_component->texture.loadFromFile("assets/background.jpg")) {
        // Fallback si le fichier n'existe pas
        sf::Image fallback_image;
        fallback_image.create(800, 600, sf::Color(20, 20, 80));
        bg_component->texture.loadFromImage(fallback_image);
    }

    // Load r-type font for score display
    if (!_scoreFont.loadFromFile("assets/r-type.otf")) {
        // If loading fails, use default font (no action needed, SFML will use default)
    }

    // Crée le player via le registry
    _player = _registry.spawn_entity();
    _registry.add_component<component::position>(*_player, component::position(getRelativeX(_playerRelativeX), getRelativeY(_playerRelativeY)));
    _registry.add_component<component::velocity>(*_player, component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(*_player, component::drawable("assets/sprites/r-typesheet42.gif", sf::IntRect(0, 0, 33, 17), 2.0f, "player"));
    _registry.add_component<component::controllable>(*_player, component::controllable(_playerSpeed));
    // Arme RAPID : cadence élevée, dégâts faibles, projectile unique
    _registry.add_component<component::weapon>(*_player,
        component::weapon(8.0f, true, 1, 0.0f, component::projectile_pattern::straight(),
                         10.0f, 600.0f, 5.0f, false, 1, false, 3, 0.1f,
                         sf::IntRect(60, 353, 12, 12)));
    _registry.add_component<component::hitbox>(*_player, component::hitbox(66.0f, 34.0f, 15.0f, 0.0f)); // Hitbox du joueur avec offset à droite
    _registry.add_component<component::input>(*_player, component::input()); // Composant input du joueur
    _registry.add_component<component::score>(*_player, component::score(0)); // Composant score du joueur

    // Add player animation with 5 phases
    auto& player_anim = _registry.add_component<component::animation>(*_player, component::animation(0.2f, true));
    player_anim->frames.push_back(sf::IntRect(0, 0, 33, 17));     // Phase 1: neutral up
    player_anim->frames.push_back(sf::IntRect(33, 0, 33, 17));    // Phase 2: slight up
    player_anim->frames.push_back(sf::IntRect(66, 0, 33, 17));    // Phase 3: middle/neutral
    player_anim->frames.push_back(sf::IntRect(99, 0, 33, 17));    // Phase 4: slight down
    player_anim->frames.push_back(sf::IntRect(132, 0, 33, 17));   // Phase 5: full down
    player_anim->current_frame = 2; // Start at middle frame
    player_anim->playing = false; // Start with animation stopped
}

void Game::handleEvents(bool& running, float /*dt*/) {
    sf::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            running = false;
            _shouldExit = true;
            return;
        }

        if (event.type == sf::Event::Resized) {
            sf::View view;
            view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
            view.setCenter(static_cast<float>(event.size.width) / 2.f, static_cast<float>(event.size.height) / 2.f);
            _window.setView(view);

            // Update all positions when window is resized
            updatePlayerPosition();
            updateEnemyPositions();
            _gameOverMenu.onWindowResize();
        }

        // Handle weapon switching
        if (!_gameOver && event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Num1:
                    changePlayerWeaponToSingle();
                    break;
                case sf::Keyboard::Num2:
                    changePlayerWeaponToRapid();
                    break;
                case sf::Keyboard::Num3:
                    changePlayerWeaponToBurst();
                    break;
                case sf::Keyboard::Num4:
                    changePlayerWeaponToSpread();
                    break;
                default:
                    break;
            }
        }

        if (_gameOver) {
            MenuAction action = _gameOverMenu.handleEvents(event);
            switch (action) {
                case MenuAction::REPLAY:
                    resetGame();
                    break;
                case MenuAction::QUIT:
                    running = false;
                    _shouldExit = true;
                    break;
                case MenuAction::NONE:
                    break;
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
    systems::input_system(_registry, inputs);

    // AI input system for enemies - always runs
    auto& ai_inputs = _registry.get_components<component::ai_input>();
    systems::ai_input_system(_registry, ai_inputs, dt);

    // Only allow player control if game is not over
    if (!_gameOver) {
        systems::control_system(_registry, controllables, velocities, inputs, dt);
    }

    // These systems should continue running even during game over
    systems::position_system(_registry, positions, velocities, inputs, _window, _gameTime, dt);

    // Update projectile lifetimes and cleanup
    systems::projectile_system(_registry, projectiles, positions, dt);

    // Update player relative position and constrain to screen bounds
    if (_player && !_gameOver) {
        auto& player_pos = positions[*_player];
        if (player_pos) {
            sf::Vector2u window_size = _window.getSize();

            // Constrain player to screen bounds
            player_pos->x = std::max(0.f, std::min(player_pos->x, static_cast<float>(window_size.x) - 40.f));
            player_pos->y = std::max(0.f, std::min(player_pos->y, static_cast<float>(window_size.y) - 40.f));

            // Update relative position
            _playerRelativeX = player_pos->x / static_cast<float>(window_size.x);
            _playerRelativeY = player_pos->y / static_cast<float>(window_size.y);
        }
    }

    // Collision system only runs if game is not over (no need to detect collisions when dead)
    if (!_gameOver) {
        auto& hitboxes = _registry.get_components<component::hitbox>();
        systems::collision_system(_registry, positions, drawables, projectiles, hitboxes);
    }

    // Score system - only update if game is not over
    if (!_gameOver) {
        auto& scores = _registry.get_components<component::score>();
        systems::score_system(_registry, scores, dt);
    }

    // Audio system
    auto& sound_effects = _registry.get_components<component::sound_effect>();
    auto& musics = _registry.get_components<component::music>();
    auto& triggers = _registry.get_components<component::audio_trigger>();
    systems::audio_system(_registry, sound_effects, musics, triggers);

    // Check if player is still alive
    if (!_gameOver && !isPlayerAlive()) {
        _gameOver = true;
        _gameOverMenu.setVisible(true);
        _audioManager.loadMusic(MusicType::GAME_OVER, "assets/audio/game_over.ogg");
        _audioManager.playMusic(MusicType::GAME_OVER, false);
    }

    // Spawn enemies - only if game is not over
    if (!_gameOver) {
        _enemySpawnTimer += dt;
        if (_enemySpawnTimer >= _enemySpawnInterval) {
            _enemySpawnTimer = 0.f;
            auto enemy = _registry.spawn_entity();
            sf::Vector2u window_size = _window.getSize();
            float relative_spawn_y = 0.16f + (rand() % 68) / 100.0f; // 0.16 to 0.84 (relative to screen height)
            float spawn_y = getRelativeY(relative_spawn_y);
            float spawn_x = static_cast<float>(window_size.x);
            _registry.add_component<component::position>(enemy, component::position(spawn_x, spawn_y));
            _registry.add_component<component::velocity>(enemy, component::velocity(0.f, 0.f)); // Velocity will be set by AI pattern

            // Randomly choose enemy weapon configuration using function pointer array
            int weapon_type = rand() % NUM_ENEMY_WEAPON_TYPES;

            // Different visuals for different enemy types
            if (weapon_type == 3) { // Zigzag enemy - use different spritesheet with larger scale
                _registry.add_component<component::drawable>(enemy, component::drawable("assets/sprites/r-typesheet3.gif", sf::IntRect(), 3.0f, "enemy_zigzag"));
            } else { // Wave enemies - use original spritesheet
                _registry.add_component<component::drawable>(enemy, component::drawable("assets/sprites/r-typesheet9.gif", sf::IntRect(), 1.0f, "enemy"));
            }
            component::weapon enemy_weapon_config = (this->*enemyWeaponCreators[weapon_type])();
            _registry.add_component<component::weapon>(enemy, std::move(enemy_weapon_config));

            // Different hitboxes for different enemy types
            if (weapon_type == 3) { // Zigzag enemy - enhanced hitbox for better collision detection (17x18 * 3.0 scale + padding)
                _registry.add_component<component::hitbox>(enemy, component::hitbox(65.0f, 70.0f, 0.0f, 00.0f));
            } else { // Wave enemies - original hitbox
                _registry.add_component<component::hitbox>(enemy, component::hitbox(50.0f, 58.0f, 0.0f, 00.0f));
            }

            // Add AI input component for enemy automatic firing with movement pattern
            float fire_interval = 1.0f + (rand() % 100) / 100.0f; // Random interval between 1.0 and 2.0 seconds

            // Choose between wave and zigzag enemies based on weapon type
            component::ai_movement_pattern movement_pattern;
            if (weapon_type == 3) { // Zigzag enemy with spread weapon
                movement_pattern = component::ai_movement_pattern::zigzag(60.0f, 0.015f, 130.0f);
            } else { // Wave enemies with other weapons
                movement_pattern = component::ai_movement_pattern::wave(50.0f, 0.01f, 120.0f);
            }

            _registry.add_component<component::ai_input>(enemy, component::ai_input(true, fire_interval, movement_pattern));

            // Add enemy animation frames - different for each enemy type
            auto& anim = _registry.add_component<component::animation>(enemy, component::animation(0.5f, true));
            if (weapon_type == 3) { // Zigzag enemy - 12 frame animation from spritesheet3 (205x18 pixels, 12 frames)
                // 12 frames arranged in a row, each frame is 17x18 pixels
                anim->frames.push_back(sf::IntRect(0, 0, 17, 18));      // Frame 1
                anim->frames.push_back(sf::IntRect(17, 0, 17, 18));     // Frame 2
                anim->frames.push_back(sf::IntRect(34, 0, 17, 18));     // Frame 3
                anim->frames.push_back(sf::IntRect(51, 0, 17, 18));     // Frame 4
                anim->frames.push_back(sf::IntRect(68, 0, 17, 18));     // Frame 5
                anim->frames.push_back(sf::IntRect(85, 0, 17, 18));     // Frame 6
                anim->frames.push_back(sf::IntRect(102, 0, 17, 18));    // Frame 7
                anim->frames.push_back(sf::IntRect(119, 0, 17, 18));    // Frame 8
                anim->frames.push_back(sf::IntRect(136, 0, 17, 18));    // Frame 9
                anim->frames.push_back(sf::IntRect(153, 0, 17, 18));    // Frame 10
                anim->frames.push_back(sf::IntRect(170, 0, 17, 18));    // Frame 11
                anim->frames.push_back(sf::IntRect(187, 0, 17, 18));    // Frame 12
            } else { // Wave enemies - original animation frames
                anim->frames.push_back(sf::IntRect(0, 0, 50, 58));     // Frame 1: 0 to 50 pixels wide
                anim->frames.push_back(sf::IntRect(51, 0, 57, 58));    // Frame 2: 51 to 108 pixels wide (57 pixels)
                anim->frames.push_back(sf::IntRect(116, 0, 49, 58));   // Frame 3: 116 to 165 pixels wide (49 pixels)
            }

            _enemies.push_back(enemy);
        }
    }

    // Remove enemies that are off-screen
    for (auto it = _enemies.begin(); it != _enemies.end();) {
        auto& pos = _registry.get_components<component::position>()[*it];
        if (pos && pos->x < -50.f) {
            _registry.kill_entity(*it);
            it = _enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::render(float dt) {
    _window.clear();

    // Always draw background and entities, even during game over
    auto& positions = _registry.get_components<component::position>();
    auto& drawables = _registry.get_components<component::drawable>();
    systems::draw_system(_registry, positions, drawables, _window, dt);

    // Draw score in top-right corner
    if (_player) {
        auto& scores = _registry.get_components<component::score>();
        if (*_player < scores.size() && scores[*_player]) {
            auto& player_score = scores[*_player];

            // Create score text
            sf::Text score_text;
            score_text.setFont(_scoreFont);
            score_text.setString("Score " + std::to_string(player_score->current_score));
            score_text.setCharacterSize(24);
            score_text.setFillColor(sf::Color::White);

            // Position in top-right corner
            sf::Vector2u window_size = _window.getSize();
            sf::FloatRect text_bounds = score_text.getLocalBounds();
            score_text.setPosition(window_size.x - text_bounds.width - 20, 20);

            _window.draw(score_text);
        }
    }

#ifdef DEBUG_MODE
    // Draw debug hitboxes when built with debug flags
    {
        auto& hitboxes = _registry.get_components<component::hitbox>();

        for (size_t i = 0; i < positions.size() && i < hitboxes.size(); ++i) {
            if (positions[i] && hitboxes[i]) {
                sf::RectangleShape hitbox_outline;
                hitbox_outline.setSize(sf::Vector2f(hitboxes[i]->width, hitboxes[i]->height));
                hitbox_outline.setPosition(
                    positions[i]->x + hitboxes[i]->offset_x,
                    positions[i]->y + hitboxes[i]->offset_y
                );
                hitbox_outline.setFillColor(sf::Color::Transparent);

                // Different colors for different entity types
                if (drawables[i] && drawables[i]->tag == "player") {
                    hitbox_outline.setOutlineColor(sf::Color::Green);
                } else if (drawables[i] && (drawables[i]->tag == "enemy" || drawables[i]->tag == "enemy_zigzag")) {
                    hitbox_outline.setOutlineColor(sf::Color::Red);
                } else {
                    hitbox_outline.setOutlineColor(sf::Color::Yellow);
                }
                hitbox_outline.setOutlineThickness(2.0f);

                _window.draw(hitbox_outline);
            }
        }
    }
#endif

    // Draw game over menu if visible (it will overlay the game)
    _gameOverMenu.render();

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

    auto render_func = [this](double interpolation) -> void {
        render(static_cast<float>(_tickSystem.getTickDelta()));
    };

    _tickSystem.run(should_continue, update_func, render_func);
}


bool Game::isPlayerAlive() const {
    if (!_player) return false;

    auto& positions = _registry.get_components<component::position>();
    auto& drawables = _registry.get_components<component::drawable>();

    return positions[*_player] && drawables[*_player];
}

void Game::resetGame() {
    _gameOver = false;
    _gameOverMenu.setVisible(false);
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
    if (!bg_component->texture.loadFromFile("assets/background.jpg")) {
        sf::Image fallback_image;
        fallback_image.create(800, 600, sf::Color(20, 20, 80));
        bg_component->texture.loadFromImage(fallback_image);
    }

    // Recreate player
    _player = _registry.spawn_entity();
    _registry.add_component<component::position>(*_player, component::position(getRelativeX(_playerRelativeX), getRelativeY(_playerRelativeY)));
    _registry.add_component<component::velocity>(*_player, component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(*_player, component::drawable("assets/sprites/r-typesheet42.gif", sf::IntRect(0, 0, 33, 17), 2.0f, "player"));
    _registry.add_component<component::controllable>(*_player, component::controllable(_playerSpeed));
    // Arme RAPID par défaut lors du reset
    _registry.add_component<component::weapon>(*_player,
        component::weapon(8.0f, true, 1, 0.0f, component::projectile_pattern::straight(),
                         10.0f, 600.0f, 5.0f, false, 1, false, 3, 0.1f,
                         sf::IntRect(60, 353, 12, 12)));
    _registry.add_component<component::hitbox>(*_player, component::hitbox(66.0f, 34.0f, 15.0f, 0.0f)); // Hitbox du joueur avec offset à droite
    _registry.add_component<component::input>(*_player, component::input()); // Composant input du joueur
    _registry.add_component<component::score>(*_player, component::score(0)); // Composant score du joueur

    // Add player animation with 5 phases
    auto& player_anim = _registry.add_component<component::animation>(*_player, component::animation(0.2f, true));
    player_anim->frames.push_back(sf::IntRect(0, 0, 33, 17));     // Phase 1: neutral up
    player_anim->frames.push_back(sf::IntRect(33, 0, 33, 17));    // Phase 2: slight up
    player_anim->frames.push_back(sf::IntRect(66, 0, 33, 17));    // Phase 3: middle/neutral
    player_anim->frames.push_back(sf::IntRect(99, 0, 33, 17));    // Phase 4: slight down
    player_anim->frames.push_back(sf::IntRect(132, 0, 33, 17));   // Phase 5: full down
    player_anim->current_frame = 2; // Start at middle frame
    player_anim->playing = false; // Start with animation stopped
}

float Game::getRelativeX(float relativeX) const {
    return relativeX * static_cast<float>(_window.getSize().x);
}

float Game::getRelativeY(float relativeY) const {
    return relativeY * static_cast<float>(_window.getSize().y);
}

void Game::updatePlayerPosition() {
    if (_player) {
        auto& positions = _registry.get_components<component::position>();
        auto& player_pos = positions[*_player];
        if (player_pos) {
            player_pos->x = getRelativeX(_playerRelativeX);
            player_pos->y = getRelativeY(_playerRelativeY);
        }
    }
}

void Game::updateEnemyPositions() {
    auto& positions = _registry.get_components<component::position>();
    sf::Vector2u window_size = _window.getSize();

    for (const auto& enemy : _enemies) {
        auto& enemy_pos = positions[enemy];
        if (enemy_pos) {
            // Keep relative position for Y, but update X if it's at the spawn edge
            if (enemy_pos->x >= static_cast<float>(window_size.x) - 50.f) {
                enemy_pos->x = static_cast<float>(window_size.x);
            }
        }
    }
}

void Game::changePlayerWeaponToSingle() {
    if (!_player) return;
    auto& weapons = _registry.get_components<component::weapon>();
    auto& player_weapon = weapons[*_player];
    if (!player_weapon) return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(2.0f, true, 1, 0.0f, component::projectile_pattern::straight(),
                                     20.0f, 500.0f, 5.0f, false, 1, false, 3, 0.1f,
                                     sf::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void Game::changePlayerWeaponToRapid() {
    if (!_player) return;
    auto& weapons = _registry.get_components<component::weapon>();
    auto& player_weapon = weapons[*_player];
    if (!player_weapon) return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(8.0f, true, 1, 0.0f, component::projectile_pattern::straight(),
                                     10.0f, 600.0f, 5.0f, false, 1, false, 3, 0.1f,
                                     sf::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void Game::changePlayerWeaponToBurst() {
    if (!_player) return;
    auto& weapons = _registry.get_components<component::weapon>();
    auto& player_weapon = weapons[*_player];
    if (!player_weapon) return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(2.0f, true, 1, 0.0f, component::projectile_pattern::straight(),
                                     15.0f, 550.0f, 5.0f, false, 1, true, 3, 0.1f,
                                     sf::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void Game::changePlayerWeaponToSpread() {
    if (!_player) return;
    auto& weapons = _registry.get_components<component::weapon>();
    auto& player_weapon = weapons[*_player];
    if (!player_weapon) return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(1.5f, true, 5, 12.0f, component::projectile_pattern::straight(),
                                     12.0f, 450.0f, 5.0f, false, 1, false, 3, 0.1f,
                                     sf::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

component::weapon Game::createEnemySingleWeapon() {
    return component::weapon(1.0f, false, 1, 0.0f,
        component::projectile_pattern::straight(), 8.0f, 200.0f, 5.0f, false, 1,
        false, 3, 0.1f, sf::IntRect(249, 103, 16, 12));
}

component::weapon Game::createEnemyBurstWeapon() {
    return component::weapon(1.5f, false, 1, 0.0f,
        component::projectile_pattern::straight(), 8.0f, 300.0f, 5.0f, false, 1,
        true, 4, 0.15f, sf::IntRect(249, 103, 16, 12));
}

component::weapon Game::createEnemySpreadWeapon() {
    return component::weapon(0.8f, false, 3, 20.0f,
        component::projectile_pattern::straight(), 6.0f, 180.0f, 5.0f, false, 1,
        false, 3, 0.1f, sf::IntRect(249, 103, 16, 12));
}

component::weapon Game::createEnemyZigzagSpreadWeapon() {
    return component::weapon(1.2f, false, 5, 25.0f,
        component::projectile_pattern::spread(25.0f), 10.0f, 220.0f, 4.0f, false, 1,
        false, 3, 0.1f, sf::IntRect(249, 115, 16, 12));
}
