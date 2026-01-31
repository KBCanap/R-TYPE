/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** boss_manager
*/

#include "../include/boss_manager.hpp"
#include <iostream>

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
        boss, component::velocity(0.f, 150.0f));
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

    // AI for firing only (no movement pattern applied)
    auto boss_ai = _registry.add_component<component::ai_input>(
        boss, component::ai_input(
                  true, 0.5f, component::ai_movement_pattern::straight(0.0f)));
    boss_ai->movement_pattern.base_speed =
        0.0f; // No AI movement, velocity handles it

    auto &boss_anim = _registry.add_component<component::animation>(
        boss, component::animation(0.1f, true));
    for (int i = 0; i < 8; ++i) {
        boss_anim->frames.push_back(render::IntRect(i * 65, 0, 65, 132));
    }

    return boss;
}

std::optional<entity> BossManager::spawnBossLevel2() {
    // Level 2 boss: spawn a dummy entity that will be used for tracking
    // The actual boss is composed of 3 separate parts
    auto boss = _registry.spawn_entity();

    // Don't add any components to the dummy boss entity
    // It's just used as a placeholder to trigger the spawn in game.cpp

    return boss;
}

void BossManager::spawnBossLevel2Phase2Parts(float boss_x, float boss_y,
                                             std::vector<entity> &boss_parts) {
    // Level 2 boss: 3 distinct parts
    // Each part has 1000 HP (3000 HP total)
    // All parts stay centered and oscillate slightly

    std::cout << "[BossManager] Spawning Level 2 boss (3 parts) at x=" << boss_x
              << " y=" << boss_y << std::endl;

    // Part 1 (Left): (24,188) to (140,188), height 69
    entity part1 = _registry.spawn_entity();
    _registry.add_component<component::position>(
        part1, component::position(boss_x - 60.0f, boss_y));
    _registry.add_component<component::velocity>(part1,
                                                 component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(
        part1,
        component::drawable("assets/sprites/r-typesheet38.gif",
                            render::IntRect(24, 188, 116, 69), 1.0f, "boss"));
    _registry.add_component<component::hitbox>(
        part1, component::hitbox(116.0f, 69.0f, 0.0f, 0.0f));
    _registry.add_component<component::health>(part1, component::health(1000));

    // Add AI for slight oscillation movement
    auto part1_ai = _registry.add_component<component::ai_input>(
        part1, component::ai_input(false, 0.0f,
                                   component::ai_movement_pattern::sine_wave(
                                       50.0f, 1.0f, 0.0f)));
    part1_ai->movement_pattern.base_speed = 0.0f;

    // Part 2 (Center): (141,158) to (239,158), height 100
    entity part2 = _registry.spawn_entity();
    _registry.add_component<component::position>(
        part2, component::position(boss_x, boss_y - 15.0f));
    _registry.add_component<component::velocity>(part2,
                                                 component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(
        part2,
        component::drawable("assets/sprites/r-typesheet38.gif",
                            render::IntRect(141, 158, 98, 100), 1.0f, "boss"));
    _registry.add_component<component::hitbox>(
        part2, component::hitbox(98.0f, 100.0f, 0.0f, 0.0f));
    _registry.add_component<component::health>(part2, component::health(1000));

    auto part2_ai = _registry.add_component<component::ai_input>(
        part2, component::ai_input(false, 0.0f,
                                   component::ai_movement_pattern::sine_wave(
                                       60.0f, 1.2f, 0.0f)));
    part2_ai->movement_pattern.base_speed = 0.0f;

    // Part 3 (Right): (240,175) to (339,175), height 83
    entity part3 = _registry.spawn_entity();
    _registry.add_component<component::position>(
        part3, component::position(boss_x + 60.0f, boss_y));
    _registry.add_component<component::velocity>(part3,
                                                 component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(
        part3,
        component::drawable("assets/sprites/r-typesheet38.gif",
                            render::IntRect(240, 175, 99, 83), 1.0f, "boss"));
    _registry.add_component<component::hitbox>(
        part3, component::hitbox(99.0f, 83.0f, 0.0f, 0.0f));
    _registry.add_component<component::health>(part3, component::health(1000));

    // Add AI for slight oscillation movement
    auto part3_ai = _registry.add_component<component::ai_input>(
        part3, component::ai_input(false, 0.0f,
                                   component::ai_movement_pattern::sine_wave(
                                       40.0f, 0.8f, 0.0f)));
    part3_ai->movement_pattern.base_speed = 0.0f;

    boss_parts.push_back(part1);
    boss_parts.push_back(part2);
    boss_parts.push_back(part3);

    std::cout << "[BossManager] Level 2 boss parts spawned: part1=" << part1
              << " (amplitude=" << part1_ai->movement_pattern.amplitude << "), "
              << "part2=" << part2
              << " (amplitude=" << part2_ai->movement_pattern.amplitude << "), "
              << "part3=" << part3
              << " (amplitude=" << part3_ai->movement_pattern.amplitude << ")"
              << std::endl;
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