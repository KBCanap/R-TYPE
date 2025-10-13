/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** boss_manager
*/

#include "../include/boss_manager.hpp"

BossManager::BossManager(registry &reg, render::IRenderWindow &win)
    : _registry(reg), _window(win) {}

std::optional<entity> BossManager::spawnBoss() {
    auto boss = _registry.spawn_entity();
    render::Vector2u window_size = _window.getSize();
    float boss_x = static_cast<float>(window_size.x) - 150.0f;
    float boss_y = static_cast<float>(window_size.y) / 2.0f;

    _registry.add_component<component::position>(
        boss, component::position(boss_x, boss_y));
    _registry.add_component<component::velocity>(
        boss, component::velocity(0.f, 100.0f));
    _registry.add_component<component::drawable>(
        boss, component::drawable("assets/sprites/r-typesheet17.gif",
                                  render::IntRect(), 2.0f, "boss"));
    _registry.add_component<component::hitbox>(
        boss, component::hitbox(130.0f, 220.0f, 0.0f, 0.0f));
    _registry.add_component<component::health>(boss, component::health(1000));

    _registry.add_component<component::weapon>(
        boss, component::weapon(2.0f, false, 5, 15.0f,
                                component::projectile_pattern::straight(),
                                25.0f, 250.0f, 5.0f, false, 1, false, 3, 0.1f,
                                render::IntRect(249, 103, 16, 12)));

    auto boss_ai = _registry.add_component<component::ai_input>(
        boss, component::ai_input(
                  true, 0.5f, component::ai_movement_pattern::straight(0.0f)));
    boss_ai->movement_pattern.base_speed = 0.0f;

    auto &boss_anim = _registry.add_component<component::animation>(
        boss, component::animation(0.1f, true));
    for (int i = 0; i < 8; ++i) {
        boss_anim->frames.push_back(render::IntRect(i * 65, 0, 65, 132));
    }

    return boss;
}

bool BossManager::shouldSpawnBoss(const std::optional<entity> &player,
                                  const std::optional<entity> &boss) const {
    if (boss || !player)
        return false;

    auto &scores = _registry.get_components<component::score>();
    if (*player < scores.size() && scores[*player]) {
        auto &player_score = scores[*player];
        return player_score->current_score >= 100;
    }

    return false;
}