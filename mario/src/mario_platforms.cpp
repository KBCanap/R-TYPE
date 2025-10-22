/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_platforms - Platform creation and management
*/

#include "mario_game.hpp"
#include <iostream>

void MarioGame::createPlatforms() {
    // Get window size and calculate scale factors
    render::Vector2u window_size = _window.getSize();
    float bg_width = 256.0f;  // Original background width
    float bg_height = 224.0f; // Original background height

    float scale_x = static_cast<float>(window_size.x) / bg_width;
    float scale_y = static_cast<float>(window_size.y) / bg_height;

    // Platform data in original background coordinates
    struct PlatformData {
        float x, y, width, height;
        const char *name;
    };

    // Coordinates based on exact pixel measurements from background
    PlatformData platforms[] = {
        // Ground floor - main section (16 pixels high)
        {16.0f, 207.0f, 224.0f, 16.0f, "Ground - Main section (16-240)"},

        // Ground floor - left edge (33 pixels high, starts at y=191)
        {0.0f, 191.0f, 16.0f, 33.0f, "Ground - Left edge (0-16, 33px high)"},

        // Ground floor - right edge (33 pixels high, starts at y=191)
        {240.0f, 191.0f, 19.0f, 33.0f, "Ground - Right edge (240-259, 33px high)"},

        // Level 1 platforms (y=159)
        {0.0f, 159.0f, 88.0f, 10.0f, "Platform L1 - Left (0-88)"},
        {167.0f, 159.0f, 92.0f, 10.0f, "Platform L1 - Right (167-259)"},

        // Level 2 platforms (y=120)
        {0.0f, 120.0f, 32.0f, 10.0f, "Platform L2 - Left (0-32)"},
        {224.0f, 120.0f, 35.0f, 10.0f, "Platform L2 - Right (224-259)"},

        // Level 3 platform (y=111) - center long platform
        {56.0f, 111.0f, 144.0f, 10.0f, "Platform L3 - Center long (56-200)"},

        // Level 4 platforms (y=63) - top level
        {0.0f, 63.0f, 111.0f, 10.0f, "Platform L4 - Left (0-111)"},
        {144.0f, 63.0f, 115.0f, 10.0f, "Platform L4 - Right (144-259)"},
    };

    for (const auto &plat : platforms) {
        auto platform = _registry.spawn_entity();

        // Scale platform coordinates to match scaled background
        float scaled_x = plat.x * scale_x;
        float scaled_y = plat.y * scale_y;
        float scaled_width = plat.width * scale_x;
        float scaled_height = plat.height * scale_y;

        // Position
        _registry.add_component<component::position>(
            platform, component::position(scaled_x, scaled_y));

        // Hitbox
        _registry.add_component<component::hitbox>(
            platform,
            component::hitbox(scaled_width, scaled_height, 0.0f, 0.0f));

        // Visual representation (semi-transparent red for debugging)
        _registry.add_component<component::drawable>(
            platform,
            component::drawable(render::Color(200, 50, 50, 128), scaled_height));

        // Tag as platform
        _registry.add_component<component::platform_tag>(
            platform, component::platform_tag());

        std::cout << "[MarioGame] Created " << plat.name << " at (" << scaled_x
                  << ", " << scaled_y << ") with size (" << scaled_width << "x"
                  << scaled_height << ")" << std::endl;
    }
}
