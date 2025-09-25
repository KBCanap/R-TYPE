#pragma once
#include <SFML/Graphics.hpp>
#include "audio_manager.hpp"
#include <vector>

enum class OptionsResult { None, Back, ResolutionChanged, SoundToggled };

struct Resolution {
    unsigned int width;
    unsigned int height;
    std::string name;
};

class OptionsMenu {
public:
    OptionsMenu(sf::RenderWindow& win, AudioManager& audioMgr);

    OptionsResult run();
    void render();

    // Getters for settings
    Resolution getCurrentResolution() const { return _resolutions[_currentResolution]; }
    bool isSoundEnabled() const { return _soundEnabled; }

private:
    void createButtons();
    void updateButtonScale();
    void handleResolutionChange();
    void toggleSound();

private:
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    sf::Font _font;
    sf::Text _titleText;

    // Resolution settings
    std::vector<Resolution> _resolutions;
    size_t _currentResolution;
    sf::Text _resolutionLabel;
    sf::Text _resolutionValue;
    sf::RectangleShape _resolutionLeftButton;
    sf::RectangleShape _resolutionRightButton;
    sf::Text _resolutionLeftText;
    sf::Text _resolutionRightText;

    // Sound settings
    bool _soundEnabled;
    sf::Text _soundLabel;
    sf::Text _soundValue;
    sf::RectangleShape _soundButton;
    sf::Text _soundButtonText;

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