/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** enemy_manager
*/

#include "../include/enemy_manager.hpp"
#include "entity.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sol/sol.hpp>

EnemyManager::EnemyManager(registry &reg, render::IRenderWindow &win)
    : _registry(reg), _window(win) {
    enemyWeaponCreators[0] = &EnemyManager::createEnemySingleWeapon;
    enemyWeaponCreators[1] = &EnemyManager::createEnemyBurstWeapon;
    enemyWeaponCreators[2] = &EnemyManager::createEnemySpreadWeapon;
}

entity EnemyManager::spawnEnemy() {
    static sol::state lua;
    static bool initialized = false;

    if (!initialized) {
        lua.open_libraries(sol::lib::base, sol::lib::math);
        try {
            lua.script_file("scripts/enemy_spawn.lua");
        } catch (const sol::error &e) {
            std::cerr << "[Lua Error] " << e.what() << std::endl;
            return entity(-1);
        }
        initialized = true;
    }

    sol::protected_function get_config = lua["get_enemy_config"];
    sol::protected_function_result result = get_config();

    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[Lua Runtime Error] " << err.what() << std::endl;
        return entity(-1);
    }

    sol::table config = result;
    sol::table movement = config["movement"];
    std::string movement_type = movement["type"];
    float amplitude = movement["amplitude"];
    float frequency = movement["frequency"];
    float speed = movement["speed"];
    std::string weapon_type = config["weapon_type"];

    component::ai_movement_pattern movement_pattern;
    if (movement_type == "wave")
        movement_pattern = component::ai_movement_pattern::wave(amplitude, frequency, speed);
    else if (movement_type == "zigzag")
        movement_pattern = component::ai_movement_pattern::zigzag(amplitude, frequency, speed);
    else if (movement_type == "sine_wave")
        movement_pattern = component::ai_movement_pattern::sine_wave(amplitude, frequency, speed);
    else
        movement_pattern = component::ai_movement_pattern::straight(speed);

    auto enemy = _registry.spawn_entity();
    render::Vector2u window_size = _window.getSize();
    float spawn_y = (0.16f + (rand() % 68) / 100.0f) * static_cast<float>(window_size.y);
    float spawn_x = static_cast<float>(window_size.x);

    component::weapon weapon_config;
        if (weapon_type == "spread")
            weapon_config = createEnemySpreadWeapon();
        else if (weapon_type == "burst")
            weapon_config = createEnemyBurstWeapon();
        else if (weapon_type == "wave")
            weapon_config = createEnemyWaveWeapon();
        else
            weapon_config = createEnemySingleWeapon();

    if (weapon_type == "spread") {
        _registry.add_component<component::drawable>(
            enemy,
            component::drawable("assets/sprites/r-typesheet3.gif",
                                render::IntRect(), 3.0f, "enemy_spread"));
    } else {
        _registry.add_component<component::drawable>(
            enemy, component::drawable("assets/sprites/r-typesheet9.gif",
                                       render::IntRect(), 1.0f, "enemy"));
    }

    _registry.add_component<component::position>(enemy, component::position(spawn_x, spawn_y));
    _registry.add_component<component::velocity>(enemy, component::velocity(0.f, 0.f));
    _registry.add_component<component::weapon>(enemy, std::move(weapon_config));
    _registry.add_component<component::health>(enemy, component::health(25));
    _registry.add_component<component::ai_input>(enemy, component::ai_input(true, 1.0f, movement_pattern));
    auto &anim = _registry.add_component<component::animation>(
        enemy, component::animation(0.5f, true));
    if (weapon_type == "spread") {
        anim->frames.push_back(render::IntRect(0, 0, 17, 18));
        anim->frames.push_back(render::IntRect(17, 0, 17, 18));
        anim->frames.push_back(render::IntRect(34, 0, 17, 18));
        anim->frames.push_back(render::IntRect(51, 0, 17, 18));
        anim->frames.push_back(render::IntRect(68, 0, 17, 18));
        anim->frames.push_back(render::IntRect(85, 0, 17, 18));
        anim->frames.push_back(render::IntRect(102, 0, 17, 18));
        anim->frames.push_back(render::IntRect(119, 0, 17, 18));
        anim->frames.push_back(render::IntRect(136, 0, 17, 18));
        anim->frames.push_back(render::IntRect(153, 0, 17, 18));
        anim->frames.push_back(render::IntRect(170, 0, 17, 18));
        anim->frames.push_back(render::IntRect(187, 0, 17, 18));
    } else {
        anim->frames.push_back(render::IntRect(0, 0, 50, 58));
        anim->frames.push_back(render::IntRect(51, 0, 57, 58));
        anim->frames.push_back(render::IntRect(116, 0, 49, 58));
    }

    return enemy;
}

