/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_player - Player creation and management
*/

#include "mario_game.hpp"

void MarioGame::createPlayer() {
    _player = _registry.spawn_entity();

    render::Vector2u window_size = _window.getSize();
    float scale_x = static_cast<float>(window_size.x) / 256.0f;
    float scale_y = static_cast<float>(window_size.y) / 224.0f;

    float start_x = 50.0f * scale_x;
    float start_y = 100.0f * scale_y;

    _registry.add_component<component::position>(
        *_player, component::position(start_x, start_y));

    _registry.add_component<component::velocity>(
        *_player, component::velocity(0.0f, 0.0f));

    _registry.add_component<component::drawable>(
        *_player,
        component::drawable("assets/mario/sprites/mario.png",
                            render::IntRect(90, 11, 17, 21),
                            2.0f, "player"));

    float hitbox_width = 17.0f * 2.0f;
    float hitbox_height = 21.0f * 2.0f;
    _registry.add_component<component::hitbox>(
        *_player, component::hitbox(hitbox_width, hitbox_height, 0.0f, 0.0f));

    _registry.add_component<component::gravity>(
        *_player, component::gravity(1500.0f, 1000.0f, 800.0f));

    _registry.add_component<component::input>(*_player, component::input());

    _registry.add_component<component::controllable>(
        *_player, component::controllable(300.0f));
}
