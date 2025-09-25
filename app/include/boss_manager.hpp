#pragma once
#include "registery.hpp"
#include "components.hpp"
#include <SFML/Graphics.hpp>

class BossManager {
public:
    BossManager(registry& reg, sf::RenderWindow& win);

    // Boss management
    std::optional<entity> spawnBoss();
    bool shouldSpawnBoss(const std::optional<entity>& player, const std::optional<entity>& boss) const;

private:
    registry& _registry;
    sf::RenderWindow& _window;
};