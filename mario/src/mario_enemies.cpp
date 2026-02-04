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

    render::Vector2u window_size = _window.getSize();
    float scale_x = static_cast<float>(window_size.x) / 256.0f;
    float scale_y = static_cast<float>(window_size.y) / 224.0f;

    float spawn_x = x * scale_x;
    float spawn_y = y * scale_y;

    const int frame_width = 18;
    const int frame_height = 17;
    float sprite_scale = scale_x * 0.625f;

    float hitbox_width = frame_width * sprite_scale;
    float hitbox_height = frame_height * sprite_scale;

    _registry.add_component<component::position>(
        enemy, component::position(spawn_x, spawn_y));

    _registry.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    render::IntRect first_frame(0, 0, frame_width, frame_height);
    auto &drawable = _registry.add_component<component::drawable>(
        enemy, component::drawable("assets/mario/sprites/turtle.png",
                                   first_frame, sprite_scale, "enemy"));
    drawable->flip_x = moving_right;

    auto &anim = _registry.add_component<component::animation>(
        enemy, component::animation(0.15f, true, false));
    anim->frames = {
        render::IntRect(0 * frame_width, 0, frame_width, frame_height),
        render::IntRect(1 * frame_width, 0, frame_width, frame_height),
        render::IntRect(2 * frame_width, 0, frame_width, frame_height),
        render::IntRect(3 * frame_width, 0, frame_width, frame_height),
        render::IntRect(4 * frame_width, 0, frame_width, frame_height),
    };
    anim->playing = true;

    _registry.add_component<component::hitbox>(
        enemy, component::hitbox(hitbox_width, hitbox_height, 0.0f, 0.0f));

    _registry.add_component<component::gravity>(
        enemy, component::gravity(1500.0f, 1000.0f, 0.0f));

    float base_speed = moving_right ? _enemyBaseSpeed : -_enemyBaseSpeed;
    auto movement_pattern = component::ai_movement_pattern::straight(base_speed);
    _registry.add_component<component::ai_input>(
        enemy, component::ai_input(false, 0.0f, movement_pattern));

    _registry.add_component<component::enemy_stunned>(
        enemy, component::enemy_stunned(false, 0.0f));

    playSound("enemy_spawn");
}

void MarioGame::spawnEnemies() {
}

void MarioGame::updateEnemySpawning(float dt) {
    if (_enemiesSpawned >= _totalEnemiesToSpawn)
        return;

    _enemySpawnTimer += dt;

    if (_enemySpawnTimer >= _enemySpawnInterval) {
        _enemySpawnTimer = 0.0f;

        float left_spawn_x = 32.0f;
        float right_spawn_x = 223.0f;
        float spawn_y = 21.0f;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        bool spawn_left = dis(gen) == 0;

        if (spawn_left) {
            spawnEnemy(left_spawn_x, spawn_y, true);
        } else {
            spawnEnemy(right_spawn_x, spawn_y, false);
        }

        _enemiesSpawned++;
    }
}
