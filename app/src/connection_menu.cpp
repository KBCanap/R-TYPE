#include "../include/connection_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

ConnectionMenu::ConnectionMenu(render::IRenderWindow& win, AudioManager& audioMgr)
    : _window(win), _audioManager(audioMgr),
      _serverHost("127.0.0.1"),
      _serverPort("8080"),
      _bgScrollSpeed(100.f)
{
    _baseWindowSize = _window.getSize();
    _windowSize = _baseWindowSize;

    // Load background
    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg")) {
        std::cerr << "Failed to load background.jpg" << std::endl;
    }

    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    _bgSprite1->setTexture(*_bgTexture);
    _bgSprite2->setTexture(*_bgTexture);

    // Scale background
    auto texSize = _bgTexture->getSize();
    float scaleX = static_cast<float>(_windowSize.x) / texSize.x;
    float scaleY = static_cast<float>(_windowSize.y) / texSize.y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    // Load font
    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf")) {
        std::cerr << "Failed to load font r-type.otf" << std::endl;
    }

    createUI();
    _lastTime = std::chrono::steady_clock::now();
}

void ConnectionMenu::createUI() {
    Settings& settings = Settings::getInstance();

    // Title
    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("SELECT GAME MODE");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    // Instructions
    _instructionText = _window.createText();
    _instructionText->setFont(*_font);
    _instructionText->setString("Choose your preferred mode");
    _instructionText->setCharacterSize(20);
    _instructionText->setFillColor(settings.applyColorblindFilter(render::Color(150, 150, 150)));

    // Solo button
    _soloButton = _window.createRectangleShape(render::Vector2f(400, 70));
    _soloButton->setFillColor(render::Color(70, 70, 180));
    _soloButton->setOutlineColor(settings.applyColorblindFilter(render::Color::White()));
    _soloButton->setOutlineThickness(3);

    _soloButtonText = _window.createText();
    _soloButtonText->setFont(*_font);
    _soloButtonText->setString("SOLO");
    _soloButtonText->setCharacterSize(28);
    _soloButtonText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    // Multiplayer button
    _multiplayerButton = _window.createRectangleShape(render::Vector2f(400, 70));
    _multiplayerButton->setFillColor(render::Color(70, 180, 70));
    _multiplayerButton->setOutlineColor(settings.applyColorblindFilter(render::Color::White()));
    _multiplayerButton->setOutlineThickness(3);

    _multiplayerButtonText = _window.createText();
    _multiplayerButtonText->setFont(*_font);
    _multiplayerButtonText->setString("MULTIPLAYER");
    _multiplayerButtonText->setCharacterSize(28);
    _multiplayerButtonText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    // Back button
    _backButton = _window.createRectangleShape(render::Vector2f(400, 70));
    _backButton->setFillColor(render::Color(180, 70, 70));
    _backButton->setOutlineColor(settings.applyColorblindFilter(render::Color::White()));
    _backButton->setOutlineThickness(3);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("BACK");
    _backButtonText->setCharacterSize(28);
    _backButtonText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void ConnectionMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;
    float centerY = _windowSize.y / 2.0f;

    // Title
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, centerY - 250);

    // Instructions
    render::FloatRect instrBounds = _instructionText->getLocalBounds();
    _instructionText->setPosition(centerX - instrBounds.width / 2, centerY - 180);

    // Solo button
    _soloButton->setPosition(centerX - 200, centerY - 80);
    render::FloatRect soloBtnBounds = _soloButtonText->getLocalBounds();
    _soloButtonText->setPosition(
        centerX - soloBtnBounds.width / 2,
        centerY - 60
    );

    // Multiplayer button
    _multiplayerButton->setPosition(centerX - 200, centerY + 20);
    render::FloatRect multiplayerBtnBounds = _multiplayerButtonText->getLocalBounds();
    _multiplayerButtonText->setPosition(
        centerX - multiplayerBtnBounds.width / 2,
        centerY + 40
    );

    // Back button
    _backButton->setPosition(centerX - 200, centerY + 120);
    render::FloatRect backBtnBounds = _backButtonText->getLocalBounds();
    _backButtonText->setPosition(
        centerX - backBtnBounds.width / 2,
        centerY + 140
    );

    // Update background scale
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    // Adjust scroll speed proportionally
    _bgScrollSpeed = _windowSize.x * 0.125f;
}

void ConnectionMenu::render() {
    _window.clear();

    // Draw scrolling background
    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);
    _window.draw(*_instructionText);

    _window.draw(*_soloButton);
    _window.draw(*_soloButtonText);
    _window.draw(*_multiplayerButton);
    _window.draw(*_multiplayerButtonText);
    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    _window.display();
}

