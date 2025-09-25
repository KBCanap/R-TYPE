#include "../include/options_menu.hpp"
#include <iostream>

OptionsMenu::OptionsMenu(sf::RenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr), _currentResolution(1), _soundEnabled(true)
{
    _windowSize = _window.getSize();

    // Initialize available resolutions
    _resolutions = {
        {800, 600, "800x600"},
        {1024, 768, "1024x768"},
        {1280, 720, "1280x720"}
    };

    // Find current resolution
    for (size_t i = 0; i < _resolutions.size(); ++i) {
        if (_resolutions[i].width == _windowSize.x && _resolutions[i].height == _windowSize.y) {
            _currentResolution = i;
            break;
        }
    }

    // Load background
    if (!_bgTexture.loadFromFile("assets/background.jpg")) {
        // Create a fallback background if file doesn't exist
        sf::Image fallback;
        fallback.create(800, 600, sf::Color(20, 20, 80));
        _bgTexture.loadFromImage(fallback);
    }

    // Initialize scrolling background sprites
    _bgSprite1.setTexture(_bgTexture);
    _bgSprite2.setTexture(_bgTexture);
    _bgScrollSpeed = 100.0f;

    // Load font
    if (!_font.loadFromFile("assets/r-type.otf")) {
        // Use default font if custom font fails
        std::cerr << "Warning: Could not load r-type.otf font" << std::endl;
    }

    createButtons();
}

void OptionsMenu::createButtons() {
    // Scale background for scrolling
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture.getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture.getSize().y;
    _bgSprite1.setScale(scaleX, scaleY);
    _bgSprite2.setScale(scaleX, scaleY);
    _bgSprite1.setPosition(0.f, 0.f);
    _bgSprite2.setPosition(static_cast<float>(_windowSize.x), 0.f);

    // Adjust scroll speed proportionally to window size
    _bgScrollSpeed = _windowSize.x * 0.125f;

    // Title
    _titleText.setFont(_font);
    _titleText.setString("OPTIONS");
    _titleText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = _titleText.getLocalBounds();
    _titleText.setOrigin(titleBounds.left + titleBounds.width/2.f, titleBounds.top + titleBounds.height/2.f);
    _titleText.setPosition(static_cast<float>(_windowSize.x) / 2.f, static_cast<float>(_windowSize.y) * 0.15f);

    float labelSize = static_cast<unsigned int>(_windowSize.y * 0.04f);
    float buttonWidth = _windowSize.x * 0.08f;
    float buttonHeight = _windowSize.y * 0.06f;

    // Resolution settings
    _resolutionLabel.setFont(_font);
    _resolutionLabel.setString("Resolution ");
    _resolutionLabel.setCharacterSize(labelSize);
    _resolutionLabel.setFillColor(sf::Color::White);
    _resolutionLabel.setPosition(_windowSize.x * 0.15f, _windowSize.y * 0.28f);

    _resolutionValue.setFont(_font);
    _resolutionValue.setString(_resolutions[_currentResolution].name);
    _resolutionValue.setCharacterSize(labelSize);
    _resolutionValue.setFillColor(sf::Color::Yellow);
    _resolutionValue.setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.28f);

    // Left arrow button - positioned below the text
    _resolutionLeftButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _resolutionLeftButton.setFillColor(sf::Color(100, 100, 200));
    _resolutionLeftButton.setPosition(_windowSize.x * 0.35f, _windowSize.y * 0.40f);

    _resolutionLeftText.setFont(_font);
    _resolutionLeftText.setCharacterSize(labelSize);
    _resolutionLeftText.setFillColor(sf::Color::White);
    sf::FloatRect leftBounds = _resolutionLeftText.getLocalBounds();
    _resolutionLeftText.setOrigin(leftBounds.left + leftBounds.width/2.f, leftBounds.top + leftBounds.height/2.f);
    _resolutionLeftText.setPosition(_resolutionLeftButton.getPosition().x + buttonWidth/2.f, _resolutionLeftButton.getPosition().y + buttonHeight/2.f);

    // Right arrow button - positioned below the text
    _resolutionRightButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    _resolutionRightButton.setFillColor(sf::Color(100, 100, 200));
    _resolutionRightButton.setPosition(_windowSize.x * 0.65f, _windowSize.y * 0.40f);

    _resolutionRightText.setFont(_font);
    _resolutionRightText.setCharacterSize(labelSize);
    _resolutionRightText.setFillColor(sf::Color::White);
    sf::FloatRect rightBounds = _resolutionRightText.getLocalBounds();
    _resolutionRightText.setOrigin(rightBounds.left + rightBounds.width/2.f, rightBounds.top + rightBounds.height/2.f);
    _resolutionRightText.setPosition(_resolutionRightButton.getPosition().x + buttonWidth/2.f, _resolutionRightButton.getPosition().y + buttonHeight/2.f);

    // Sound settings
    _soundLabel.setFont(_font);
    _soundLabel.setString("Sound ");
    _soundLabel.setCharacterSize(labelSize);
    _soundLabel.setFillColor(sf::Color::White);
    _soundLabel.setPosition(_windowSize.x * 0.2f, _windowSize.y * 0.5f);

    _soundValue.setFont(_font);
    _soundValue.setString(_soundEnabled ? "ON" : "OFF");
    _soundValue.setCharacterSize(labelSize);
    _soundValue.setFillColor(_soundEnabled ? sf::Color::Green : sf::Color::Red);
    _soundValue.setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.5f);

    _soundButton.setSize(sf::Vector2f(buttonWidth * 2.f, buttonHeight));
    _soundButton.setFillColor(sf::Color(100, 100, 200));
    _soundButton.setPosition(_windowSize.x * 0.6f, _windowSize.y * 0.48f);

    _soundButtonText.setFont(_font);
    _soundButtonText.setString("Toggle");
    _soundButtonText.setCharacterSize(labelSize * 0.8f);
    _soundButtonText.setFillColor(sf::Color::White);
    sf::FloatRect soundBounds = _soundButtonText.getLocalBounds();
    _soundButtonText.setOrigin(soundBounds.left + soundBounds.width/2.f, soundBounds.top + soundBounds.height/2.f);
    _soundButtonText.setPosition(_soundButton.getPosition().x + buttonWidth, _soundButton.getPosition().y + buttonHeight/2.f);

    // Back button
    _backButton.setSize(sf::Vector2f(_windowSize.x * 0.2f, _windowSize.y * 0.08f));
    _backButton.setFillColor(sf::Color(150, 50, 50));
    _backButton.setPosition((_windowSize.x - _backButton.getSize().x) / 2.f, _windowSize.y * 0.75f);

    _backButtonText.setFont(_font);
    _backButtonText.setString("Back");
    _backButtonText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.05f));
    _backButtonText.setFillColor(sf::Color::White);
    sf::FloatRect backBounds = _backButtonText.getLocalBounds();
    _backButtonText.setOrigin(backBounds.left + backBounds.width/2.f, backBounds.top + backBounds.height/2.f);
    _backButtonText.setPosition(_backButton.getPosition().x + _backButton.getSize().x/2.f, _backButton.getPosition().y + _backButton.getSize().y/2.f);
}

void OptionsMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void OptionsMenu::handleResolutionChange() {
    Resolution newRes = _resolutions[_currentResolution];

    // Resize the window
    _window.create(sf::VideoMode(newRes.width, newRes.height), "R-Type", sf::Style::Close | sf::Style::Resize);

    // Update window size and recreate buttons
    updateButtonScale();

    std::cout << "Resolution changed to: " << newRes.name << std::endl;
}

void OptionsMenu::toggleSound() {
    _soundEnabled = !_soundEnabled;
    _soundValue.setString(_soundEnabled ? "ON" : "OFF");
    _soundValue.setFillColor(_soundEnabled ? sf::Color::Green : sf::Color::Red);

    // Apply sound setting to AudioManager
    if (_soundEnabled) {
        _audioManager.setMasterVolume(100.0f);
    } else {
        _audioManager.setMasterVolume(0.0f);
    }

    std::cout << "Sound " << (_soundEnabled ? "enabled" : "disabled") << std::endl;
}

OptionsResult OptionsMenu::run() {
    sf::Clock clock;
    bool running = true;

    while (running) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return OptionsResult::Back;
            }

            if (event.type == sf::Event::Resized) {
                sf::View view;
                view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
                view.setCenter(static_cast<float>(event.size.width) / 2.f, static_cast<float>(event.size.height) / 2.f);
                _window.setView(view);
                updateButtonScale();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(static_cast<float>(sf::Mouse::getPosition(_window).x),
                                     static_cast<float>(sf::Mouse::getPosition(_window).y));

                // Check resolution left button
                if (_resolutionLeftButton.getGlobalBounds().contains(mousePos)) {
                    if (_currentResolution > 0) {
                        _currentResolution--;
                        _resolutionValue.setString(_resolutions[_currentResolution].name);
                        handleResolutionChange();
                    }
                }

                // Check resolution right button
                if (_resolutionRightButton.getGlobalBounds().contains(mousePos)) {
                    if (_currentResolution < _resolutions.size() - 1) {
                        _currentResolution++;
                        _resolutionValue.setString(_resolutions[_currentResolution].name);
                        handleResolutionChange();
                    }
                }

                // Check sound toggle button
                if (_soundButton.getGlobalBounds().contains(mousePos)) {
                    toggleSound();
                }

                // Check back button
                if (_backButton.getGlobalBounds().contains(mousePos)) {
                    return OptionsResult::Back;
                }
            }
        }

        // Background scrolling
        _bgSprite1.move(-_bgScrollSpeed * dt, 0.f);
        _bgSprite2.move(-_bgScrollSpeed * dt, 0.f);
        if (_bgSprite1.getPosition().x + _windowSize.x < 0)
            _bgSprite1.setPosition(_bgSprite2.getPosition().x + _windowSize.x, 0.f);
        if (_bgSprite2.getPosition().x + _windowSize.x < 0)
            _bgSprite2.setPosition(_bgSprite1.getPosition().x + _windowSize.x, 0.f);

        render();
    }

    return OptionsResult::None;
}

void OptionsMenu::render() {
    _window.clear();

    // Draw scrolling background
    _window.draw(_bgSprite1);
    _window.draw(_bgSprite2);

    // Draw title
    _window.draw(_titleText);

    // Draw resolution settings
    _window.draw(_resolutionLabel);
    _window.draw(_resolutionValue);
    _window.draw(_resolutionLeftButton);
    _window.draw(_resolutionLeftText);
    _window.draw(_resolutionRightButton);
    _window.draw(_resolutionRightText);

    // Draw sound settings
    _window.draw(_soundLabel);
    _window.draw(_soundValue);
    _window.draw(_soundButton);
    _window.draw(_soundButtonText);

    // Draw back button
    _window.draw(_backButton);
    _window.draw(_backButtonText);

    _window.display();
}