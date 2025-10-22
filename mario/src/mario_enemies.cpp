/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_enemies - Enemy spawning and management
*/

#include "mario_game.hpp"
#include <iostream>
#include <random>

void MarioGame::spawnEnemy(float x, float y, bool moving_right) {
    entity enemy = _registry.spawn_entity();

    // Get window size for scaling
    render::Vector2u window_size = _window.getSize();
    float scale_x = static_cast<float>(window_size.x) / 256.0f;
    float scale_y = static_cast<float>(window_size.y) / 224.0f;

    // Convert from original coordinates to scaled coordinates
    float spawn_x = x * scale_x;
    float spawn_y = y * scale_y;

    // Enemy size (square for now)
    float enemy_size = 20.0f;

    // Position
    _registry.add_component<component::position>(
        enemy, component::position(spawn_x, spawn_y));

    // Velocity
    _registry.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    // Drawable - red square with "enemy" tag
    auto &drawable = _registry.add_component<component::drawable>(
        enemy, component::drawable(render::Color(255, 0, 0), enemy_size));
    drawable->tag = "enemy";

    // Hitbox
    _registry.add_component<component::hitbox>(
        enemy, component::hitbox(enemy_size, enemy_size, 0.0f, 0.0f));

    // Gravity - same as player but no jump
    _registry.add_component<component::gravity>(
        enemy, component::gravity(1500.0f, 1000.0f, 0.0f));

    // AI input - use straight movement pattern with direction (half speed)
    float base_speed = moving_right ? 75.0f : -75.0f;
    auto movement_pattern = component::ai_movement_pattern::straight(base_speed);
    _registry.add_component<component::ai_input>(
        enemy, component::ai_input(false, 0.0f, movement_pattern));

    // Enemy stunned component (initially not stunned)
    _registry.add_component<component::enemy_stunned>(
        enemy, component::enemy_stunned(false, 0.0f));

    std::cout << "[MarioGame] Spawned enemy at (" << spawn_x << ", " << spawn_y
              << ") moving " << (moving_right ? "right" : "left") << std::endl;
}

void MarioGame::spawnEnemies() {
    // Initialize enemy spawning - enemies will spawn over time
    std::cout << "[MarioGame] Enemy spawning initialized - "
              << _totalEnemiesToSpawn << " enemies will spawn with "
              << _enemySpawnInterval << "s interval" << std::endl;
}

void MarioGame::updateEnemySpawning(float dt) {
    // Check if we still have enemies to spawn
    if (_enemiesSpawned >= _totalEnemiesToSpawn) {
        return;
    }

    // Update spawn timer
    _enemySpawnTimer += dt;

    // Check if it's time to spawn an enemy
    if (_enemySpawnTimer >= _enemySpawnInterval) {
        _enemySpawnTimer = 0.0f;

        // Spawn positions in original coordinates (256x224)
        float left_spawn_x = 32.0f;
        float right_spawn_x = 223.0f;
        float spawn_y = 21.0f;

        // Random distribution for choosing spawn position
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        bool spawn_left = dis(gen) == 0;

        if (spawn_left) {
            // Spawn on left, moving right
            spawnEnemy(left_spawn_x, spawn_y, true);
        } else {
            // Spawn on right, moving left
            spawnEnemy(right_spawn_x, spawn_y, false);
        }

        _enemiesSpawned++;

        std::cout << "[MarioGame] Enemy " << _enemiesSpawned << "/"
                  << _totalEnemiesToSpawn << " spawned" << std::endl;
    }
}
