/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_game_core - Core game loop and initialization
*/

#include "mario_game.hpp"
#include "systems.hpp"
#include <chrono>
#include <iostream>

MarioGame::MarioGame(registry &reg, render::IRenderWindow &win,
                     AudioManager &audioMgr)
    : _registry(reg), _window(win), _audioManager(audioMgr) {

    // Register all necessary components
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

    // Create background
    _background = _registry.spawn_entity();
    auto &bg_component = _registry.add_component<component::background>(
        *_background, component::background(0.0f)); // No scrolling for Mario
    bg_component->texture = _window.createTexture();
    if (!bg_component->texture->loadFromFile(
            "assets/mario/sprites/background_mario.png")) {
        std::cerr << "[MarioGame] Failed to load Mario background" << std::endl;
        // Create fallback colored background
        auto fallback_image = _window.createImage();
        fallback_image->create(800, 600, render::Color(100, 150, 255));
        bg_component->texture->loadFromImage(*fallback_image);
    }

    _registry.add_component<component::position>(*_background,
                                                  component::position(0.0f, 0.0f));

    // Load debug font
    _debugFont = _window.createFont();
    if (!_debugFont->loadFromFile("assets/r-type.otf")) {
        std::cerr << "[MarioGame] Failed to load font" << std::endl;
    }

    // Create platforms
    createPlatforms();

    // Create player
    createPlayer();

    // Setup player animation
    setupPlayerAnimation();

    // Spawn enemies
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

    // Update enemy spawning over time
    updateEnemySpawning(dt);

    // Handle controls (horizontal movement + jump)
    updateControls(dt);

    // Update player animation based on movement state
    updatePlayerAnimation(dt);

    // Enemy AI system - use existing ai_input_system for enemy movement
    systems::ai_input_system(_registry, ai_inputs, dt);

    // Gravity system - apply gravity first
    systems::gravity_system(_registry, gravities, velocities, dt);

    // Position system - update positions based on velocities
    auto &inputs = _registry.get_components<component::input>();
    systems::position_system(_registry, positions, velocities, inputs, _window,
                             _gameTime, dt);

    // Platform collision system - resolve collisions after movement
    systems::platform_collision_system(_registry, positions, velocities,
                                        gravities, platforms, hitboxes);

    // Collision system - handles Mario stomp, death, and stunned enemy knockback
    systems::collision_system(_registry, positions, drawables, projectiles,
                              hitboxes);
}

void MarioGame::run() {
    auto last_time = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running && _window.isOpen() && !_shouldExit) {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float dt = elapsed.count();
        last_time = current_time;

        // Cap delta time to avoid huge jumps
        if (dt > 0.1f)
            dt = 0.1f;

        handleEvents(running, dt);
        update(dt);
        render();
    }
}
