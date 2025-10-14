/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** accessibility_menu
*/

#include "../include/accessibility_menu.hpp"
#include "../include/settings.hpp"
#include <chrono>
#include <iostream>

static bool containsPoint(const render::FloatRect &rect,
                          const render::Vector2f &point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

AccessibilityMenu::AccessibilityMenu(render::IRenderWindow &win,
                                     AudioManager &audioMgr)
    : _window(win), _audioManager(audioMgr),
      _currentMode(Settings::getInstance().getColorblindMode()) {
    _windowSize = _window.getSize();

    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg")) {
        std::cerr << "Warning: Could not load background.jpg" << std::endl;
    }

    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    if (_bgTexture->loadFromFile("assets/background.jpg")) {
        _bgSprite1->setTexture(*_bgTexture);
        _bgSprite2->setTexture(*_bgTexture);
    }
    _bgScrollSpeed = 100.0f;

    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf")) {
        std::cerr << "Warning: Could not load r-type.otf font" << std::endl;
    }

    createButtons();
}

void AccessibilityMenu::createButtons() {
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite1->setPosition(0.f, 0.f);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    _bgScrollSpeed = _windowSize.x * 0.125f;

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("GRAPHISMS");
    _titleText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText->setFillColor(render::Color::White());
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    float titleX =
        static_cast<float>(_windowSize.x) / 2.f - titleBounds.width / 2.f;
    float titleY =
        static_cast<float>(_windowSize.y) * 0.15f - titleBounds.height / 2.f;
    _titleText->setPosition(titleX, titleY);

    float labelSize = static_cast<unsigned int>(_windowSize.y * 0.04f);
    float buttonWidth = _windowSize.x * 0.08f;
    float buttonHeight = _windowSize.y * 0.06f;

    float squareSize = _windowSize.y * 0.08f;
    float squareY = _windowSize.y * 0.25f;
    float spacing = squareSize * 1.5f;
    float startX = (_windowSize.x - (3 * squareSize + 2 * spacing)) / 2.f;

    _referenceSquare1 =
        _window.createRectangleShape(render::Vector2f(squareSize, squareSize));
    _referenceSquare1->setFillColor(render::Color(220, 50, 50));
    _referenceSquare1->setPosition(startX, squareY);

    _referenceSquare2 =
        _window.createRectangleShape(render::Vector2f(squareSize, squareSize));
    _referenceSquare2->setFillColor(render::Color(50, 220, 50));
    _referenceSquare2->setPosition(startX + squareSize + spacing, squareY);

    _referenceSquare3 =
        _window.createRectangleShape(render::Vector2f(squareSize, squareSize));
    _referenceSquare3->setFillColor(render::Color(50, 50, 220));
    _referenceSquare3->setPosition(startX + 2 * (squareSize + spacing),
                                   squareY);

    _referenceLabel = _window.createText();
    _referenceLabel->setFont(*_font);
    _referenceLabel->setString("Color reference");
    _referenceLabel->setCharacterSize(labelSize * 0.8f);
    _referenceLabel->setFillColor(render::Color::White());
    render::FloatRect labelBounds = _referenceLabel->getLocalBounds();
    _referenceLabel->setPosition((_windowSize.x - labelBounds.width) / 2.f,
                                 squareY - labelSize * 1.2f);

    _colorblindLabel = _window.createText();
    _colorblindLabel->setFont(*_font);
    _colorblindLabel->setString("Filter ");
    _colorblindLabel->setCharacterSize(labelSize);
    _colorblindLabel->setFillColor(render::Color::White());
    _colorblindLabel->setPosition(_windowSize.x * 0.1f, _windowSize.y * 0.45f);

    _colorblindValue = _window.createText();
    _colorblindValue->setFont(*_font);
    _colorblindValue->setString(
        Settings::getInstance().getColorblindModeName());
    _colorblindValue->setCharacterSize(labelSize);
    _colorblindValue->setFillColor(render::Color::Yellow());
    _colorblindValue->setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.45f);

    _colorblindLeftButton = _window.createRectangleShape(
        render::Vector2f(buttonWidth, buttonHeight));
    _colorblindLeftButton->setFillColor(render::Color(100, 100, 200));
    _colorblindLeftButton->setPosition(_windowSize.x * 0.35f,
                                       _windowSize.y * 0.55f);

    _colorblindLeftText = _window.createText();
    _colorblindLeftText->setFont(*_font);
    _colorblindLeftText->setString("<");
    _colorblindLeftText->setCharacterSize(labelSize);
    _colorblindLeftText->setFillColor(render::Color::White());
    render::FloatRect leftBounds = _colorblindLeftText->getLocalBounds();
    float leftTextX =
        _windowSize.x * 0.35f + buttonWidth / 2.f - leftBounds.width / 2.f;
    float leftTextY =
        _windowSize.y * 0.55f + buttonHeight / 2.f - leftBounds.height / 2.f;
    _colorblindLeftText->setPosition(leftTextX, leftTextY);

    _colorblindRightButton = _window.createRectangleShape(
        render::Vector2f(buttonWidth, buttonHeight));
    _colorblindRightButton->setFillColor(render::Color(100, 100, 200));
    _colorblindRightButton->setPosition(_windowSize.x * 0.65f,
                                        _windowSize.y * 0.55f);

    _colorblindRightText = _window.createText();
    _colorblindRightText->setFont(*_font);
    _colorblindRightText->setString(">");
    _colorblindRightText->setCharacterSize(labelSize);
    _colorblindRightText->setFillColor(render::Color::White());
    render::FloatRect rightBounds = _colorblindRightText->getLocalBounds();
    float rightTextX =
        _windowSize.x * 0.65f + buttonWidth / 2.f - rightBounds.width / 2.f;
    float rightTextY =
        _windowSize.y * 0.55f + buttonHeight / 2.f - rightBounds.height / 2.f;
    _colorblindRightText->setPosition(rightTextX, rightTextY);

    float btnWidth = _windowSize.x * 0.2f;
    float btnHeight = _windowSize.y * 0.08f;
    _backButton =
        _window.createRectangleShape(render::Vector2f(btnWidth, btnHeight));
    _backButton->setFillColor(render::Color(150, 50, 50));
    _backButton->setPosition((_windowSize.x - btnWidth) / 2.f,
                             _windowSize.y * 0.75f);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.05f));
    _backButtonText->setFillColor(render::Color::White());
    render::FloatRect backBounds = _backButtonText->getLocalBounds();
    float backTextX = (_windowSize.x - btnWidth) / 2.f + btnWidth / 2.f -
                      backBounds.width / 2.f;
    float backTextY =
        _windowSize.y * 0.75f + btnHeight / 2.f - backBounds.height / 2.f;
    _backButtonText->setPosition(backTextX, backTextY);
}

void AccessibilityMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void AccessibilityMenu::cycleColorblindMode(int direction) {
    int currentMode = static_cast<int>(_currentMode);
    int newMode = currentMode + direction;

    if (newMode < 0) {
        newMode = 3;
    } else if (newMode > 3) {
        newMode = 0;
    }

    _currentMode = static_cast<ColorblindMode>(newMode);
    Settings::getInstance().setColorblindMode(_currentMode);

    _colorblindValue->setString(
        Settings::getInstance().getColorblindModeName());

    std::cout << "Colorblind mode set to: "
              << Settings::getInstance().getColorblindModeName() << std::endl;
}

AccessibilityResult AccessibilityMenu::run() {
    bool running = true;
    auto lastTime = std::chrono::steady_clock::now();

    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        render::Event event;
        render::Vector2f mousePos;
        while (_window.pollEvent(event)) {
            if (event.type == render::EventType::Closed) {
                return AccessibilityResult::Back;
            }

            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape ||
                    event.key.code == render::Key::P) {
                    return AccessibilityResult::Resumed;
                }
            }

            if (event.type == render::EventType::Resized) {
                _windowSize = {event.size.width, event.size.height};
                updateButtonScale();
            }

            if (event.type == render::EventType::MouseMoved) {
                mousePos.x = static_cast<float>(event.mouseMove.x);
                mousePos.y = static_cast<float>(event.mouseMove.y);
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                mousePos.x = static_cast<float>(event.mouseButton.x);
                mousePos.y = static_cast<float>(event.mouseButton.y);

                if (containsPoint(_colorblindLeftButton->getGlobalBounds(),
                                  mousePos)) {
                    cycleColorblindMode(-1);
                }

                if (containsPoint(_colorblindRightButton->getGlobalBounds(),
                                  mousePos)) {
                    cycleColorblindMode(1);
                }

                if (containsPoint(_backButton->getGlobalBounds(), mousePos)) {
                    return AccessibilityResult::Back;
                }
            }
        }

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

        render();
    }

    return AccessibilityResult::None;
}

void AccessibilityMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);
    _window.draw(*_titleText);
    _window.draw(*_referenceLabel);
    _window.draw(*_referenceSquare1);
    _window.draw(*_referenceSquare2);
    _window.draw(*_referenceSquare3);
    _window.draw(*_colorblindLabel);
    _window.draw(*_colorblindValue);
    _window.draw(*_colorblindLeftButton);
    _window.draw(*_colorblindLeftText);
    _window.draw(*_colorblindRightButton);
    _window.draw(*_colorblindRightText);
    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    _window.display();
}