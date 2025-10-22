/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_render - Rendering and display
*/

#include "mario_game.hpp"
#include <string>

void MarioGame::render() {
    _window.clear(render::Color(100, 150, 255));

    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    auto &backgrounds = _registry.get_components<component::background>();

    // Render background first - scaled to window size
    if (_background && *_background < backgrounds.size()) {
        auto &bg_component = backgrounds[*_background];
        auto &bg_pos = positions[*_background];

        if (bg_component && bg_component->texture && bg_pos) {
            auto sprite = _window.createSprite();
            sprite->setTexture(*bg_component->texture);

            // Scale background to fit window
            render::Vector2u window_size = _window.getSize();
            render::Vector2u texture_size = bg_component->texture->getSize();

            float scale_x =
                static_cast<float>(window_size.x) / texture_size.x;
            float scale_y =
                static_cast<float>(window_size.y) / texture_size.y;

            sprite->setScale(scale_x, scale_y);
            sprite->setPosition(bg_pos->x, bg_pos->y);
            _window.draw(*sprite);
        }
    }

    // Render platforms and player
    for (size_t i = 0; i < positions.size(); ++i) {
        std::optional<component::position> &pos = positions[i];
        if (!pos.has_value())
            continue;
        if (i >= drawables.size() || !drawables[i].has_value())
            continue;

        std::optional<component::drawable> &draw = drawables[i];

        bool has_hitbox = i < hitboxes.size() && hitboxes[i].has_value();

        // Skip background
        if (_background && i == *_background)
            continue;

        // Draw player as sprite
        if (_player && i == *_player && draw->use_sprite) {
            // Initialize sprite and texture if needed
            if (!draw->sprite && !draw->texture_path.empty()) {
                draw->texture = _window.createTexture();
                if (draw->texture->loadFromFile(draw->texture_path)) {
                    draw->sprite = _window.createSprite();
                    draw->sprite->setTexture(*draw->texture);
                }
            }

            if (draw->sprite) {
                draw->sprite->setTextureRect(draw->sprite_rect);
                draw->sprite->setPosition(pos->x, pos->y);

                // Handle horizontal flip for direction
                if (draw->scale < 0) {
                    // Facing left - flip horizontally
                    draw->sprite->setScale(-std::abs(draw->scale), std::abs(draw->scale));
                    // Adjust position for flipped sprite
                    draw->sprite->setOrigin(draw->sprite_rect.width, 0);
                } else {
                    // Facing right - normal
                    draw->sprite->setScale(draw->scale, draw->scale);
                    draw->sprite->setOrigin(0, 0);
                }

                _window.draw(*draw->sprite);
            }
        }
        // Draw platforms as rectangles
        else if (has_hitbox) {
            std::optional<component::hitbox> &hitbox = hitboxes[i];
            auto rect = _window.createRectangleShape(
                render::Vector2f(hitbox->width, hitbox->height));
            rect->setPosition(pos->x + hitbox->offset_x,
                              pos->y + hitbox->offset_y);
            rect->setFillColor(draw->color);
            _window.draw(*rect);
        }
    }

    // Draw debug info
    renderDebugInfo();

    _window.display();
}

void MarioGame::renderDebugInfo() {
    if (!_player || !_debugFont)
        return;

    auto &positions = _registry.get_components<component::position>();
    auto &gravities = _registry.get_components<component::gravity>();

    auto &player_pos = positions[*_player];
    auto &player_grav = gravities[*_player];

    if (player_pos && player_grav) {
        auto text = _window.createText();
        text->setFont(*_debugFont);
        text->setCharacterSize(12);
        text->setFillColor(render::Color::White());

        std::string debug_text = "Position: (" +
                                 std::to_string(static_cast<int>(player_pos->x)) + ", " +
                                 std::to_string(static_cast<int>(player_pos->y)) + ")";
        debug_text += "\nOn Ground: ";
        debug_text += player_grav->on_ground ? "YES" : "NO";
        debug_text += "\nControls: Arrow keys to move, Up to jump, ESC to "
                      "exit";

        text->setString(debug_text);
        text->setPosition(10.0f, 10.0f);
        _window.draw(*text);
    }
}
