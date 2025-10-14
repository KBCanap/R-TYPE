/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** menu
*/

#include "../include/menu.hpp"
#include "../include/accessibility_menu.hpp"
#include "../include/keybindings_menu.hpp"
#include "../include/render/sfml/SFMLRenderWindow.hpp"
#include <chrono>
#include <cmath>
#include <stdexcept>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect &rect,
                          const render::Vector2f &point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

Menu::Menu(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
           KeyBindings &keyBindings)
    : _registry(reg), _window(win), _audioManager(audioMgr),
      _keyBindings(keyBindings) {
    _baseWindowSize = _window.getSize();
    _windowSize = _baseWindowSize;

    // Load background
    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg"))
        throw std::runtime_error("Failed to load background.jpg");

    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    _bgSprite1->setTexture(*_bgTexture);
    _bgSprite2->setTexture(*_bgTexture);

    // Scale background
    auto texSize = _bgTexture->getSize();
    float scaleX = static_cast<float>(_windowSize.x) / texSize.x;
    float scaleY = static_cast<float>(_windowSize.y) / texSize.y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    // Load font
    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");

    createButtons();
    createEnemies();

    _audioManager.loadMusic(MusicType::MAIN_MENU,
                            "assets/audio/main_music.ogg");
    _audioManager.playMusic(MusicType::MAIN_MENU, true);
}

void Menu::createButtons() {
    _buttons.clear();
    _buttonTexts.clear();

    std::vector<std::string> labels = {"Play", "Options", "Quit"};

    float btnWidth = _windowSize.x * 0.25f;
    float btnHeight = _windowSize.y * 0.08f;
    float startY = _windowSize.y * 0.6f;
    float spacing = btnHeight * 1.3f;

    for (size_t i = 0; i < labels.size(); ++i) {
        auto btn =
            _window.createRectangleShape(render::Vector2f(btnWidth, btnHeight));
        btn->setFillColor(
            render::Color(static_cast<uint8_t>(100 + i * 50), 150, 200));
        btn->setPosition((_windowSize.x - btnWidth) / 2.f,
                         startY + i * spacing);
        _buttons.push_back(std::move(btn));

        auto text = _window.createText();
        text->setFont(*_font);
        text->setString(labels[i]);
        text->setCharacterSize(static_cast<unsigned int>(btnHeight * 0.5f));
        text->setFillColor(render::Color::White());

        render::FloatRect bounds = text->getLocalBounds();
        float textX = (_windowSize.x - btnWidth) / 2.f + btnWidth / 2.f -
                      (bounds.width / 2.f);
        float textY =
            startY + i * spacing + btnHeight / 2.f - (bounds.height / 2.f);
        text->setPosition(textX, textY);

        _buttonTexts.push_back(std::move(text));
    }
}

void Menu::updateButtonScale() {
    createButtons();

    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    _bgScrollSpeed = _windowSize.x * 0.125f;

    _bgSprite1->setPosition(0.f, 0.f);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    updateEnemyPositions();
}

void Menu::updateEnemyPositions() {
    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();

    for (size_t i = 0; i < _enemies.size(); ++i) {
        auto &pos = positions[_enemies[i]];
        auto &draw = drawables[_enemies[i]];

        if (pos) {
            float x = _windowSize.x * 0.1f + i * (_windowSize.x * 0.15f);
            float y = _windowSize.y * 0.1f + (i % 2) * (_windowSize.y * 0.08f);

            pos->x = x;
            pos->y = y;

            _enemyStartPositions[i] = {x, y};
        }

        if (draw) {
            float enemySize = std::min(_windowSize.x, _windowSize.y) * 0.04f;
            draw->size = enemySize;
        }
    }
}

void Menu::createEnemies() {
    _enemies.clear();
    _enemyStartPositions.clear();

    for (int i = 0; i < 5; ++i) {
        auto e = _registry.spawn_entity();

        float x = _windowSize.x * 0.1f + i * (_windowSize.x * 0.15f);
        float y = _windowSize.y * 0.1f + (i % 2) * (_windowSize.y * 0.08f);

        _registry.add_component<component::position>(e,
                                                     component::position(x, y));
        _registry.add_component<component::velocity>(
            e, component::velocity(50.f, 0.f));

        float enemySize = std::min(_windowSize.x, _windowSize.y) * 0.04f;
        _registry.add_component<component::drawable>(
            e, component::drawable(render::Color::Red(), enemySize));

        _enemies.push_back(e);
        _enemyStartPositions.push_back({x, y});
    }
}

void Menu::resetEnemies() {
    auto &positions = _registry.get_components<component::position>();
    auto &velocities = _registry.get_components<component::velocity>();

    for (size_t i = 0; i < _enemies.size(); ++i) {
        auto &pos = positions[_enemies[i]];
        auto &vel = velocities[_enemies[i]];

        if (pos) {
            pos->x = _enemyStartPositions[i].x;
            pos->y = _enemyStartPositions[i].y;
        }
        if (vel) {
            vel->vx = 50.f;
            vel->vy = 0.f;
        }
    }
}

