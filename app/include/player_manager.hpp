#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "components.hpp"
#include "registery.hpp"

class PlayerManager {
  public:
    PlayerManager(registry &reg, render::IRenderWindow &win);

    // Player creation and management
    std::optional<entity> createPlayer(float relativeX = 0.1f,
                                       float relativeY = 0.5f,
                                       float speed = 500.0f);
    void updatePlayerPosition(std::optional<entity> &player, float relativeX,
                              float relativeY);
    bool isPlayerAlive(const std::optional<entity> &player) const;

    // Weapon management
    void changePlayerWeaponToSingle(const std::optional<entity> &player);
    void changePlayerWeaponToRapid(const std::optional<entity> &player);
    void changePlayerWeaponToBurst(const std::optional<entity> &player);
    void changePlayerWeaponToSpread(const std::optional<entity> &player);

    // Utility functions
    float getRelativeX(float relativeX) const;
    float getRelativeY(float relativeY) const;

  private:
    registry &_registry;
    render::IRenderWindow &_window;
};