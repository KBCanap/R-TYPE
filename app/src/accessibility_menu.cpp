#include "../include/accessibility_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>
#include <chrono>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect& rect, const render::Vector2f& point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

AccessibilityMenu::AccessibilityMenu(render::IRenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr), _currentContrast(Settings::getInstance().getContrast()),
      _isDraggingContrastSlider(false), _contrastSliderMin(0.1f), _contrastSliderMax(3.0f)
{
    _windowSize = _window.getSize();

    // Load background
    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg")) {
        // Note: Cannot create fallback image with render interface
        // Will use a simple color-filled background instead
        std::cerr << "Warning: Could not load background.jpg" << std::endl;
    }

    // Initialize scrolling background sprites
    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    if (_bgTexture->loadFromFile("assets/background.jpg")) {
        _bgSprite1->setTexture(*_bgTexture);
        _bgSprite2->setTexture(*_bgTexture);
    }
    _bgScrollSpeed = 100.0f;

    // Load font
    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf")) {
        // Use default font if custom font fails
        std::cerr << "Warning: Could not load r-type.otf font" << std::endl;
    }

    createButtons();
}

void AccessibilityMenu::createButtons() {
    // Scale background for scrolling
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite1->setPosition(0.f, 0.f);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    // Adjust scroll speed proportionally to window size
    _bgScrollSpeed = _windowSize.x * 0.125f;

    // Title
    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("GRAPHISMS");
    _titleText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.08f));
    _titleText->setFillColor(render::Color::White());
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    // Center text by adjusting position (IText doesn't have setOrigin)
    float titleX = static_cast<float>(_windowSize.x) / 2.f - titleBounds.width / 2.f;
    float titleY = static_cast<float>(_windowSize.y) * 0.15f - titleBounds.height / 2.f;
    _titleText->setPosition(titleX, titleY);

    float labelSize = static_cast<unsigned int>(_windowSize.y * 0.04f);

    // Reference square for contrast testing (centered)
    float squareSize = _windowSize.y * 0.1f;
    float squareY = _windowSize.y * 0.25f;

    _referenceSquare = _window.createRectangleShape(render::Vector2f(squareSize, squareSize));
    _referenceSquare->setFillColor(render::Color(128, 128, 128)); // Gray color for better contrast testing
    _referenceSquare->setPosition((_windowSize.x - squareSize) / 2.f, squareY);

    _referenceLabel = _window.createText();
    _referenceLabel->setFont(*_font);
    _referenceLabel->setString("Reference square");
    _referenceLabel->setCharacterSize(labelSize * 0.8f);
    _referenceLabel->setFillColor(render::Color::White());
    render::FloatRect labelBounds = _referenceLabel->getLocalBounds();
    _referenceLabel->setPosition((_windowSize.x - labelBounds.width) / 2.f, squareY - labelSize);

    // Contrast settings
    _contrastLabel = _window.createText();
    _contrastLabel->setFont(*_font);
    _contrastLabel->setString("Contrast ");
    _contrastLabel->setCharacterSize(labelSize);
    _contrastLabel->setFillColor(render::Color::White());
    _contrastLabel->setPosition(_windowSize.x * 0.15f, _windowSize.y * 0.45f);

    _contrastValue = _window.createText();
    _contrastValue->setFont(*_font);
    _contrastValue->setString(std::to_string(static_cast<int>(_currentContrast * 100)));
    _contrastValue->setCharacterSize(labelSize);
    _contrastValue->setFillColor(render::Color::Yellow());
    _contrastValue->setPosition(_windowSize.x * 0.5f, _windowSize.y * 0.45f);

    // Contrast slider track
    float sliderWidth = _windowSize.x * 0.4f;
    float sliderHeight = _windowSize.y * 0.02f;
    _contrastSliderTrack = _window.createRectangleShape(render::Vector2f(sliderWidth, sliderHeight));
    _contrastSliderTrack->setFillColor(render::Color(100, 100, 100));
    _contrastSliderTrack->setPosition(_windowSize.x * 0.3f, _windowSize.y * 0.55f);

    // Contrast slider handle
    float handleRadius = _windowSize.y * 0.025f;
    _contrastSliderHandle = _window.createCircleShape(handleRadius);
    _contrastSliderHandle->setFillColor(render::Color(200, 200, 200));
    // Note: IShape doesn't have setOrigin, position will be adjusted during updateContrastSliderPosition

    // Position slider handle based on current contrast value
    updateContrastSliderPosition();

    // Back button
    float btnWidth = _windowSize.x * 0.2f;
    float btnHeight = _windowSize.y * 0.08f;
    _backButton = _window.createRectangleShape(render::Vector2f(btnWidth, btnHeight));
    _backButton->setFillColor(render::Color(150, 50, 50));
    _backButton->setPosition((_windowSize.x - btnWidth) / 2.f, _windowSize.y * 0.75f);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(static_cast<unsigned int>(_windowSize.y * 0.05f));
    _backButtonText->setFillColor(render::Color::White());
    render::FloatRect backBounds = _backButtonText->getLocalBounds();
    // Center text in button
    float backTextX = (_windowSize.x - btnWidth) / 2.f + btnWidth / 2.f - backBounds.width / 2.f;
    float backTextY = _windowSize.y * 0.75f + btnHeight / 2.f - backBounds.height / 2.f;
    _backButtonText->setPosition(backTextX, backTextY);
}

void AccessibilityMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    createButtons();
}

void AccessibilityMenu::updateContrastSlider(float mouseX) {
    render::FloatRect trackBounds = _contrastSliderTrack->getGlobalBounds();
    float trackLeft = trackBounds.left;
    float trackWidth = trackBounds.width;

    // Calculate new contrast value based on mouse position
    float relativePos = (mouseX - trackLeft) / trackWidth;
    relativePos = std::max(0.0f, std::min(1.0f, relativePos));

    _currentContrast = _contrastSliderMin + relativePos * (_contrastSliderMax - _contrastSliderMin);

    // Update the settings singleton
    Settings::getInstance().setContrast(_currentContrast);

    // Update the display value
    _contrastValue->setString(std::to_string(static_cast<int>(_currentContrast * 100)));

    // Update slider handle position
    updateContrastSliderPosition();

    std::cout << "Contrast set to: " << (_currentContrast * 100) << "%" << std::endl;
}

void AccessibilityMenu::updateContrastSliderPosition() {
    render::FloatRect trackBounds = _contrastSliderTrack->getGlobalBounds();
    float trackLeft = trackBounds.left;
    float trackWidth = trackBounds.width;
    float trackCenter = trackBounds.top + trackBounds.height / 2.0f;

    // Calculate handle position based on current contrast value
    float relativePos = (_currentContrast - _contrastSliderMin) / (_contrastSliderMax - _contrastSliderMin);
    float handleX = trackLeft + relativePos * trackWidth;

    // For circle shape, we need to offset by radius to center it
    float handleRadius = _windowSize.y * 0.025f;
    _contrastSliderHandle->setPosition(handleX - handleRadius, trackCenter - handleRadius);
}

AccessibilityResult AccessibilityMenu::run() {
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
                return AccessibilityResult::Back;
            }

            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape || event.key.code == render::Key::P) {
                    return AccessibilityResult::Resumed;
                }
            }

            if (event.type == render::EventType::Resized) {
                _windowSize = {event.size.width, event.size.height};
                // Note: IRenderWindow doesn't have setView, so we skip view management
                // The SFML implementation should handle this internally
                updateButtonScale();
            }

            if (event.type == render::EventType::MouseMoved) {
                mousePos.x = static_cast<float>(event.mouseMove.x);
                mousePos.y = static_cast<float>(event.mouseMove.y);

                if (_isDraggingContrastSlider) {
                    updateContrastSlider(mousePos.x);
                }
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                mousePos.x = static_cast<float>(event.mouseButton.x);
                mousePos.y = static_cast<float>(event.mouseButton.y);

                // Check contrast slider
                if (containsPoint(_contrastSliderTrack->getGlobalBounds(), mousePos) ||
                    containsPoint(_contrastSliderHandle->getGlobalBounds(), mousePos)) {
                    _isDraggingContrastSlider = true;
                    updateContrastSlider(mousePos.x);
                }

                // Check back button
                if (containsPoint(_backButton->getGlobalBounds(), mousePos)) {
                    return AccessibilityResult::Back;
                }
            }

            if (event.type == render::EventType::MouseButtonReleased) {
                _isDraggingContrastSlider = false;
            }
        }

        // Background scrolling
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

    return AccessibilityResult::None;
}

void AccessibilityMenu::render() {
    _window.clear();

    // Draw scrolling background
    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    // Draw title
    _window.draw(*_titleText);

    // Draw reference square with contrast applied
    Settings& settings = Settings::getInstance();

    _window.draw(*_referenceLabel);

    // Note: Shader support is not available in the render interface
    // Apply contrast via color modulation as fallback
    auto referenceSquareCopy = _window.createRectangleShape(render::Vector2f(
        _referenceSquare->getGlobalBounds().width,
        _referenceSquare->getGlobalBounds().height
    ));
    referenceSquareCopy->setPosition(
        _referenceSquare->getGlobalBounds().left,
        _referenceSquare->getGlobalBounds().top
    );
    render::Color grayWithContrast = settings.applyContrast(render::Color(128, 128, 128));
    referenceSquareCopy->setFillColor(grayWithContrast);
    _window.draw(*referenceSquareCopy);

    // Draw contrast settings
    _window.draw(*_contrastLabel);
    _window.draw(*_contrastValue);
    _window.draw(*_contrastSliderTrack);
    _window.draw(*_contrastSliderHandle);

    // Draw back button
    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    _window.display();
}