MenuResult Menu::run() {
    bool running = true;
    auto lastTime = std::chrono::steady_clock::now();

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        render::Event event;
        render::Vector2f mousePos;
        while (_window.pollEvent(event)) {
            if (event.type == render::EventType::Closed)
                return MenuResult::Quit;

            if (event.type == render::EventType::MouseMoved) {
                mousePos.x = static_cast<float>(event.mouseMove.x);
                mousePos.y = static_cast<float>(event.mouseMove.y);
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                mousePos.x = static_cast<float>(event.mouseButton.x);
                mousePos.y = static_cast<float>(event.mouseButton.y);

                for (size_t i = 0; i < _buttons.size(); ++i) {
                    if (containsPoint(_buttons[i]->getGlobalBounds(),
                                      mousePos)) {
                        switch (i) {
                        case 0:
                            // Supprimer tous les ennemis du menu avant de
                            // lancer le jeu
                            for (auto enemy : _enemies) {
                                _registry.kill_entity(enemy);
                            }
                            _enemies.clear();
                            _enemyStartPositions.clear();
                            return MenuResult::Play;
                        case 1: {
                            OptionsMenu optionsMenu(_window, _audioManager,
                                                    _keyBindings);
                            OptionsResult result = optionsMenu.run();

                            if (result == OptionsResult::Accessibility) {
                                AccessibilityMenu accessibilityMenu(
                                    _window, _audioManager);
                                accessibilityMenu.run();
                            }

                            if (result == OptionsResult::KeyBindings) {
                                KeyBindingsMenu keyBindingsMenu(
                                    _window, _audioManager, _keyBindings);
                                keyBindingsMenu.run();
                            }

                            _windowSize = _window.getSize();
                            updateButtonScale();
                            break;
                        }
                        case 2:
                            return MenuResult::Quit;
                        }
                    }
                }
            }

            if (event.type == render::EventType::Resized) {
                _windowSize = {event.size.width, event.size.height};

                float scaleX =
                    static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
                float scaleY =
                    static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
                _bgSprite1->setScale(scaleX, scaleY);
                _bgSprite2->setScale(scaleX, scaleY);

                float currentBg1X = _bgSprite1->getPosition().x;
                float currentBg2X = _bgSprite2->getPosition().x;

                if (currentBg1X <= 0 &&
                    currentBg1X > -static_cast<float>(_windowSize.x)) {
                    _bgSprite1->setPosition(
                        currentBg1X * (_windowSize.x /
                                       static_cast<float>(_baseWindowSize.x)),
                        0.f);
                    _bgSprite2->setPosition(
                        _bgSprite1->getPosition().x +
                            static_cast<float>(_windowSize.x),
                        0.f);
                } else {
                    _bgSprite2->setPosition(
                        currentBg2X * (_windowSize.x /
                                       static_cast<float>(_baseWindowSize.x)),
                        0.f);
                    _bgSprite1->setPosition(
                        _bgSprite2->getPosition().x +
                            static_cast<float>(_windowSize.x),
                        0.f);
                }

                updateButtonScale();

                _bgScrollSpeed = _windowSize.x * 0.125f;

                _baseWindowSize = _windowSize;
            }
        }

        // Background scroll
        auto bg1Pos = _bgSprite1->getPosition();
        auto bg2Pos = _bgSprite2->getPosition();
        _bgSprite1->setPosition(bg1Pos.x - _bgScrollSpeed * dt, bg1Pos.y);
        _bgSprite2->setPosition(bg2Pos.x - _bgScrollSpeed * dt, bg2Pos.y);

        bg1Pos = _bgSprite1->getPosition();
        bg2Pos = _bgSprite2->getPosition();
        if (bg1Pos.x + _windowSize.x < 0)
            _bgSprite1->setPosition(bg2Pos.x + _windowSize.x, 0.f);
        if (bg2Pos.x + _windowSize.x < 0)
            _bgSprite2->setPosition(bg1Pos.x + _windowSize.x, 0.f);

        // Enemy movement
        auto &positions = _registry.get_components<component::position>();
        auto &velocities = _registry.get_components<component::velocity>();
        for (size_t i = 0; i < _enemies.size(); ++i) {
            auto &pos = positions[_enemies[i]];
            auto &vel = velocities[_enemies[i]];
            if (pos && vel) {
                pos->x += vel->vx * dt;
                pos->y =
                    _enemyStartPositions[i].y + 20.f * std::sin(pos->x / 50.f);
            }
        }

        // Render
        _window.clear();
        _window.draw(*_bgSprite1);
        _window.draw(*_bgSprite2);
        for (auto &btn : _buttons)
            _window.draw(*btn);
        for (auto &txt : _buttonTexts)
            _window.draw(*txt);

        auto &drawables = _registry.get_components<component::drawable>();
        for (auto &e : _enemies) {
            auto &pos = positions[e];
            auto &draw = drawables[e];
            if (pos && draw) {
                auto shape = _window.createRectangleShape(
                    render::Vector2f(draw->size, draw->size));
                shape->setPosition(pos->x, pos->y);
                shape->setFillColor(draw->color);
                _window.draw(*shape);
            }
        }

        _window.display();
    }

    return MenuResult::None;
}