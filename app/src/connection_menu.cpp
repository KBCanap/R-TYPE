/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** connection_menu
*/

#include "../include/connection_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

ConnectionMenu::ConnectionMenu(render::IRenderWindow &win,
                               AudioManager &audioMgr)
    : _window(win), _audioManager(audioMgr), _username(""),
      _isTypingUsername(false), _serverHost("127.0.0.1"), _serverPort("8080"),
      _bgScrollSpeed(100.f) {
    _baseWindowSize = _window.getSize();
    _windowSize = _baseWindowSize;

    _bgTexture = _window.createTexture();
    if (!_bgTexture->loadFromFile("assets/background.jpg")) {
        std::cerr << "Failed to load background.jpg" << std::endl;
    }

    _bgSprite1 = _window.createSprite();
    _bgSprite2 = _window.createSprite();
    _bgSprite1->setTexture(*_bgTexture);
    _bgSprite2->setTexture(*_bgTexture);

    auto texSize = _bgTexture->getSize();
    float scaleX = static_cast<float>(_windowSize.x) / texSize.x;
    float scaleY = static_cast<float>(_windowSize.y) / texSize.y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);
    _bgSprite2->setPosition(static_cast<float>(_windowSize.x), 0.f);

    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf")) {
        std::cerr << "Failed to load font r-type.otf" << std::endl;
    }

    createUI();
    _lastTime = std::chrono::steady_clock::now();
}

void ConnectionMenu::createUI() {
    Settings &settings = Settings::getInstance();

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("SELECT GAME MODE");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _instructionText = _window.createText();
    _instructionText->setFont(*_font);
    _instructionText->setString("Choose your preferred mode");
    _instructionText->setCharacterSize(20);
    _instructionText->setFillColor(
        settings.applyColorblindFilter(render::Color(150, 150, 150)));

    _soloButton = _window.createRectangleShape(render::Vector2f(400, 70));
    _soloButton->setFillColor(render::Color(70, 70, 180));
    _soloButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _soloButton->setOutlineThickness(3);

    _soloButtonText = _window.createText();
    _soloButtonText->setFont(*_font);
    _soloButtonText->setString("SOLO");
    _soloButtonText->setCharacterSize(28);
    _soloButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _multiplayerButton =
        _window.createRectangleShape(render::Vector2f(400, 70));
    _multiplayerButton->setFillColor(render::Color(70, 180, 70));
    _multiplayerButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _multiplayerButton->setOutlineThickness(3);

    _multiplayerButtonText = _window.createText();
    _multiplayerButtonText->setFont(*_font);
    _multiplayerButtonText->setString("MULTIPLAYER");
    _multiplayerButtonText->setCharacterSize(28);
    _multiplayerButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _backButton = _window.createRectangleShape(render::Vector2f(400, 70));
    _backButton->setFillColor(render::Color(180, 70, 70));
    _backButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _backButton->setOutlineThickness(3);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("BACK");
    _backButtonText->setCharacterSize(28);
    _backButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    // Username input field
    _usernameLabel = _window.createText();
    _usernameLabel->setFont(*_font);
    _usernameLabel->setString("Username:");
    _usernameLabel->setCharacterSize(24);
    _usernameLabel->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _usernameInputBox = _window.createRectangleShape(render::Vector2f(400, 50));
    _usernameInputBox->setFillColor(render::Color(40, 40, 40));
    _usernameInputBox->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _usernameInputBox->setOutlineThickness(2);

    _usernameInputText = _window.createText();
    _usernameInputText->setFont(*_font);
    _usernameInputText->setString("");
    _usernameInputText->setCharacterSize(22);
    _usernameInputText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void ConnectionMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;
    float centerY = _windowSize.y / 2.0f;

    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, centerY - 250);

    render::FloatRect instrBounds = _instructionText->getLocalBounds();
    _instructionText->setPosition(centerX - instrBounds.width / 2,
                                  centerY - 180);

    // Username label and input box
    _usernameLabel->setPosition(centerX - 200, centerY - 140);
    _usernameInputBox->setPosition(centerX - 200, centerY - 105);
    _usernameInputText->setPosition(centerX - 190, centerY - 95);

    // Buttons
    _soloButton->setPosition(centerX - 200, centerY - 30);
    render::FloatRect soloBtnBounds = _soloButtonText->getLocalBounds();
    _soloButtonText->setPosition(centerX - soloBtnBounds.width / 2,
                                 centerY - 10);

    _multiplayerButton->setPosition(centerX - 200, centerY + 70);
    render::FloatRect multiplayerBtnBounds =
        _multiplayerButtonText->getLocalBounds();
    _multiplayerButtonText->setPosition(
        centerX - multiplayerBtnBounds.width / 2, centerY + 90);

    _backButton->setPosition(centerX - 200, centerY + 170);
    render::FloatRect backBtnBounds = _backButtonText->getLocalBounds();
    _backButtonText->setPosition(centerX - backBtnBounds.width / 2,
                                 centerY + 190);

    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    _bgScrollSpeed = _windowSize.x * 0.125f;
}

void ConnectionMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);
    _window.draw(*_instructionText);

    _window.draw(*_usernameLabel);
    _window.draw(*_usernameInputBox);
    _window.draw(*_usernameInputText);

    _window.draw(*_soloButton);
    _window.draw(*_soloButtonText);
    _window.draw(*_multiplayerButton);
    _window.draw(*_multiplayerButtonText);
    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    _window.display();
}

