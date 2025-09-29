#include "../include/accessibility_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

AccessibilityMenu::AccessibilityMenu(sf::RenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr), _currentContrast(Settings::getInstance().getContrast()),
      _isDraggingContrastSlider(false), _contrastSliderMin(0.1f), _contrastSliderMax(3.0f)
{
    _windowSize = _window.getSize();

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

void AccessibilityMenu::createButtons() {
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
    _titleText.setString("GRAPHISMS");
    _titleText.setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = _titleText.getLocalBounds();
    _titleText.setOrigin(titleBounds.left + titleBounds.width/2.f, titleBounds.top + titleBounds.height/2.f);
    _titleText.setPosition(static_cast<float>(_windowSize.x) / 2.f, static_cast<float>(_windowSize.y) * 0.15f);

    float labelSize = static_cast<unsigned int>(_windowSize.y * 0.04f);

    // Reference square for contrast testing (centered)
    float squareSize = _windowSize.y * 0.1f;
    float squareY = _windowSize.y * 0.25f;

    _referenceSquare.setSize(sf::Vector2f(squareSize, squareSize));
    _referenceSquare.setFillColor(sf::Color(128, 128, 128)); // Gray color for better contrast testing
    _referenceSquare.setPosition((_windowSize.x - squareSize) / 2.f, squareY);

    _referenceLabel.setFont(_font);
    _referenceLabel.setString("Reference square");
    _referenceLabel.setCharacterSize(labelSize * 0.8f);
    _referenceLabel.setFillColor(sf::Color::White);
    sf::FloatRect labelBounds = _referenceLabel.getLocalBounds();
    _referenceLabel.setPosition((_windowSize.x - labelBounds.width) / 2.f, squareY - labelSize);

    // Contrast settings
    _contrastLabel.setFont(_font);
    _contrastLabel.setString("Contrast ");
    _contrastLabel.setCharacterSize(labelSize);
    _contrastLabel.setFillColor(sf::Color::White);
    _contrastLabel.setPosition(_windowSize.x * 0.15f, _windowSize.y * 0.45f);

    _contrastValue.setFont(_font);
    _contrastValue.setString(std::to_string(static_cast<int>(_currentContrast * 100)));
    _contrastValue.setCharacterSize(labelSize);
    _contrastValue.setFillColor(sf::Color::Yellow);
    _contrastValue.setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.45f);

    // Contrast slider track
    float sliderWidth = _windowSize.x * 0.4f;
    float sliderHeight = _windowSize.y * 0.02f;
    _contrastSliderTrack.setSize(sf::Vector2f(sliderWidth, sliderHeight));
    _contrastSliderTrack.setFillColor(sf::Color(100, 100, 100));
    _contrastSliderTrack.setPosition(_windowSize.x * 0.3f, _windowSize.y * 0.55f);

    // Contrast slider handle
    float handleRadius = _windowSize.y * 0.025f;
    _contrastSliderHandle.setRadius(handleRadius);
    _contrastSliderHandle.setFillColor(sf::Color(200, 200, 200));
    _contrastSliderHandle.setOrigin(handleRadius, handleRadius);

    // Position slider handle based on current contrast value
    updateContrastSliderPosition();

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

void AccessibilityMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void AccessibilityMenu::updateContrastSlider(float mouseX) {
    float trackLeft = _contrastSliderTrack.getPosition().x;
    float trackWidth = _contrastSliderTrack.getSize().x;

    // Calculate new contrast value based on mouse position
    float relativePos = (mouseX - trackLeft) / trackWidth;
    relativePos = std::max(0.0f, std::min(1.0f, relativePos));

    _currentContrast = _contrastSliderMin + relativePos * (_contrastSliderMax - _contrastSliderMin);

    // Update the settings singleton
    Settings::getInstance().setContrast(_currentContrast);

    // Update the display value
    _contrastValue.setString(std::to_string(static_cast<int>(_currentContrast * 100)));

    // Update slider handle position
    updateContrastSliderPosition();

    std::cout << "Contrast set to: " << (_currentContrast * 100) << "%" << std::endl;
}

void AccessibilityMenu::updateContrastSliderPosition() {
    float trackLeft = _contrastSliderTrack.getPosition().x;
    float trackWidth = _contrastSliderTrack.getSize().x;
    float trackCenter = _contrastSliderTrack.getPosition().y + _contrastSliderTrack.getSize().y / 2.0f;

    // Calculate handle position based on current contrast value
    float relativePos = (_currentContrast - _contrastSliderMin) / (_contrastSliderMax - _contrastSliderMin);
    float handleX = trackLeft + relativePos * trackWidth;

    _contrastSliderHandle.setPosition(handleX, trackCenter);
}

AccessibilityResult AccessibilityMenu::run() {
    sf::Clock clock;
    bool running = true;

    while (running) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return AccessibilityResult::Back;
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P) {
                    return AccessibilityResult::Resumed;
                }
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

                // Check contrast slider
                if (_contrastSliderTrack.getGlobalBounds().contains(mousePos) ||
                    _contrastSliderHandle.getGlobalBounds().contains(mousePos)) {
                    _isDraggingContrastSlider = true;
                    updateContrastSlider(mousePos.x);
                }

                // Check back button
                if (_backButton.getGlobalBounds().contains(mousePos)) {
                    return AccessibilityResult::Back;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                _isDraggingContrastSlider = false;
            }

            if (event.type == sf::Event::MouseMoved && _isDraggingContrastSlider) {
                sf::Vector2f mousePos(static_cast<float>(sf::Mouse::getPosition(_window).x),
                                     static_cast<float>(sf::Mouse::getPosition(_window).y));
                updateContrastSlider(mousePos.x);
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

    return AccessibilityResult::None;
}

void AccessibilityMenu::render() {
    _window.clear();

    // Draw scrolling background
    _window.draw(_bgSprite1);
    _window.draw(_bgSprite2);

    // Draw title
    _window.draw(_titleText);

    // Draw reference square with contrast applied
    Settings& settings = Settings::getInstance();
    sf::Shader* contrastShader = settings.getContrastShader();

    _window.draw(_referenceLabel);

    if (contrastShader) {
        _window.draw(_referenceSquare, contrastShader);
    } else {
        // Fallback: use color modulation
        sf::RectangleShape referenceSquareCopy = _referenceSquare;
        sf::Color grayWithContrast = settings.applyContrast(sf::Color(128, 128, 128));
        referenceSquareCopy.setFillColor(grayWithContrast);
        _window.draw(referenceSquareCopy);
    }

    // Draw contrast settings
    _window.draw(_contrastLabel);
    _window.draw(_contrastValue);
    _window.draw(_contrastSliderTrack);
    _window.draw(_contrastSliderHandle);

    // Draw back button
    _window.draw(_backButton);
    _window.draw(_backButtonText);

    _window.display();
}