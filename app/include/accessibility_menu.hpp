#pragma once
#include <SFML/Graphics.hpp>
#include "audio_manager.hpp"

enum class AccessibilityResult { None, Back, Resumed };

class AccessibilityMenu {
public:
    AccessibilityMenu(sf::RenderWindow& win, AudioManager& audioMgr);

    AccessibilityResult run();
    void render();

private:
    void createButtons();
    void updateButtonScale();
    void updateContrastSlider(float mouseX);
    void updateContrastSliderPosition();

private:
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    sf::Font _font;
    sf::Text _titleText;

    // Contrast settings
    float _currentContrast;
    sf::Text _contrastLabel;
    sf::Text _contrastValue;
    sf::RectangleShape _contrastSliderTrack;
    sf::CircleShape _contrastSliderHandle;
    bool _isDraggingContrastSlider;
    float _contrastSliderMin;
    float _contrastSliderMax;

    // Reference square for contrast testing
    sf::RectangleShape _referenceSquare;
    sf::Text _referenceLabel;

    // Back button
    sf::RectangleShape _backButton;
    sf::Text _backButtonText;

    sf::Vector2u _windowSize;

    // Background (scrolling)
    sf::Texture _bgTexture;
    sf::Sprite _bgSprite1;
    sf::Sprite _bgSprite2;
    float _bgScrollSpeed;
};