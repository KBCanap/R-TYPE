#pragma once
#include "registery.hpp"
#include "components.hpp"
#include "../../ecs/include/render/IRenderWindow.hpp"

class BossManager {
public:
    BossManager(registry& reg, render::IRenderWindow& win);

    // Boss management
    std::optional<entity> spawnBoss();
    bool shouldSpawnBoss(const std::optional<entity>& player, const std::optional<entity>& boss) const;

private:
    registry& _registry;
    render::IRenderWindow& _window;
};