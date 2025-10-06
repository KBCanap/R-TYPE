#include "../include/pause_menu.hpp"
#include <iostream>

PauseMenu::PauseMenu(render::IRenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr), _visible(false)
{
    _windowSize = _window.getSize();

    // Load font
    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf")) {
        // Use default font if custom font fails
        std::cerr << "Warning: Could not load r-type.otf font" << std::endl;
    }

    createButtons();
}

void PauseMenu::createButtons() {
    // Create semi-transparent overlay
    _overlay = _window.createRectangleShape(render::Vector2f(static_cast<float>(_windowSize.x), static_cast<float>(_windowSize.y)));
    _overlay->setFillColor(render::Color(0, 0, 0, 128)); // Semi-transparent black

    // Title
    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("PAUSE");
    _titleText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText->setFillColor(render::Color::White());
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    // Center text by adjusting position (IText doesn't have setOrigin)
    float titleX = static_cast<float>(_windowSize.x) / 2.f - titleBounds.width / 2.f;
    float titleY = static_cast<float>(_windowSize.y) * 0.25f - titleBounds.height / 2.f;
    _titleText->setPosition(titleX, titleY);

    float buttonWidth = _windowSize.x * 0.25f;
    float buttonHeight = _windowSize.y * 0.08f;
    float startY = _windowSize.y * 0.4f;
    float spacing = buttonHeight * 1.3f;

    // Continue button
    _continueButton = _window.createRectangleShape(render::Vector2f(buttonWidth, buttonHeight));
    _continueButton->setFillColor(render::Color(100, 200, 100));
    _continueButton->setPosition((_windowSize.x - buttonWidth) / 2.f, startY);

    _continueButtonText = _window.createText();
    _continueButtonText->setFont(*_font);
    _continueButtonText->setString("Continue");
    _continueButtonText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _continueButtonText->setFillColor(render::Color::White());
    render::FloatRect continueBounds = _continueButtonText->getLocalBounds();
    // Center text in button
    float continueTextX = (_windowSize.x - buttonWidth) / 2.f + buttonWidth / 2.f - continueBounds.width / 2.f;
    float continueTextY = startY + buttonHeight / 2.f - continueBounds.height / 2.f;
    _continueButtonText->setPosition(continueTextX, continueTextY);

    // Options button
    _optionsButton = _window.createRectangleShape(render::Vector2f(buttonWidth, buttonHeight));
    _optionsButton->setFillColor(render::Color(100, 150, 200));
    _optionsButton->setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing);

    _optionsButtonText = _window.createText();
    _optionsButtonText->setFont(*_font);
    _optionsButtonText->setString("Options");
    _optionsButtonText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _optionsButtonText->setFillColor(render::Color::White());
    render::FloatRect optionsBounds = _optionsButtonText->getLocalBounds();
    // Center text in button
    float optionsTextX = (_windowSize.x - buttonWidth) / 2.f + buttonWidth / 2.f - optionsBounds.width / 2.f;
    float optionsTextY = startY + spacing + buttonHeight / 2.f - optionsBounds.height / 2.f;
    _optionsButtonText->setPosition(optionsTextX, optionsTextY);

    // Accessibility button
    _accessibilityButton = _window.createRectangleShape(render::Vector2f(buttonWidth, buttonHeight));
    _accessibilityButton->setFillColor(render::Color(150, 100, 200));
    _accessibilityButton->setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing * 2);

    _accessibilityButtonText = _window.createText();
    _accessibilityButtonText->setFont(*_font);
    _accessibilityButtonText->setString("Accessibility");
    _accessibilityButtonText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _accessibilityButtonText->setFillColor(render::Color::White());
    render::FloatRect accessibilityBounds = _accessibilityButtonText->getLocalBounds();
    // Center text in button
    float accessibilityTextX = (_windowSize.x - buttonWidth) / 2.f + buttonWidth / 2.f - accessibilityBounds.width / 2.f;
    float accessibilityTextY = startY + spacing * 2 + buttonHeight / 2.f - accessibilityBounds.height / 2.f;
    _accessibilityButtonText->setPosition(accessibilityTextX, accessibilityTextY);

    // Quit button
    _quitButton = _window.createRectangleShape(render::Vector2f(buttonWidth, buttonHeight));
    _quitButton->setFillColor(render::Color(200, 100, 100));
    _quitButton->setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing * 3);

    _quitButtonText = _window.createText();
    _quitButtonText->setFont(*_font);
    _quitButtonText->setString("Quit");
    _quitButtonText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _quitButtonText->setFillColor(render::Color::White());
    render::FloatRect quitBounds = _quitButtonText->getLocalBounds();
    // Center text in button
    float quitTextX = (_windowSize.x - buttonWidth) / 2.f + buttonWidth / 2.f - quitBounds.width / 2.f;
    float quitTextY = startY + spacing * 3 + buttonHeight / 2.f - quitBounds.height / 2.f;
    _quitButtonText->setPosition(quitTextX, quitTextY);
}

void PauseMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

PauseResult PauseMenu::run() {
    // This method is now simplified since events are handled in Game class
    // Just render and return None to keep the pause state
    render();
    return PauseResult::None;
}

void PauseMenu::render() {
    if (!_visible) {
        std::cout << "PauseMenu::render() - not visible, returning" << std::endl;
        return;
    }

    std::cout << "PauseMenu::render() - drawing pause menu" << std::endl;

    // Draw overlay
    _window.draw(*_overlay);

    // Draw title
    _window.draw(*_titleText);

    // Draw buttons
    _window.draw(*_continueButton);
    _window.draw(*_continueButtonText);
    _window.draw(*_optionsButton);
    _window.draw(*_optionsButtonText);
    _window.draw(*_accessibilityButton);
    _window.draw(*_accessibilityButtonText);
    _window.draw(*_quitButton);
    _window.draw(*_quitButtonText);
}