ConnectionMenuResult ConnectionMenu::run(ConnectionInfo& outConnectionInfo) {
    int selectedButton = 0; // 0=solo, 1=multiplayer, 2=back

    while (_window.isOpen()) {
        // Calculate delta time
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - _lastTime).count();
        _lastTime = currentTime;

        // Background scroll
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

        render::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == render::EventType::Closed) {
                _window.close();
                return ConnectionMenuResult::Back;
            }

            if (event.type == render::EventType::Resized) {
                updateButtonScale();
            }

            // Mouse click handling
            if (event.type == render::EventType::MouseButtonPressed) {
                if (event.mouseButton.button == render::Mouse::Left) {
                    float mouseX = static_cast<float>(event.mouseButton.x);
                    float mouseY = static_cast<float>(event.mouseButton.y);

                    float centerX = _windowSize.x / 2.0f;
                    float centerY = _windowSize.y / 2.0f;

                    // Check solo button (centerY - 80, size 400x70)
                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY - 80 && mouseY <= centerY - 10) {
                        return ConnectionMenuResult::Solo;
                    }

                    // Check multiplayer button (centerY + 20, size 400x70)
                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY + 20 && mouseY <= centerY + 90) {
                        outConnectionInfo.serverHost = _serverHost;
                        try {
                            outConnectionInfo.serverPort = static_cast<uint16_t>(std::stoi(_serverPort));
                        } catch (...) {
                            outConnectionInfo.serverPort = 8080;
                        }
                        return ConnectionMenuResult::Multiplayer;
                    }

                    // Check back button (centerY + 120, size 400x70)
                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY + 120 && mouseY <= centerY + 190) {
                        return ConnectionMenuResult::Back;
                    }
                }
            }

            // Mouse hover handling for visual feedback
            if (event.type == render::EventType::MouseMoved) {
                float mouseX = static_cast<float>(event.mouseMove.x);
                float mouseY = static_cast<float>(event.mouseMove.y);

                float centerX = _windowSize.x / 2.0f;
                float centerY = _windowSize.y / 2.0f;

                // Check which button is hovered
                if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                    mouseY >= centerY - 80 && mouseY <= centerY - 10) {
                    selectedButton = 0;
                } else if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                           mouseY >= centerY + 20 && mouseY <= centerY + 90) {
                    selectedButton = 1;
                } else if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                           mouseY >= centerY + 120 && mouseY <= centerY + 190) {
                    selectedButton = 2;
                }
            }

            // Keyboard navigation
            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape) {
                    return ConnectionMenuResult::Back;
                }
                if (event.key.code == render::Key::Up) {
                    selectedButton = (selectedButton - 1 + 3) % 3;
                }
                if (event.key.code == render::Key::Down) {
                    selectedButton = (selectedButton + 1) % 3;
                }

                // Direct selection with numbers
                if (event.key.code == render::Key::Num1) {
                    return ConnectionMenuResult::Solo;
                }
                if (event.key.code == render::Key::Num2) {
                    outConnectionInfo.serverHost = _serverHost;
                    try {
                        outConnectionInfo.serverPort = static_cast<uint16_t>(std::stoi(_serverPort));
                    } catch (...) {
                        outConnectionInfo.serverPort = 8080;
                    }
                    return ConnectionMenuResult::Multiplayer;
                }
                if (event.key.code == render::Key::Num3) {
                    return ConnectionMenuResult::Back;
                }

                // Enter to confirm selection
                if (event.key.code == render::Key::Enter) {
                    if (selectedButton == 0) {
                        return ConnectionMenuResult::Solo;
                    } else if (selectedButton == 1) {
                        outConnectionInfo.serverHost = _serverHost;
                        try {
                            outConnectionInfo.serverPort = static_cast<uint16_t>(std::stoi(_serverPort));
                        } catch (...) {
                            outConnectionInfo.serverPort = 8080;
                        }
                        return ConnectionMenuResult::Multiplayer;
                    } else {
                        return ConnectionMenuResult::Back;
                    }
                }
            }
        }

        // Update button highlights
        if (selectedButton == 0) {
            _soloButton->setOutlineThickness(5);
            _multiplayerButton->setOutlineThickness(3);
            _backButton->setOutlineThickness(3);
        } else if (selectedButton == 1) {
            _soloButton->setOutlineThickness(3);
            _multiplayerButton->setOutlineThickness(5);
            _backButton->setOutlineThickness(3);
        } else {
            _soloButton->setOutlineThickness(3);
            _multiplayerButton->setOutlineThickness(3);
            _backButton->setOutlineThickness(5);
        }

        render();
    }

    return ConnectionMenuResult::Back;
}
