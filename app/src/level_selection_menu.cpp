/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** level_selection_menu
*/

#include "../include/level_selection_menu.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect &rect,
                          const render::Vector2f &point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

LevelSelectionMenu::LevelSelectionMenu(render::IRenderWindow &window,
                                       AudioManager &audioMgr)
    : _window(window), _audioManager(audioMgr), _selectedButton(0) {
    _buttonLabels = {"LEVEL 1", "LEVEL 2", "BACK"};
    initializeMenu();
}

void LevelSelectionMenu::initializeMenu() {
    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");

    render::Vector2u windowSize = _window.getSize();
    float centerX = windowSize.x / 2.0f;
    float startY = windowSize.y / 2.0f - 100.0f;

    // Load background
    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg")) {
        auto fallback_image = _window.createImage();
        fallback_image->create(windowSize.x, windowSize.y,
                               render::Color(20, 20, 80));
        _bgTexture->loadFromImage(*fallback_image);
    }

    // Create two sprites for scrolling effect
    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    _bgSprite1->setTexture(*_bgTexture);
    _bgSprite2->setTexture(*_bgTexture);

    // Scale background to fit window
    auto texSize = _bgTexture->getSize();
    float scaleX = static_cast<float>(windowSize.x) / texSize.x;
    float scaleY = static_cast<float>(windowSize.y) / texSize.y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite2->setPosition(static_cast<float>(windowSize.x), 0.f);

    // Set scroll speed
    _bgScrollSpeed = windowSize.x * 0.125f;

    // Title
    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("SELECT LEVEL");
    _titleText->setCharacterSize(60);
    _titleText->setFillColor(render::Color::White());
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, 100);

    // Create buttons
    _buttons.clear();
    _buttonTexts.clear();

    for (size_t i = 0; i < _buttonLabels.size(); ++i) {
        // Create button shape
        auto button = _window.createRectangleShape(render::Vector2f(300, 60));
        button->setPosition(centerX - 150, startY + i * 80);
        button->setFillColor(NORMAL_BUTTON_COLOR);
        _buttons.push_back(std::move(button));

        // Create button text
        auto text = _window.createText();
        text->setFont(*_font);
        text->setString(_buttonLabels[i]);
        text->setCharacterSize(32);
        text->setFillColor(TEXT_COLOR);
        render::FloatRect textBounds = text->getLocalBounds();
        text->setPosition(centerX - textBounds.width / 2,
                          startY + i * 80 + 15);
        _buttonTexts.push_back(std::move(text));
    }

    updateButtonColors();
}

void LevelSelectionMenu::handleEvents(bool &running) {
    render::Event event;
    while (_window.pollEvent(event)) {
        if (event.type == render::EventType::Closed) {
            running = false;
            return;
        }

        if (event.type == render::EventType::KeyPressed) {
            switch (event.key.code) {
            case render::Key::Up:
                _selectedButton =
                    (_selectedButton - 1 + _buttonLabels.size()) %
                    _buttonLabels.size();
                updateButtonColors();
                break;
            case render::Key::Down:
                _selectedButton = (_selectedButton + 1) % _buttonLabels.size();
                updateButtonColors();
                break;
            case render::Key::Enter:
            case render::Key::Space:
                running = false;
                break;
            case render::Key::Escape:
                _selectedButton = 2; // Back button
                running = false;
                break;
            default:
                break;
            }
        } else if (event.type == render::EventType::MouseButtonPressed &&
                   event.mouseButton.button == render::Mouse::Left) {
            render::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                                      static_cast<float>(event.mouseButton.y));

            for (size_t i = 0; i < _buttons.size(); ++i) {
                if (containsPoint(_buttons[i]->getGlobalBounds(), mousePos)) {
                    _selectedButton = static_cast<int>(i);
                    running = false;
                    break;
                }
            }
        } else if (event.type == render::EventType::MouseMoved) {
            render::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                                      static_cast<float>(event.mouseMove.y));

            for (size_t i = 0; i < _buttons.size(); ++i) {
                if (containsPoint(_buttons[i]->getGlobalBounds(), mousePos)) {
                    if (_selectedButton != static_cast<int>(i)) {
                        _selectedButton = static_cast<int>(i);
                        updateButtonColors();
                    }
                    break;
                }
            }
        }
    }
}

void LevelSelectionMenu::update(float dt) {
    // Background scrolling
    auto bg1Pos = _bgSprite1->getPosition();
    auto bg2Pos = _bgSprite2->getPosition();

    _bgSprite1->setPosition(bg1Pos.x - _bgScrollSpeed * dt, bg1Pos.y);
    _bgSprite2->setPosition(bg2Pos.x - _bgScrollSpeed * dt, bg2Pos.y);

    // Reset positions when sprites go off screen
    bg1Pos = _bgSprite1->getPosition();
    bg2Pos = _bgSprite2->getPosition();
    render::Vector2u windowSize = _window.getSize();

    if (bg1Pos.x + windowSize.x < 0)
        _bgSprite1->setPosition(bg2Pos.x + windowSize.x, 0.f);
    if (bg2Pos.x + windowSize.x < 0)
        _bgSprite2->setPosition(bg1Pos.x + windowSize.x, 0.f);
}

void LevelSelectionMenu::render() {
    _window.clear();

    // Draw both background sprites for scrolling effect
    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    // Draw title
    _window.draw(*_titleText);

    // Draw buttons and texts
    for (size_t i = 0; i < _buttons.size(); ++i) {
        _window.draw(*_buttons[i]);
        _window.draw(*_buttonTexts[i]);
    }

    _window.display();
}

void LevelSelectionMenu::updateButtonColors() {
    for (size_t i = 0; i < _buttons.size(); ++i) {
        if (static_cast<int>(i) == _selectedButton) {
            _buttons[i]->setFillColor(SELECTED_BUTTON_COLOR);
            _buttonTexts[i]->setFillColor(SELECTED_TEXT_COLOR);
        } else {
            _buttons[i]->setFillColor(NORMAL_BUTTON_COLOR);
            _buttonTexts[i]->setFillColor(TEXT_COLOR);
        }
    }
}

bool LevelSelectionMenu::isMouseOverButton(
    int buttonIndex, const render::Vector2f &mousePos) const {
    if (buttonIndex < 0 || buttonIndex >= static_cast<int>(_buttons.size()))
        return false;
    return containsPoint(_buttons[buttonIndex]->getGlobalBounds(), mousePos);
}

LevelSelectionResult LevelSelectionMenu::run() {

    bool running = true;
    auto lastTime = std::chrono::steady_clock::now();

    while (running && _window.isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        handleEvents(running);
        update(dt);
        render();
    }

    // Return result based on selected button
    switch (_selectedButton) {
    case 0:
        return LevelSelectionResult::LEVEL_1;
    case 1:
        return LevelSelectionResult::LEVEL_2;
    case 2:
    default:
        return LevelSelectionResult::BACK;
    }
}
