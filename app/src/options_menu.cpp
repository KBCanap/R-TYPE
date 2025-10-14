/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** options_menu
*/

#include "../include/options_menu.hpp"
#include "../include/settings.hpp"
#include <chrono>
#include <iostream>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect &rect,
                          const render::Vector2f &point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

OptionsMenu::OptionsMenu(render::IRenderWindow &win, AudioManager &audioMgr,
                         KeyBindings &keyBindings)
    : _window(win), _audioManager(audioMgr), _keyBindings(keyBindings),
      _currentResolution(1), _soundEnabled(true) {
    _windowSize = _window.getSize();

    // Initialize available resolutions
    _resolutions = {{800, 600, "800x600"},
                    {1024, 768, "1024x768"},
                    {1280, 720, "1280x720"}};

    // Find current resolution
    for (size_t i = 0; i < _resolutions.size(); ++i) {
        if (_resolutions[i].width == _windowSize.x &&
            _resolutions[i].height == _windowSize.y) {
            _currentResolution = i;
            break;
        }
    }

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

void OptionsMenu::createButtons() {
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite1->setPosition(0.f, 0.f);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    _bgScrollSpeed = _windowSize.x * 0.125f;

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("OPTIONS");
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

    _resolutionLabel = _window.createText();
    _resolutionLabel->setFont(*_font);
    _resolutionLabel->setString("Resolution ");
    _resolutionLabel->setCharacterSize(labelSize);
    _resolutionLabel->setFillColor(render::Color::White());
    _resolutionLabel->setPosition(_windowSize.x * 0.15f, _windowSize.y * 0.28f);

    _resolutionValue = _window.createText();
    _resolutionValue->setFont(*_font);
    _resolutionValue->setString(_resolutions[_currentResolution].name);
    _resolutionValue->setCharacterSize(labelSize);
    _resolutionValue->setFillColor(render::Color::Yellow());
    _resolutionValue->setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.28f);

    _resolutionLeftButton = _window.createRectangleShape(
        render::Vector2f(buttonWidth, buttonHeight));
    _resolutionLeftButton->setFillColor(render::Color(100, 100, 200));
    _resolutionLeftButton->setPosition(_windowSize.x * 0.35f,
                                       _windowSize.y * 0.40f);

    _resolutionLeftText = _window.createText();
    _resolutionLeftText->setFont(*_font);
    _resolutionLeftText->setString("<");
    _resolutionLeftText->setCharacterSize(labelSize);
    _resolutionLeftText->setFillColor(render::Color::White());
    render::FloatRect leftBounds = _resolutionLeftText->getLocalBounds();
    float leftTextX =
        _windowSize.x * 0.35f + buttonWidth / 2.f - leftBounds.width / 2.f;
    float leftTextY =
        _windowSize.y * 0.40f + buttonHeight / 2.f - leftBounds.height / 2.f;
    _resolutionLeftText->setPosition(leftTextX, leftTextY);

    _resolutionRightButton = _window.createRectangleShape(
        render::Vector2f(buttonWidth, buttonHeight));
    _resolutionRightButton->setFillColor(render::Color(100, 100, 200));
    _resolutionRightButton->setPosition(_windowSize.x * 0.65f,
                                        _windowSize.y * 0.40f);

    _resolutionRightText = _window.createText();
    _resolutionRightText->setFont(*_font);
    _resolutionRightText->setString(">");
    _resolutionRightText->setCharacterSize(labelSize);
    _resolutionRightText->setFillColor(render::Color::White());
    render::FloatRect rightBounds = _resolutionRightText->getLocalBounds();
    float rightTextX =
        _windowSize.x * 0.65f + buttonWidth / 2.f - rightBounds.width / 2.f;
    float rightTextY =
        _windowSize.y * 0.40f + buttonHeight / 2.f - rightBounds.height / 2.f;
    _resolutionRightText->setPosition(rightTextX, rightTextY);

    _soundLabel = _window.createText();
    _soundLabel->setFont(*_font);
    _soundLabel->setString("Sound ");
    _soundLabel->setCharacterSize(labelSize);
    _soundLabel->setFillColor(render::Color::White());
    _soundLabel->setPosition(_windowSize.x * 0.2f, _windowSize.y * 0.5f);

    _soundValue = _window.createText();
    _soundValue->setFont(*_font);
    _soundValue->setString(_soundEnabled ? "ON" : "OFF");
    _soundValue->setCharacterSize(labelSize);
    _soundValue->setFillColor(_soundEnabled ? render::Color::Green()
                                            : render::Color::Red());
    _soundValue->setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.5f);

    _soundButton = _window.createRectangleShape(
        render::Vector2f(buttonWidth * 2.f, buttonHeight));
    _soundButton->setFillColor(render::Color(100, 100, 200));
    _soundButton->setPosition(_windowSize.x * 0.6f, _windowSize.y * 0.48f);

    _soundButtonText = _window.createText();
    _soundButtonText->setFont(*_font);
    _soundButtonText->setString("Toggle");
    _soundButtonText->setCharacterSize(labelSize * 0.8f);
    _soundButtonText->setFillColor(render::Color::White());
    render::FloatRect soundBounds = _soundButtonText->getLocalBounds();
    float soundTextX =
        _windowSize.x * 0.6f + buttonWidth - soundBounds.width / 2.f;
    float soundTextY =
        _windowSize.y * 0.48f + buttonHeight / 2.f - soundBounds.height / 2.f;
    _soundButtonText->setPosition(soundTextX, soundTextY);

    _accessibilityButton = _window.createRectangleShape(
        render::Vector2f(_windowSize.x * 0.4f, _windowSize.y * 0.08f));
    _accessibilityButton->setFillColor(render::Color(150, 100, 200));
    _accessibilityButton->setPosition(
        (_windowSize.x - _windowSize.x * 0.4f) / 2.f, _windowSize.y * 0.60f);

    _accessibilityButtonText = _window.createText();
    _accessibilityButtonText->setFont(*_font);
    _accessibilityButtonText->setString("Accessibility");
    _accessibilityButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.04f));
    _accessibilityButtonText->setFillColor(render::Color::White());
    render::FloatRect accessibilityBounds =
        _accessibilityButtonText->getLocalBounds();
    float accessibilityTextX = (_windowSize.x - _windowSize.x * 0.4f) / 2.f +
                               _windowSize.x * 0.4f / 2.f -
                               accessibilityBounds.width / 2.f;
    float accessibilityTextY = _windowSize.y * 0.60f +
                               _windowSize.y * 0.08f / 2.f -
                               accessibilityBounds.height / 2.f;
    _accessibilityButtonText->setPosition(accessibilityTextX,
                                          accessibilityTextY);

    _keyBindingsButton = _window.createRectangleShape(
        render::Vector2f(_windowSize.x * 0.4f, _windowSize.y * 0.08f));
    _keyBindingsButton->setFillColor(render::Color(100, 150, 200));
    _keyBindingsButton->setPosition(
        (_windowSize.x - _windowSize.x * 0.4f) / 2.f, _windowSize.y * 0.70f);

    _keyBindingsButtonText = _window.createText();
    _keyBindingsButtonText->setFont(*_font);
    _keyBindingsButtonText->setString("Key Bindings");
    _keyBindingsButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.04f));
    _keyBindingsButtonText->setFillColor(render::Color::White());
    render::FloatRect keyBindingsBounds =
        _keyBindingsButtonText->getLocalBounds();
    float keyBindingsTextX = (_windowSize.x - _windowSize.x * 0.4f) / 2.f +
                             _windowSize.x * 0.4f / 2.f -
                             keyBindingsBounds.width / 2.f;
    float keyBindingsTextY = _windowSize.y * 0.70f +
                             _windowSize.y * 0.08f / 2.f -
                             keyBindingsBounds.height / 2.f;
    _keyBindingsButtonText->setPosition(keyBindingsTextX, keyBindingsTextY);

    _backButton = _window.createRectangleShape(
        render::Vector2f(_windowSize.x * 0.2f, _windowSize.y * 0.08f));
    _backButton->setFillColor(render::Color(150, 50, 50));
    _backButton->setPosition((_windowSize.x - _windowSize.x * 0.2f) / 2.f,
                             _windowSize.y * 0.82f);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.05f));
    _backButtonText->setFillColor(render::Color::White());
    render::FloatRect backBounds = _backButtonText->getLocalBounds();
    float backTextX = (_windowSize.x - _windowSize.x * 0.2f) / 2.f +
                      _windowSize.x * 0.2f / 2.f - backBounds.width / 2.f;
    float backTextY = _windowSize.y * 0.82f + _windowSize.y * 0.08f / 2.f -
                      backBounds.height / 2.f;
    _backButtonText->setPosition(backTextX, backTextY - _windowSize.y * 0.01f);
}

void OptionsMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void OptionsMenu::handleResolutionChange() {
    Resolution newRes = _resolutions[_currentResolution];

    _window.setSize(render::Vector2u(newRes.width, newRes.height));
    _windowSize.x = newRes.width;
    _windowSize.y = newRes.height;

    updateButtonScale();

    std::cout << "Resolution changed to: " << newRes.name << std::endl;
}

void OptionsMenu::toggleSound() {
    _soundEnabled = !_soundEnabled;
    _soundValue->setString(_soundEnabled ? "ON" : "OFF");
    _soundValue->setFillColor(_soundEnabled ? render::Color::Green()
                                            : render::Color::Red());

    if (_soundEnabled) {
        _audioManager.setMasterVolume(100.0f);
    } else {
        _audioManager.setMasterVolume(0.0f);
    }

    std::cout << "Sound " << (_soundEnabled ? "enabled" : "disabled")
              << std::endl;
}

OptionsResult OptionsMenu::run() {
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
                return OptionsResult::Back;
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

                if (containsPoint(_resolutionLeftButton->getGlobalBounds(),
                                  mousePos)) {
                    if (_currentResolution > 0) {
                        _currentResolution--;
                        _resolutionValue->setString(
                            _resolutions[_currentResolution].name);
                        handleResolutionChange();
                    }
                }

                if (containsPoint(_resolutionRightButton->getGlobalBounds(),
                                  mousePos)) {
                    if (_currentResolution < _resolutions.size() - 1) {
                        _currentResolution++;
                        _resolutionValue->setString(
                            _resolutions[_currentResolution].name);
                        handleResolutionChange();
                    }
                }

                if (containsPoint(_soundButton->getGlobalBounds(), mousePos)) {
                    toggleSound();
                }

                if (containsPoint(_accessibilityButton->getGlobalBounds(),
                                  mousePos)) {
                    return OptionsResult::Accessibility;
                }

                if (containsPoint(_keyBindingsButton->getGlobalBounds(),
                                  mousePos)) {
                    return OptionsResult::KeyBindings;
                }

                if (containsPoint(_backButton->getGlobalBounds(), mousePos)) {
                    return OptionsResult::Back;
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

    return OptionsResult::None;
}

void OptionsMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);

    _window.draw(*_resolutionLabel);
    _window.draw(*_resolutionValue);
    _window.draw(*_resolutionLeftButton);
    _window.draw(*_resolutionLeftText);
    _window.draw(*_resolutionRightButton);
    _window.draw(*_resolutionRightText);

    _window.draw(*_soundLabel);
    _window.draw(*_soundValue);
    _window.draw(*_soundButton);
    _window.draw(*_soundButtonText);

    _window.draw(*_accessibilityButton);
    _window.draw(*_accessibilityButtonText);

    _window.draw(*_keyBindingsButton);
    _window.draw(*_keyBindingsButtonText);

    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    _window.display();
}
