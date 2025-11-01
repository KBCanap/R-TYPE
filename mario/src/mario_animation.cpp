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

    // Add animation component to player
    auto &anim = _registry.add_component<component::animation>(
        *_player, component::animation(0.1f, true, false));

    // Setup animation frames for walking (7 frames)
    anim->frames = {
        render::IntRect(0, 11, 17, 21),    // Frame 1: 0-17
        render::IntRect(18, 11, 15, 21),   // Frame 2: 18-33 (width = 33-18 = 15)
        render::IntRect(38, 11, 13, 21),   // Frame 3: 38-51 (width = 51-38 = 13)
        render::IntRect(55, 11, 15, 21),   // Frame 4: 55-70 (width = 70-55 = 15)
        render::IntRect(73, 11, 14, 21),   // Frame 5: 73-87 (width = 87-73 = 14)
    };

    anim->current_frame = 0;
    anim->playing = false; // Start idle
}

void MarioGame::updatePlayerAnimation(float dt) {
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

    // Determine animation state
    bool is_moving = (vel->vx != 0.0f);
    bool is_jumping = !grav->on_ground;
    bool facing_right = vel->vx > 0.0f;
    bool facing_left = vel->vx < 0.0f;

    // Update facing direction
    if (facing_right) {
        _playerFacingRight = true;
    } else if (facing_left) {
        _playerFacingRight = false;
    }

    // Set sprite based on state
    if (is_jumping) {
        // Jumping frame: 109,9 to 125,9
        draw->sprite_rect = render::IntRect(109, 9, 16, 21); // width = 125-109 = 16
        anim->playing = false;
    } else if (is_moving) {
        // Walking animation
        if (!anim->playing) {
            anim->playing = true;
            anim->current_frame = 0;
            anim->current_time = 0.0f;
        }

        // Update animation
        anim->current_time += dt;
        if (anim->current_time >= anim->frame_duration) {
            anim->current_time = 0.0f;
            anim->current_frame = (anim->current_frame + 1) % anim->frames.size();
        }

        draw->sprite_rect = anim->frames[anim->current_frame];
    } else {
        // Idle frame: 90,11 to 107,11
        draw->sprite_rect = render::IntRect(90, 11, 17, 21); // width = 107-90 = 17
        anim->playing = false;
        anim->current_frame = 0;
    }

    // Apply horizontal flip - sprite faces left by default, flip when facing right
    if (_playerFacingRight) {
        // Mark sprite as flipped (will be handled in render)
        draw->scale = -2.0f; // Negative scale = flip to face right
    } else {
        draw->scale = 2.0f; // Normal = face left
    }
}
