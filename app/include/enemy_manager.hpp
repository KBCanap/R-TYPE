/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** enemy_manager
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "components.hpp"
#include "registery.hpp"
#include <vector>

class EnemyManager {
  public:
    EnemyManager(registry &reg, render::IRenderWindow &win);

    entity spawnEnemy();
    void updateEnemyPositions(std::vector<entity> &enemies);
    void cleanupOffscreenEnemies(std::vector<entity> &enemies);

    component::weapon createEnemySingleWeapon();
    component::weapon createEnemyBurstWeapon();
    component::weapon createEnemySpreadWeapon();

    static constexpr int NUM_ENEMY_WEAPON_TYPES = 3;

    typedef component::weapon (EnemyManager::*WeaponCreatorFunc)();
    WeaponCreatorFunc enemyWeaponCreators[NUM_ENEMY_WEAPON_TYPES];

  private:
    registry &_registry;
    render::IRenderWindow &_window;
};