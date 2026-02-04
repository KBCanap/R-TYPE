/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_render - Rendering and display
*/

#include "mario_game.hpp"
#include "systems.hpp"
#include <string>

void MarioGame::render(float dt) {
    _window.clear(render::Color(100, 150, 255));

    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();

    systems::render_system(_registry, positions, drawables, _window, dt);

    if (_debugFont) {
        render::Vector2u window_size = _window.getSize();
        float sw = static_cast<float>(window_size.x);

        auto levelText = _window.createText();
        levelText->setFont(*_debugFont);
        levelText->setCharacterSize(20);
        levelText->setFillColor(render::Color::White());
        levelText->setString("LEVEL " + std::to_string(_level));
        levelText->setPosition(sw * 0.52f, 10.0f);
        _window.draw(*levelText);
    }

    if (_gameOver && _debugFont) {
        render::Vector2u window_size = _window.getSize();
        float sw = static_cast<float>(window_size.x);
        float sh = static_cast<float>(window_size.y);

        auto gameOverText = _window.createText();
        gameOverText->setFont(*_debugFont);
        gameOverText->setCharacterSize(40);
        gameOverText->setFillColor(render::Color::Red());
        gameOverText->setString("GAME OVER");
        gameOverText->setPosition(sw * 0.35f, sh * 0.4f);
        _window.draw(*gameOverText);

        auto restartText = _window.createText();
        restartText->setFont(*_debugFont);
        restartText->setCharacterSize(16);
        restartText->setFillColor(render::Color::White());
        restartText->setString("Press ENTER to restart");
        restartText->setPosition(sw * 0.35f, sh * 0.55f);
        _window.draw(*restartText);
    }

    if (_victory && _debugFont) {
        render::Vector2u window_size = _window.getSize();
        float sw = static_cast<float>(window_size.x);
        float sh = static_cast<float>(window_size.y);

        auto victoryText = _window.createText();
        victoryText->setFont(*_debugFont);
        victoryText->setCharacterSize(40);
        victoryText->setFillColor(render::Color::Green());
        victoryText->setString("VICTORY!");
        victoryText->setPosition(sw * 0.38f, sh * 0.4f);
        _window.draw(*victoryText);

        auto nextText = _window.createText();
        nextText->setFont(*_debugFont);
        nextText->setCharacterSize(16);
        nextText->setFillColor(render::Color::White());
        nextText->setString("Press ENTER for next level");
        nextText->setPosition(sw * 0.32f, sh * 0.55f);
        _window.draw(*nextText);
    }

    _window.display();
}
