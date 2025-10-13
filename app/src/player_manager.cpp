/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** player_manager
*/

#include "../include/player_manager.hpp"

PlayerManager::PlayerManager(registry &reg, render::IRenderWindow &win)
    : _registry(reg), _window(win) {}

std::optional<entity>
PlayerManager::createPlayer(float relativeX, float relativeY, float speed) {
    auto player = _registry.spawn_entity();

    _registry.add_component<component::position>(
        player,
        component::position(getRelativeX(relativeX), getRelativeY(relativeY)));
    _registry.add_component<component::velocity>(player,
                                                 component::velocity(0.f, 0.f));
    _registry.add_component<component::drawable>(
        player,
        component::drawable("assets/sprites/r-typesheet42.gif",
                            render::IntRect(0, 0, 33, 17), 2.0f, "player"));
    _registry.add_component<component::controllable>(
        player, component::controllable(speed));

    // Arme RAPID par défaut : cadence élevée, dégâts faibles, projectile unique
    _registry.add_component<component::weapon>(
        player, component::weapon(8.0f, true, 1, 0.0f,
                                  component::projectile_pattern::straight(),
                                  25.0f, 600.0f, 5.0f, false, 1, false, 3, 0.1f,
                                  render::IntRect(60, 353, 12, 12)));

    _registry.add_component<component::hitbox>(
        player, component::hitbox(66.0f, 34.0f, 15.0f, 0.0f));
    _registry.add_component<component::input>(player, component::input());
    _registry.add_component<component::score>(player, component::score(0));
    _registry.add_component<component::health>(player, component::health(100));

    // Add player animation with 5 phases
    auto &player_anim = _registry.add_component<component::animation>(
        player, component::animation(0.2f, true));
    player_anim->frames.push_back(
        render::IntRect(0, 0, 33, 17)); // Phase 1: neutral up
    player_anim->frames.push_back(
        render::IntRect(33, 0, 33, 17)); // Phase 2: slight up
    player_anim->frames.push_back(
        render::IntRect(66, 0, 33, 17)); // Phase 3: middle/neutral
    player_anim->frames.push_back(
        render::IntRect(99, 0, 33, 17)); // Phase 4: slight down
    player_anim->frames.push_back(
        render::IntRect(132, 0, 33, 17)); // Phase 5: full down
    player_anim->current_frame = 2;       // Start at middle frame
    player_anim->playing = false;         // Start with animation stopped

    return player;
}

void PlayerManager::updatePlayerPosition(std::optional<entity> &player,
                                         float relativeX, float relativeY) {
    if (player) {
        auto &positions = _registry.get_components<component::position>();
        auto &player_pos = positions[*player];
        if (player_pos) {
            player_pos->x = getRelativeX(relativeX);
            player_pos->y = getRelativeY(relativeY);
        }
    }
}

bool PlayerManager::isPlayerAlive(const std::optional<entity> &player) const {
    if (!player)
        return false;

    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();

    return positions[*player] && drawables[*player];
}

void PlayerManager::changePlayerWeaponToSingle(
    const std::optional<entity> &player) {
    if (!player)
        return;
    auto &weapons = _registry.get_components<component::weapon>();
    auto &player_weapon = weapons[*player];
    if (!player_weapon)
        return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(
        2.0f, true, 1, 0.0f, component::projectile_pattern::straight(), 25.0f,
        500.0f, 5.0f, false, 1, false, 3, 0.1f,
        render::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void PlayerManager::changePlayerWeaponToRapid(
    const std::optional<entity> &player) {
    if (!player)
        return;
    auto &weapons = _registry.get_components<component::weapon>();
    auto &player_weapon = weapons[*player];
    if (!player_weapon)
        return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(
        8.0f, true, 1, 0.0f, component::projectile_pattern::straight(), 25.0f,
        600.0f, 5.0f, false, 1, false, 3, 0.1f,
        render::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void PlayerManager::changePlayerWeaponToBurst(
    const std::optional<entity> &player) {
    if (!player)
        return;
    auto &weapons = _registry.get_components<component::weapon>();
    auto &player_weapon = weapons[*player];
    if (!player_weapon)
        return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(
        2.0f, true, 1, 0.0f, component::projectile_pattern::straight(), 25.0f,
        550.0f, 5.0f, false, 1, true, 3, 0.1f,
        render::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

void PlayerManager::changePlayerWeaponToSpread(
    const std::optional<entity> &player) {
    if (!player)
        return;
    auto &weapons = _registry.get_components<component::weapon>();
    auto &player_weapon = weapons[*player];
    if (!player_weapon)
        return;

    float last_shot_time = player_weapon->last_shot_time;
    *player_weapon = component::weapon(
        1.5f, true, 5, 12.0f, component::projectile_pattern::straight(), 25.0f,
        450.0f, 5.0f, false, 1, false, 3, 0.1f,
        render::IntRect(60, 353, 12, 12));
    player_weapon->last_shot_time = last_shot_time;
}

float PlayerManager::getRelativeX(float relativeX) const {
    return relativeX * static_cast<float>(_window.getSize().x);
}

float PlayerManager::getRelativeY(float relativeY) const {
    return relativeY * static_cast<float>(_window.getSize().y);
}