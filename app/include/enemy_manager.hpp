#pragma once
#include "registery.hpp"
#include "components.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

class EnemyManager {
public:
    EnemyManager(registry& reg, sf::RenderWindow& win);

    // Enemy spawning and management
    void spawnEnemy();
    void updateEnemyPositions(std::vector<entity>& enemies);
    void cleanupOffscreenEnemies(std::vector<entity>& enemies);

    // Enemy weapon configurations
    component::weapon createEnemySingleWeapon();
    component::weapon createEnemyBurstWeapon();
    component::weapon createEnemySpreadWeapon();

    // Constants
    static constexpr int NUM_ENEMY_WEAPON_TYPES = 3;

    // Enemy weapon creators function pointer array
    typedef component::weapon (EnemyManager::*WeaponCreatorFunc)();
    WeaponCreatorFunc enemyWeaponCreators[NUM_ENEMY_WEAPON_TYPES];

private:
    registry& _registry;
    sf::RenderWindow& _window;
    std::vector<entity> _tempEnemies; // For managing spawned enemies
};