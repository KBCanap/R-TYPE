/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** powerup_manager
*/

#include "../include/powerup_manager.hpp"

PowerupManager::PowerupManager(registry &reg, render::IRenderWindow &win)
    : _registry(reg), _window(win) {}

float PowerupManager::getRelativeX(float relative) const {
    return relative * _window.getSize().x;
}

float PowerupManager::getRelativeY(float relative) const {
    return relative * _window.getSize().y;
}

entity PowerupManager::spawnShieldPowerup(float x, float y) {
    auto powerup = _registry.spawn_entity();

    _registry.add_component<component::position>(
        powerup, component::position(x, y));

    _registry.add_component<component::velocity>(
        powerup, component::velocity(-100.0f, 0.0f));

    // Animated sprite from r-typesheet2.gif
    // Height: Y 34-51 (17px), 12 frames with variable widths
    _registry.add_component<component::drawable>(
        powerup,
        component::drawable("assets/sprites/r-typesheet2.gif",
                            render::IntRect(159, 34, 19, 17), 2.0f, "powerup"));

    // Add animation component with 12 frames (slower animation: 0.2s per frame)
    auto &anim = _registry.add_component<component::animation>(
        powerup, component::animation(0.2f, true));

    // Add all 12 frames with their exact coordinates and widths
    anim->frames.push_back(render::IntRect(159, 34, 19, 17)); // Frame 1: 159-178 (width 19)
    anim->frames.push_back(render::IntRect(182, 34, 21, 17)); // Frame 2: 182-203 (width 21)
    anim->frames.push_back(render::IntRect(207, 34, 19, 17)); // Frame 3: 207-226 (width 19)
    anim->frames.push_back(render::IntRect(230, 34, 21, 17)); // Frame 4: 230-251 (width 21)
    anim->frames.push_back(render::IntRect(254, 34, 21, 17)); // Frame 5: 254-275 (width 21)
    anim->frames.push_back(render::IntRect(279, 34, 19, 17)); // Frame 6: 279-298 (width 19)
    anim->frames.push_back(render::IntRect(301, 34, 19, 17)); // Frame 7: 301-320 (width 19)
    anim->frames.push_back(render::IntRect(324, 34, 21, 17)); // Frame 8: 324-345 (width 21)
    anim->frames.push_back(render::IntRect(348, 34, 21, 17)); // Frame 9: 348-369 (width 21)
    anim->frames.push_back(render::IntRect(373, 34, 19, 17)); // Frame 10: 373-392 (width 19)
    anim->frames.push_back(render::IntRect(396, 34, 21, 17)); // Frame 11: 396-417 (width 21)
    anim->frames.push_back(render::IntRect(421, 34, 19, 17)); // Frame 12: 421-440 (width 19)

    _registry.add_component<component::hitbox>(
        powerup, component::hitbox(42.0f, 34.0f, 0.0f, 0.0f)); // Max width 21*2 scale, 17*2 scale

    return powerup;
}

entity PowerupManager::spawnSpreadPowerup(float x, float y) {
    auto powerup = _registry.spawn_entity();

    _registry.add_component<component::position>(
        powerup, component::position(x, y));

    _registry.add_component<component::velocity>(
        powerup, component::velocity(-100.0f, 0.0f));

    // Animated sprite from r-typesheet2.gif
    // Y: 68-91 (height 23px), X: 119-480, 12 frames separated by 2px
    // First frame width calculation: assume ~28-29px per frame
    _registry.add_component<component::drawable>(
        powerup,
        component::drawable("assets/sprites/r-typesheet2.gif",
                            render::IntRect(119, 68, 28, 23), 2.0f, "spread_powerup"));

    // Add animation component with 12 frames
    auto &anim = _registry.add_component<component::animation>(
        powerup, component::animation(0.1f, true));

    // Add all 12 frames - each frame is ~28-29px wide with 2px separation
    int frame_height = 23;
    int start_y = 68;
    int start_x = 119;
    int frame_width = 28;
    int separation = 2;

    for (int i = 0; i < 12; ++i) {
        int x_pos = start_x + i * (frame_width + separation);
        anim->frames.push_back(render::IntRect(x_pos, start_y, frame_width, frame_height));
    }

    _registry.add_component<component::hitbox>(
        powerup, component::hitbox(56.0f, 46.0f, 0.0f, 0.0f)); // 28*2 scale, 23*2 scale

    return powerup;
}
