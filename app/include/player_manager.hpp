/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** player_manager
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "components.hpp"
#include "registery.hpp"

class PlayerManager {
  public:
    PlayerManager(registry &reg, render::IRenderWindow &win);

    std::optional<entity> createPlayer(float relativeX = 0.1f,
                                       float relativeY = 0.5f,
                                       float speed = 500.0f);
    void updatePlayerPosition(std::optional<entity> &player, float relativeX,
                              float relativeY);
    bool isPlayerAlive(const std::optional<entity> &player) const;

    void changePlayerWeaponToSingle(const std::optional<entity> &player);
    void changePlayerWeaponToRapid(const std::optional<entity> &player);
    void changePlayerWeaponToBurst(const std::optional<entity> &player);
    void changePlayerWeaponToSpread(const std::optional<entity> &player);

    void updateShieldVisual(const std::optional<entity> &player,
                            std::optional<entity> &shield_entity);

    float getRelativeX(float relativeX) const;
    float getRelativeY(float relativeY) const;

  private:
    registry &_registry;
    render::IRenderWindow &_window;
};