entity EnemyManager::spawnEnemyLevel2() {
    auto enemy = _registry.spawn_entity();
    render::Vector2u window_size = _window.getSize();
    float relative_spawn_y = 0.16f + (rand() % 68) / 100.0f;
    float spawn_y = relative_spawn_y * static_cast<float>(window_size.y);
    float spawn_x = static_cast<float>(window_size.x);

    _registry.add_component<component::position>(
        enemy, component::position(spawn_x, spawn_y));
    _registry.add_component<component::velocity>(enemy,
                                                 component::velocity(0.f, 0.f));

    // Level 2 enemy sprite from r-typesheet5.gif
    // Frame 1: x=6, y=6 to x=28, y=29
    // Frame dimensions: 22x23 pixels
    // 8 frames with 10px spacing between each frame
    _registry.add_component<component::drawable>(
        enemy,
        component::drawable("assets/sprites/r-typesheet5.gif",
                            render::IntRect(6, 6, 22, 23), 2.0f, "enemy_level2"));

    // Level 2 enemies fire in wave pattern
    component::weapon enemy_weapon_config = createEnemyWaveWeapon();
    _registry.add_component<component::weapon>(enemy,
                                               std::move(enemy_weapon_config));

    // Standard hitbox for level 2 enemies
    _registry.add_component<component::hitbox>(
        enemy, component::hitbox(44.0f, 46.0f, 0.0f, 0.0f));

    _registry.add_component<component::health>(enemy, component::health(25));

    float fire_interval = 1.0f + (rand() % 100) / 100.0f;

    // Level 2 enemies use sine wave movement
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::sine_wave(80.0f, 0.02f, 110.0f);

    _registry.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    // Add animation with 8 frames, 10px spacing between each
    auto &anim = _registry.add_component<component::animation>(
        enemy, component::animation(0.2f, true));

    int frame_width = 22;
    int frame_height = 23;
    int start_x = 6;
    int start_y = 6;
    int spacing = 10;

    for (int i = 0; i < 8; ++i) {
        int x_pos = start_x + i * (frame_width + spacing);
        anim->frames.push_back(render::IntRect(x_pos, start_y, frame_width, frame_height));
    }

    return enemy;
}

entity EnemyManager::spawnEnemyLevel2Spread() {
    auto enemy = _registry.spawn_entity();
    render::Vector2u window_size = _window.getSize();
    float relative_spawn_y = 0.16f + (rand() % 68) / 100.0f;
    float spawn_y = relative_spawn_y * static_cast<float>(window_size.y);
    float spawn_x = static_cast<float>(window_size.x);

    _registry.add_component<component::position>(
        enemy, component::position(spawn_x, spawn_y));
    _registry.add_component<component::velocity>(enemy,
                                                 component::velocity(0.f, 0.f));

    // r-typesheet11 sprite: 3 frames, 34x31 pixels each
    // Frame 1: x=0 to x=34
    // Frame 2: x=34 to x=66 (width=32)
    // Frame 3: x=66 to x=99 (width=33)
    _registry.add_component<component::drawable>(
        enemy,
        component::drawable("assets/sprites/r-typesheet11.gif",
                            render::IntRect(0, 0, 34, 31), 2.0f, "enemy_spread_level2"));

    // Create spread weapon (3 projectiles with 20 degree spread)
    component::weapon spread_weapon = createEnemySpreadWeapon();
    _registry.add_component<component::weapon>(enemy, std::move(spread_weapon));

    // Hitbox for r-typesheet11 sprite
    // Using average width of 33px and height of 31px at 2.0 scale
    _registry.add_component<component::hitbox>(
        enemy, component::hitbox(66.0f, 62.0f, 0.0f, 0.0f));

    _registry.add_component<component::health>(enemy, component::health(35));

    float fire_interval = 0.8f + (rand() % 80) / 100.0f;

    // Zigzag movement pattern for more aggressive behavior
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::zigzag(70.0f, 0.018f, 150.0f);

    _registry.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    // Add animation with 3 frames
    // Frame 1: 0-34 (width 34)
    // Frame 2: 34-66 (width 32)
    // Frame 3: 66-99 (width 33)
    // Height: 31 pixels for all frames
    // Very slow animation: 0.8s per frame
    auto &anim = _registry.add_component<component::animation>(
        enemy, component::animation(0.8f, true));

    anim->frames.push_back(render::IntRect(0, 0, 34, 31));
    anim->frames.push_back(render::IntRect(34, 0, 32, 31));
    anim->frames.push_back(render::IntRect(66, 0, 33, 31));

    return enemy;
}

void EnemyManager::updateEnemyPositions(std::vector<entity> &enemies) {
    auto &positions = _registry.get_components<component::position>();
    render::Vector2u window_size = _window.getSize();

    for (const auto &enemy : enemies) {
        auto &enemy_pos = positions[enemy];
        if (enemy_pos) {
            if (enemy_pos->x >= static_cast<float>(window_size.x) - 50.f) {
                enemy_pos->x = static_cast<float>(window_size.x);
            }
        }
    }
}

void EnemyManager::cleanupOffscreenEnemies(std::vector<entity> &enemies) {
    auto &positions = _registry.get_components<component::position>();

    for (auto it = enemies.begin(); it != enemies.end();) {
        auto &pos = positions[*it];
        if (pos && pos->x < -50.f) {
            _registry.kill_entity(*it);
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

component::weapon EnemyManager::createEnemySingleWeapon() {
    return component::weapon(1.0f, false, 1, 0.0f,
                             component::projectile_pattern::straight(), 25.0f,
                             200.0f, 5.0f, false, 1, false, 3, 0.1f,
                             render::IntRect(249, 103, 16, 12));
}

component::weapon EnemyManager::createEnemyBurstWeapon() {
    return component::weapon(1.5f, false, 1, 0.0f,
                             component::projectile_pattern::straight(), 25.0f,
                             300.0f, 5.0f, false, 1, true, 4, 0.15f,
                             render::IntRect(249, 103, 16, 12));
}

component::weapon EnemyManager::createEnemySpreadWeapon() {
    return component::weapon(0.8f, false, 3, 20.0f,
                             component::projectile_pattern::straight(), 25.0f,
                             180.0f, 5.0f, false, 1, false, 3, 0.1f,
                             render::IntRect(249, 103, 16, 12));
}

component::weapon EnemyManager::createEnemyWaveWeapon() {
    return component::weapon(1.2f, false, 1, 0.0f,
                             component::projectile_pattern::wave(60.0f, 0.015f), 25.0f,
                             200.0f, 5.0f, false, 1, false, 3, 0.1f,
                             render::IntRect(249, 103, 16, 12));
}