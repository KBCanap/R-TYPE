/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_animation - Player animation management
*/

#include "mario_game.hpp"

void MarioGame::setupPlayerAnimation() {
    if (!_player)
        return;

    auto &anim = _registry.add_component<component::animation>(
        *_player, component::animation(0.1f, true, false));

    anim->frames = {
        render::IntRect(0, 11, 17, 21),
        render::IntRect(18, 11, 15, 21),
        render::IntRect(38, 11, 13, 21),
        render::IntRect(55, 11, 15, 21),
        render::IntRect(73, 11, 14, 21),
    };

    anim->current_frame = 0;
    anim->playing = false;
}

void MarioGame::updatePlayerAnimation(float /*dt*/) {
    if (!_player)
        return;

    auto &animations = _registry.get_components<component::animation>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &gravities = _registry.get_components<component::gravity>();
    auto &drawables = _registry.get_components<component::drawable>();

    auto &anim = animations[*_player];
    auto &vel = velocities[*_player];
    auto &grav = gravities[*_player];
    auto &draw = drawables[*_player];

    if (!anim || !vel || !grav || !draw)
        return;

    bool is_moving = (vel->vx != 0.0f);
    bool is_jumping = !grav->on_ground;
    bool facing_right = vel->vx > 0.0f;
    bool facing_left = vel->vx < 0.0f;

    if (facing_right) {
        _playerFacingRight = true;
    } else if (facing_left) {
        _playerFacingRight = false;
    }

    if (is_jumping) {
        draw->sprite_rect = render::IntRect(109, 9, 16, 21);
        anim->playing = false;
    } else if (is_moving) {
        if (!anim->playing) {
            anim->playing = true;
            anim->current_frame = 0;
            anim->current_time = 0.0f;
        }
    } else {
        draw->sprite_rect = render::IntRect(90, 11, 17, 21);
        anim->playing = false;
        anim->current_frame = 0;
    }

    draw->flip_x = _playerFacingRight;
    draw->scale = 2.0f;
}
