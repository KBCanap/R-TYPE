/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** boss_manager
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "components.hpp"
#include "registery.hpp"

class BossManager {
  public:
    BossManager(registry &reg, render::IRenderWindow &win);

    std::optional<entity> spawnBoss();
    std::optional<entity> spawnBossLevel2();
    void spawnBossLevel2Phase2Parts(float boss_x, float boss_y,
                                    std::vector<entity> &boss_parts);
    bool shouldSpawnBoss(const std::optional<entity> &player,
                         const std::optional<entity> &boss) const;

  private:
    registry &_registry;
    render::IRenderWindow &_window;
};