/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** powerup_manager
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "components.hpp"
#include "registery.hpp"

class PowerupManager {
  public:
    PowerupManager(registry &reg, render::IRenderWindow &win);

    entity spawnShieldPowerup(float x, float y);
    entity spawnSpreadPowerup(float x, float y);

  private:
    registry &_registry;
    render::IRenderWindow &_window;

    float getRelativeX(float relative) const;
    float getRelativeY(float relative) const;
};