ConnectionMenuResult ConnectionMenu::run(ConnectionInfo &outConnectionInfo) {
    std::cout << "[ConnectionMenu] Using server: "
              << outConnectionInfo.serverHost << ":"
              << outConnectionInfo.serverPort << std::endl;

    int selectedButton = 0; // 0=solo, 1=multiplayer, 2=back

    while (_window.isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt =
            std::chrono::duration<float>(currentTime - _lastTime).count();
        _lastTime = currentTime;

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

            if (event.type == render::EventType::MouseButtonPressed) {
                if (event.mouseButton.button == render::Mouse::Left) {
                    float mouseX = static_cast<float>(event.mouseButton.x);
                    float mouseY = static_cast<float>(event.mouseButton.y);

                    float centerX = _windowSize.x / 2.0f;
                    float centerY = _windowSize.y / 2.0f;

                    // Check if username input box is clicked
                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY - 105 && mouseY <= centerY - 55) {
                        _isTypingUsername = true;
                        _usernameInputBox->setOutlineThickness(3);
                    } else {
                        _isTypingUsername = false;
                        _usernameInputBox->setOutlineThickness(2);
                    }

                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY - 30 && mouseY <= centerY + 40) {
                        return ConnectionMenuResult::Solo;
                    }

                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY + 70 && mouseY <= centerY + 140) {

                        // Set username in ConnectionInfo
                        if (!_username.empty()) {
                            outConnectionInfo.username = _username;
                        } else {
                            outConnectionInfo.username = "Player";
                        }

                        std::cout << "[ConnectionMenu] Multiplayer selected "
                                     "with username: "
                                  << outConnectionInfo.username << std::endl;

                        return ConnectionMenuResult::Multiplayer;
                    }

                    if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                        mouseY >= centerY + 170 && mouseY <= centerY + 240) {
                        return ConnectionMenuResult::Back;
                    }
                }
            }

            if (event.type == render::EventType::MouseMoved) {
                float mouseX = static_cast<float>(event.mouseMove.x);
                float mouseY = static_cast<float>(event.mouseMove.y);

                float centerX = _windowSize.x / 2.0f;
                float centerY = _windowSize.y / 2.0f;

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

            if (event.type == render::EventType::KeyPressed) {
                // Handle username input
                if (_isTypingUsername) {
                    if (event.key.code == render::Key::Backspace &&
                        !_username.empty()) {
                        _username.pop_back();
                        _usernameInputText->setString(_username);
                    } else if (event.key.code == render::Key::Enter) {
                        _isTypingUsername = false;
                        _usernameInputBox->setOutlineThickness(2);
                    } else if (event.key.code >= render::Key::A &&
                               event.key.code <= render::Key::Z) {
                        if (_username.length() < 20) {
                            char c = 'A' + (static_cast<int>(event.key.code) -
                                            static_cast<int>(render::Key::A));
                            _username += c;
                            _usernameInputText->setString(_username);
                        }
                    } else if (event.key.code >= render::Key::Num0 &&
                               event.key.code <= render::Key::Num9) {
                        if (_username.length() < 20) {
                            char c =
                                '0' + (static_cast<int>(event.key.code) -
                                       static_cast<int>(render::Key::Num0));
                            _username += c;
                            _usernameInputText->setString(_username);
                        }
                    } else if (event.key.code == render::Key::Space &&
                               _username.length() < 20) {
                        _username += ' ';
                        _usernameInputText->setString(_username);
                    }
                    continue;
                }

                if (event.key.code == render::Key::Escape) {
                    return ConnectionMenuResult::Back;
                }
                if (event.key.code == render::Key::Up) {
                    selectedButton = (selectedButton - 1 + 3) % 3;
                }
                if (event.key.code == render::Key::Down) {
                    selectedButton = (selectedButton + 1) % 3;
                }

                if (event.key.code == render::Key::Num1) {
                    return ConnectionMenuResult::Solo;
                }
                if (event.key.code == render::Key::Num2) {
                    // Set username in ConnectionInfo
                    if (!_username.empty()) {
                        outConnectionInfo.username = _username;
                    } else {
                        outConnectionInfo.username = "Player";
                    }
                    std::cout << "[ConnectionMenu] Multiplayer selected (key) "
                                 "with username: "
                              << outConnectionInfo.username << std::endl;
                    return ConnectionMenuResult::Multiplayer;
                }
                if (event.key.code == render::Key::Num3) {
                    return ConnectionMenuResult::Back;
                }

                if (event.key.code == render::Key::Enter) {
                    if (selectedButton == 0) {
                        return ConnectionMenuResult::Solo;
                    } else if (selectedButton == 1) {
                        // Set username in ConnectionInfo
                        if (!_username.empty()) {
                            outConnectionInfo.username = _username;
                        } else {
                            outConnectionInfo.username = "Player";
                        }
                        std::cout << "[ConnectionMenu] Multiplayer selected "
                                     "(enter) with username: "
                                  << outConnectionInfo.username << std::endl;
                        return ConnectionMenuResult::Multiplayer;
                    } else {
                        return ConnectionMenuResult::Back;
                    }
                }
            }
        }

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
