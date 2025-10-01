#include "../include/enemy_manager.hpp"
#include <cstdlib>
#include <cmath>

EnemyManager::EnemyManager(registry& reg, sf::RenderWindow& win)
    : _registry(reg), _window(win) {

    // Initialize enemy weapon creators array
    enemyWeaponCreators[0] = &EnemyManager::createEnemySingleWeapon;
    enemyWeaponCreators[1] = &EnemyManager::createEnemyBurstWeapon;
    enemyWeaponCreators[2] = &EnemyManager::createEnemySpreadWeapon;
}

void EnemyManager::spawnEnemy() {
    auto enemy = _registry.spawn_entity();
    sf::Vector2u window_size = _window.getSize();
    float relative_spawn_y = 0.16f + (rand() % 68) / 100.0f; // 0.16 to 0.84 (relative to screen height)
    float spawn_y = relative_spawn_y * static_cast<float>(window_size.y);
    float spawn_x = static_cast<float>(window_size.x);

    _registry.add_component<component::position>(enemy, component::position(spawn_x, spawn_y));
    _registry.add_component<component::velocity>(enemy, component::velocity(0.f, 0.f)); // Velocity will be set by AI pattern

    // Randomly choose enemy weapon configuration using function pointer array
    int weapon_type = rand() % NUM_ENEMY_WEAPON_TYPES;

    // Different visuals for different enemy types
    if (weapon_type == 2) { // Spread enemy - use different spritesheet
        _registry.add_component<component::drawable>(enemy, component::drawable("assets/sprites/r-typesheet3.gif", sf::IntRect(), 3.0f, "enemy_spread"));
    } else { // Single/Burst enemies - use original spritesheet
        _registry.add_component<component::drawable>(enemy, component::drawable("assets/sprites/r-typesheet9.gif", sf::IntRect(), 1.0f, "enemy"));
    }

    component::weapon enemy_weapon_config = (this->*enemyWeaponCreators[weapon_type])();
    _registry.add_component<component::weapon>(enemy, std::move(enemy_weapon_config));

    // Different hitboxes for different enemy types
    if (weapon_type == 2) { // Spread enemy - 17x18 sprite * 3.0 scale
        _registry.add_component<component::hitbox>(enemy, component::hitbox(51.0f, 54.0f, 0.0f, 0.0f));
    } else { // Single/Burst enemies
        _registry.add_component<component::hitbox>(enemy, component::hitbox(50.0f, 58.0f, 0.0f, 0.0f));
    }

    _registry.add_component<component::health>(enemy, component::health(25)); // Enemies have 25 HP

    // Add AI input component for enemy automatic firing with movement pattern
    float fire_interval = 1.0f + (rand() % 100) / 100.0f; // Random interval between 1.0 and 2.0 seconds

    // Different movement patterns
    component::ai_movement_pattern movement_pattern;
    if (weapon_type == 2) { // Spread enemy - zigzag movement
        movement_pattern = component::ai_movement_pattern::zigzag(60.0f, 0.015f, 130.0f);
    } else { // Single/Burst enemies - wave movement
        movement_pattern = component::ai_movement_pattern::wave(50.0f, 0.01f, 120.0f);
    }

    _registry.add_component<component::ai_input>(enemy, component::ai_input(true, fire_interval, movement_pattern));

    // Add enemy animation frames - different for each enemy type
    auto& anim = _registry.add_component<component::animation>(enemy, component::animation(0.5f, true));
    if (weapon_type == 2) { // Spread enemy - 12 frame animation from spritesheet3
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
    } else { // Single/Burst enemies - original animation
        anim->frames.push_back(sf::IntRect(0, 0, 50, 58));     // Frame 1
        anim->frames.push_back(sf::IntRect(51, 0, 57, 58));    // Frame 2
        anim->frames.push_back(sf::IntRect(116, 0, 49, 58));   // Frame 3
    }

    _tempEnemies.push_back(enemy);
}

void EnemyManager::updateEnemyPositions(std::vector<entity>& enemies) {
    auto& positions = _registry.get_components<component::position>();
    sf::Vector2u window_size = _window.getSize();

    for (const auto& enemy : enemies) {
        auto& enemy_pos = positions[enemy];
        if (enemy_pos) {
            // Keep relative position for Y, but update X if it's at the spawn edge
            if (enemy_pos->x >= static_cast<float>(window_size.x) - 50.f) {
                enemy_pos->x = static_cast<float>(window_size.x);
            }
        }
    }
}

void EnemyManager::cleanupOffscreenEnemies(std::vector<entity>& enemies) {
    auto& positions = _registry.get_components<component::position>();

    for (auto it = enemies.begin(); it != enemies.end();) {
        auto& pos = positions[*it];
        if (pos && pos->x < -50.f) {
            _registry.kill_entity(*it);
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }

    // Also cleanup temp enemies
    for (auto it = _tempEnemies.begin(); it != _tempEnemies.end();) {
        auto& pos = positions[*it];
        if (pos && pos->x < -50.f) {
            _registry.kill_entity(*it);
            it = _tempEnemies.erase(it);
        } else {
            ++it;
        }
    }
}

component::weapon EnemyManager::createEnemySingleWeapon() {
    return component::weapon(1.0f, false, 1, 0.0f,
        component::projectile_pattern::straight(), 25.0f, 200.0f, 5.0f, false, 1,
        false, 3, 0.1f, sf::IntRect(249, 103, 16, 12));
}

component::weapon EnemyManager::createEnemyBurstWeapon() {
    return component::weapon(1.5f, false, 1, 0.0f,
        component::projectile_pattern::straight(), 25.0f, 300.0f, 5.0f, false, 1,
        true, 4, 0.15f, sf::IntRect(249, 103, 16, 12));
}

component::weapon EnemyManager::createEnemySpreadWeapon() {
    return component::weapon(0.8f, false, 3, 20.0f,
        component::projectile_pattern::straight(), 25.0f, 180.0f, 5.0f, false, 1,
        false, 3, 0.1f, sf::IntRect(249, 103, 16, 12));
}