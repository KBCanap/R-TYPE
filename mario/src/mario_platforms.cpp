/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_platforms - Platform creation and management
*/

#include "mario_game.hpp"
#include <iostream>

void MarioGame::createPlatforms() {
    render::Vector2u window_size = _window.getSize();
    float bg_width = 256.0f;
    float bg_height = 224.0f;

    float scale_x = static_cast<float>(window_size.x) / bg_width;
    float scale_y = static_cast<float>(window_size.y) / bg_height;

    struct PlatformData {
        float x, y, width, height;
    };

    PlatformData platforms[] = {
        {16.0f, 207.0f, 224.0f, 16.0f},
        {0.0f, 191.0f, 16.0f, 33.0f},
        {240.0f, 191.0f, 19.0f, 33.0f},
        {0.0f, 159.0f, 88.0f, 10.0f},
        {167.0f, 159.0f, 92.0f, 10.0f},
        {0.0f, 120.0f, 32.0f, 10.0f},
        {224.0f, 120.0f, 35.0f, 10.0f},
        {56.0f, 111.0f, 144.0f, 10.0f},
        {0.0f, 63.0f, 111.0f, 10.0f},
        {144.0f, 63.0f, 115.0f, 10.0f},
    };

    for (const auto &plat : platforms) {
        auto platform = _registry.spawn_entity();

        float scaled_x = plat.x * scale_x;
        float scaled_y = plat.y * scale_y;
        float scaled_width = plat.width * scale_x;
        float scaled_height = plat.height * scale_y;

        _registry.add_component<component::position>(
            platform, component::position(scaled_x, scaled_y));

        _registry.add_component<component::hitbox>(
            platform, component::hitbox(scaled_width, scaled_height, 0.0f, 0.0f));

        _registry.add_component<component::platform_tag>(
            platform, component::platform_tag());
    }
}

void MarioGame::createPowBlock(float scale_x, float scale_y) {
    const float pow_x = 120.0f;
    const float pow_y = 159.0f;
    const float pow_width = 18.0f;
    const float pow_height = 16.0f;

    _powBlock = _registry.spawn_entity();

    float scaled_x = pow_x * scale_x;
    float scaled_y = pow_y * scale_y;
    float scaled_width = pow_width * scale_x;
    float scaled_height = pow_height * scale_y;

    _registry.add_component<component::position>(
        *_powBlock, component::position(scaled_x, scaled_y));

    _registry.add_component<component::hitbox>(
        *_powBlock, component::hitbox(scaled_width, scaled_height, 0.0f, 0.0f));

    float cover_size = std::max(scaled_width, scaled_height);
    auto &drawable = _registry.add_component<component::drawable>(
        *_powBlock, component::drawable(render::Color(0, 0, 0, 0), cover_size));
    drawable->tag = "pow_block";

    _registry.add_component<component::platform_tag>(
        *_powBlock, component::platform_tag());

    _registry.add_component<component::pow_block>(
        *_powBlock, component::pow_block(3));
}
