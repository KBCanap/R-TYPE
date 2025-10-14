/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** keybindings_menu
*/

#include "../include/keybindings_menu.hpp"
#include <chrono>
#include <iostream>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect &rect,
                          const render::Vector2f &point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

// Helper function to get action name
static std::string getActionName(GameAction action) {
    switch (action) {
    case GameAction::MoveLeft:
        return "Move Left";
    case GameAction::MoveRight:
        return "Move Right";
    case GameAction::MoveUp:
        return "Move Up";
    case GameAction::MoveDown:
        return "Move Down";
    case GameAction::Fire:
        return "Fire";
    default:
        return "Unknown";
    }
}

KeyBindingsMenu::KeyBindingsMenu(render::IRenderWindow &win,
                                 AudioManager &audioMgr,
                                 KeyBindings &keyBindings)
    : _window(win), _audioManager(audioMgr), _keyBindings(keyBindings),
      _waitingForKey(false) {
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

void KeyBindingsMenu::createButtons() {
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite1->setPosition(0.f, 0.f);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    _bgScrollSpeed = _windowSize.x * 0.125f;

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("KEY BINDINGS");
    _titleText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText->setFillColor(render::Color::White());
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    float titleX =
        static_cast<float>(_windowSize.x) / 2.f - titleBounds.width / 2.f;
    float titleY =
        static_cast<float>(_windowSize.y) * 0.10f - titleBounds.height / 2.f;
    _titleText->setPosition(titleX, titleY);

    float labelSize = static_cast<unsigned int>(_windowSize.y * 0.035f);
    float buttonWidth = _windowSize.x * 0.15f;
    float buttonHeight = _windowSize.y * 0.05f;
    float startY = _windowSize.y * 0.22f;
    float spacing = _windowSize.y * 0.09f;

    _bindings.clear();
    std::vector<GameAction> actions = {GameAction::MoveUp, GameAction::MoveDown,
                                       GameAction::MoveLeft,
                                       GameAction::MoveRight, GameAction::Fire};

    for (size_t i = 0; i < actions.size(); ++i) {
        KeyBindingEntry entry;
        entry.action = actions[i];
        entry.actionName = getActionName(actions[i]);

        entry.label = _window.createText();
        entry.label->setFont(*_font);
        entry.label->setString(entry.actionName);
        entry.label->setCharacterSize(labelSize);
        entry.label->setFillColor(render::Color::White());
        entry.label->setPosition(_windowSize.x * 0.15f, startY + i * spacing);

        entry.keyText = _window.createText();
        entry.keyText->setFont(*_font);
        render::Key currentKey = _keyBindings.getBinding(actions[i]);
        entry.keyText->setString(KeyBindings::getKeyName(currentKey));
        entry.keyText->setCharacterSize(labelSize);
        entry.keyText->setFillColor(render::Color::Yellow());
        entry.keyText->setPosition(_windowSize.x * 0.45f, startY + i * spacing);

        entry.button = _window.createRectangleShape(
            render::Vector2f(buttonWidth, buttonHeight));
        entry.button->setFillColor(render::Color(100, 100, 200));
        entry.button->setPosition(_windowSize.x * 0.65f,
                                  startY + i * spacing - buttonHeight * 0.1f);

        entry.buttonText = _window.createText();
        entry.buttonText->setFont(*_font);
        entry.buttonText->setString("Change");
        entry.buttonText->setCharacterSize(labelSize * 0.8f);
        entry.buttonText->setFillColor(render::Color::White());
        render::FloatRect buttonTextBounds = entry.buttonText->getLocalBounds();
        float buttonTextX = _windowSize.x * 0.65f + buttonWidth / 2.f -
                            buttonTextBounds.width / 2.f;
        float buttonTextY = startY + i * spacing - buttonHeight * 0.1f +
                            buttonHeight / 2.f - buttonTextBounds.height / 2.f;
        entry.buttonText->setPosition(buttonTextX, buttonTextY);

        _bindings.push_back(std::move(entry));
    }

    _resetButton = _window.createRectangleShape(
        render::Vector2f(_windowSize.x * 0.45f, _windowSize.y * 0.07f));
    _resetButton->setFillColor(render::Color(200, 100, 50));
    _resetButton->setPosition((_windowSize.x - _windowSize.x * 0.45f) / 2.f,
                              _windowSize.y * 0.75f);

    _resetButtonText = _window.createText();
    _resetButtonText->setFont(*_font);
    _resetButtonText->setString("Reset to Defaults");
    _resetButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.035f));
    _resetButtonText->setFillColor(render::Color::White());
    render::FloatRect resetBounds = _resetButtonText->getLocalBounds();
    float resetTextX = (_windowSize.x - _windowSize.x * 0.25f) / 2.f +
                       _windowSize.x * 0.25f / 2.f - resetBounds.width / 2.f;
    float resetTextY = _windowSize.y * 0.75f + _windowSize.y * 0.07f / 2.f -
                       resetBounds.height / 2.f;
    _resetButtonText->setPosition(resetTextX, resetTextY);

    _backButton = _window.createRectangleShape(
        render::Vector2f(_windowSize.x * 0.2f, _windowSize.y * 0.08f));
    _backButton->setFillColor(render::Color(150, 50, 50));
    _backButton->setPosition((_windowSize.x - _windowSize.x * 0.2f) / 2.f,
                             _windowSize.y * 0.85f);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.05f));
    _backButtonText->setFillColor(render::Color::White());
    render::FloatRect backBounds = _backButtonText->getLocalBounds();
    float backTextX = (_windowSize.x - _windowSize.x * 0.2f) / 2.f +
                      _windowSize.x * 0.2f / 2.f - backBounds.width / 2.f;
    float backTextY = _windowSize.y * 0.85f + _windowSize.y * 0.08f / 2.f -
                      backBounds.height / 2.f;
    _backButtonText->setPosition(backTextX, backTextY);

    _waitingText = _window.createText();
    _waitingText->setFont(*_font);
    _waitingText->setString("Press a key... ESC to cancel");
    _waitingText->setCharacterSize(
        static_cast<unsigned int>(_windowSize.y * 0.04f));
    _waitingText->setFillColor(render::Color::Green());
    render::FloatRect waitingBounds = _waitingText->getLocalBounds();
    float waitingX =
        static_cast<float>(_windowSize.x) / 2.f - waitingBounds.width / 2.f;
    float waitingY = static_cast<float>(_windowSize.y) * 0.65f;
    _waitingText->setPosition(waitingX, waitingY);
}

void KeyBindingsMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void KeyBindingsMenu::startRebinding(GameAction action) {
    _waitingForKey = true;
    _currentAction = action;
    std::cout << "Waiting for key for action: " << getActionName(action)
              << std::endl;
}

void KeyBindingsMenu::handleKeyPress(render::Key key) {
    if (!_waitingForKey) {
        return;
    }

    if (key == render::Key::Escape) {
        _waitingForKey = false;
        std::cout << "Rebinding cancelled" << std::endl;
        return;
    }

    _keyBindings.setBinding(_currentAction, key);
    _waitingForKey = false;

    for (auto &binding : _bindings) {
        if (binding.action == _currentAction) {
            binding.keyText->setString(KeyBindings::getKeyName(key));
            break;
        }
    }

    std::cout << "Key binding updated: " << getActionName(_currentAction)
              << " -> " << KeyBindings::getKeyName(key) << std::endl;
}

void KeyBindingsMenu::resetToDefaults() {
    _keyBindings.resetToDefaults();

    for (auto &binding : _bindings) {
        render::Key currentKey = _keyBindings.getBinding(binding.action);
        binding.keyText->setString(KeyBindings::getKeyName(currentKey));
    }

    std::cout << "Key bindings reset to defaults" << std::endl;
}

KeyBindingsResult KeyBindingsMenu::run() {
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
                return KeyBindingsResult::Back;
            }

            if (event.type == render::EventType::Resized) {
                _windowSize = {event.size.width, event.size.height};
                updateButtonScale();
            }

            if (event.type == render::EventType::MouseMoved) {
                mousePos.x = static_cast<float>(event.mouseMove.x);
                mousePos.y = static_cast<float>(event.mouseMove.y);
            }

            if (event.type == render::EventType::KeyPressed) {
                if (_waitingForKey) {
                    handleKeyPress(event.key.code);
                }
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                if (_waitingForKey) {
                    continue;
                }

                mousePos.x = static_cast<float>(event.mouseButton.x);
                mousePos.y = static_cast<float>(event.mouseButton.y);

                for (auto &binding : _bindings) {
                    if (containsPoint(binding.button->getGlobalBounds(),
                                      mousePos)) {
                        startRebinding(binding.action);
                        break;
                    }
                }

                if (containsPoint(_resetButton->getGlobalBounds(), mousePos)) {
                    resetToDefaults();
                }

                if (containsPoint(_backButton->getGlobalBounds(), mousePos)) {
                    return KeyBindingsResult::Back;
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

    return KeyBindingsResult::None;
}

void KeyBindingsMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);

    for (auto &binding : _bindings) {
        _window.draw(*binding.label);
        _window.draw(*binding.keyText);
        _window.draw(*binding.button);
        _window.draw(*binding.buttonText);
    }

    _window.draw(*_resetButton);
    _window.draw(*_resetButtonText);

    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    if (_waitingForKey) {
        _window.draw(*_waitingText);
    }

    _window.display();
}
