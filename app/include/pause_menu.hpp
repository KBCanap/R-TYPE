#pragma once
#include <SFML/Graphics.hpp>
#include "audio_manager.hpp"
#include "options_menu.hpp"
#include "accessibility_menu.hpp"

enum class PauseResult { None, Continue, Options, Accessibility, Quit };

class PauseMenu {
public:
    PauseMenu(sf::RenderWindow& win, AudioManager& audioMgr);

    PauseResult run();
    void render();
    void setVisible(bool visible) { _visible = visible; }
    bool isVisible() const { return _visible; }

    // Getters for button access
    const sf::RectangleShape& getContinueButton() const { return _continueButton; }
    const sf::RectangleShape& getOptionsButton() const { return _optionsButton; }
    const sf::RectangleShape& getAccessibilityButton() const { return _accessibilityButton; }
    const sf::RectangleShape& getQuitButton() const { return _quitButton; }

private:
    void createButtons();
    void updateButtonScale();

private:
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    sf::Font _font;
    sf::Text _titleText;

    // Buttons
    sf::RectangleShape _continueButton;
    sf::RectangleShape _optionsButton;
    sf::RectangleShape _accessibilityButton;
    sf::RectangleShape _quitButton;

    sf::Text _continueButtonText;
    sf::Text _optionsButtonText;
    sf::Text _accessibilityButtonText;
    sf::Text _quitButtonText;

    // Semi-transparent overlay
    sf::RectangleShape _overlay;

    sf::Vector2u _windowSize;
    bool _visible;
};