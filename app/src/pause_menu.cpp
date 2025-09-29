#include "../include/pause_menu.hpp"
#include <iostream>

PauseMenu::PauseMenu(sf::RenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr), _visible(false)
{
    _windowSize = _window.getSize();

    // Load font
    if (!_font.loadFromFile("assets/r-type.otf")) {
        // Use default font if custom font fails
        std::cerr << "Warning: Could not load r-type.otf font" << std::endl;
    }

    createButtons();
}

void PauseMenu::createButtons() {
    // Create semi-transparent overlay
    _overlay.setSize(sf::Vector2f(static_cast<float>(_windowSize.x), static_cast<float>(_windowSize.y)));
    _overlay.setFillColor(sf::Color(0, 0, 0, 128)); // Semi-transparent black

    // Title
    _titleText.setFont(_font);
    _titleText.setString("PAUSE");
    _titleText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = _titleText.getLocalBounds();
    _titleText.setOrigin(titleBounds.left + titleBounds.width/2.f, titleBounds.top + titleBounds.height/2.f);
    _titleText.setPosition(static_cast<float>(_windowSize.x) / 2.f, static_cast<float>(_windowSize.y) * 0.25f);

    float buttonWidth = _windowSize.x * 0.25f;
    float buttonHeight = _windowSize.y * 0.08f;
    float startY = _windowSize.y * 0.4f;
    float spacing = buttonHeight * 1.3f;

    // Continue button
    _continueButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _continueButton.setFillColor(sf::Color(100, 200, 100));
    _continueButton.setPosition((_windowSize.x - buttonWidth) / 2.f, startY);

    _continueButtonText.setFont(_font);
    _continueButtonText.setString("Continue");
    _continueButtonText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _continueButtonText.setFillColor(sf::Color::White);
    sf::FloatRect continueBounds = _continueButtonText.getLocalBounds();
    _continueButtonText.setOrigin(continueBounds.left + continueBounds.width/2.f, continueBounds.top + continueBounds.height/2.f);
    _continueButtonText.setPosition(_continueButton.getPosition().x + buttonWidth/2.f, _continueButton.getPosition().y + buttonHeight/2.f);

    // Options button
    _optionsButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _optionsButton.setFillColor(sf::Color(100, 150, 200));
    _optionsButton.setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing);

    _optionsButtonText.setFont(_font);
    _optionsButtonText.setString("Options");
    _optionsButtonText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _optionsButtonText.setFillColor(sf::Color::White);
    sf::FloatRect optionsBounds = _optionsButtonText.getLocalBounds();
    _optionsButtonText.setOrigin(optionsBounds.left + optionsBounds.width/2.f, optionsBounds.top + optionsBounds.height/2.f);
    _optionsButtonText.setPosition(_optionsButton.getPosition().x + buttonWidth/2.f, _optionsButton.getPosition().y + buttonHeight/2.f);

    // Accessibility button
    _accessibilityButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _accessibilityButton.setFillColor(sf::Color(150, 100, 200));
    _accessibilityButton.setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing * 2);

    _accessibilityButtonText.setFont(_font);
    _accessibilityButtonText.setString("Accessibility");
    _accessibilityButtonText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _accessibilityButtonText.setFillColor(sf::Color::White);
    sf::FloatRect accessibilityBounds = _accessibilityButtonText.getLocalBounds();
    _accessibilityButtonText.setOrigin(accessibilityBounds.left + accessibilityBounds.width/2.f, accessibilityBounds.top + accessibilityBounds.height/2.f);
    _accessibilityButtonText.setPosition(_accessibilityButton.getPosition().x + buttonWidth/2.f, _accessibilityButton.getPosition().y + buttonHeight/2.f);

    // Quit button
    _quitButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _quitButton.setFillColor(sf::Color(200, 100, 100));
    _quitButton.setPosition((_windowSize.x - buttonWidth) / 2.f, startY + spacing * 3);

    _quitButtonText.setFont(_font);
    _quitButtonText.setString("Quit");
    _quitButtonText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.04f));
    _quitButtonText.setFillColor(sf::Color::White);
    sf::FloatRect quitBounds = _quitButtonText.getLocalBounds();
    _quitButtonText.setOrigin(quitBounds.left + quitBounds.width/2.f, quitBounds.top + quitBounds.height/2.f);
    _quitButtonText.setPosition(_quitButton.getPosition().x + buttonWidth/2.f, _quitButton.getPosition().y + buttonHeight/2.f);
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
    _window.draw(_overlay);

    // Draw title
    _window.draw(_titleText);

    // Draw buttons
    _window.draw(_continueButton);
    _window.draw(_continueButtonText);
    _window.draw(_optionsButton);
    _window.draw(_optionsButtonText);
    _window.draw(_accessibilityButton);
    _window.draw(_accessibilityButtonText);
    _window.draw(_quitButton);
    _window.draw(_quitButtonText);
}