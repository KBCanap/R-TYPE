#include "../include/lobby_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

LobbyMenu::LobbyMenu(render::IRenderWindow& win, AudioManager& audioMgr, network::INetwork& netMgr)
    : _window(win), _audioManager(audioMgr), _networkManager(netMgr),
      _isReady(false), _waitingForGameStart(false), _bgScrollSpeed(100.f)
{
    _baseWindowSize = _window.getSize();
    _windowSize = _baseWindowSize;
    _myPlayerId = _networkManager.getPlayerID();

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

void LobbyMenu::createUI() {
    Settings& settings = Settings::getInstance();

    // Title
    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("WAITING FOR PLAYERS");
    _titleText->setCharacterSize(40);
    _titleText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    // Player ID text
    _playerIdText = _window.createText();
    _playerIdText->setFont(*_font);
    _playerIdText->setString("YOU ARE PLAYER " + std::to_string(_myPlayerId));
    _playerIdText->setCharacterSize(30);
    _playerIdText->setFillColor(settings.applyColorblindFilter(render::Color(150, 200, 255)));

    // Ready button
    _readyButton = _window.createRectangleShape(render::Vector2f(350, 70));
    _readyButton->setFillColor(render::Color(70, 180, 70));
    _readyButton->setOutlineColor(settings.applyColorblindFilter(render::Color::White()));
    _readyButton->setOutlineThickness(3);

    _readyButtonText = _window.createText();
    _readyButtonText->setFont(*_font);
    _readyButtonText->setString("READY");
    _readyButtonText->setCharacterSize(28);
    _readyButtonText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    // Disconnect button
    _disconnectButton = _window.createRectangleShape(render::Vector2f(350, 70));
    _disconnectButton->setFillColor(render::Color(180, 70, 70));
    _disconnectButton->setOutlineColor(settings.applyColorblindFilter(render::Color::White()));
    _disconnectButton->setOutlineThickness(3);

    _disconnectButtonText = _window.createText();
    _disconnectButtonText->setFont(*_font);
    _disconnectButtonText->setString("DISCONNECT");
    _disconnectButtonText->setCharacterSize(28);
    _disconnectButtonText->setFillColor(settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void LobbyMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;
    float centerY = _windowSize.y / 2.0f;

    // Title
    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, centerY - 200);

    // Player ID
    render::FloatRect playerIdBounds = _playerIdText->getLocalBounds();
    _playerIdText->setPosition(centerX - playerIdBounds.width / 2, centerY - 120);

    // Ready button
    _readyButton->setPosition(centerX - 175, centerY + 20);
    render::FloatRect readyBtnBounds = _readyButtonText->getLocalBounds();
    _readyButtonText->setPosition(
        centerX - readyBtnBounds.width / 2,
        centerY + 40
    );

    // Disconnect button
    _disconnectButton->setPosition(centerX - 175, centerY + 110);
    render::FloatRect disconnectBtnBounds = _disconnectButtonText->getLocalBounds();
    _disconnectButtonText->setPosition(
        centerX - disconnectBtnBounds.width / 2,
        centerY + 130
    );

    // Update background scale
    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    // Adjust scroll speed proportionally
    _bgScrollSpeed = _windowSize.x * 0.125f;
}

void LobbyMenu::handleNetworkMessages() {
    // Poll TCP messages
    auto tcpMessages = _networkManager.pollTCP();
    for (const auto& msg : tcpMessages) {
        if (msg.msg_type == network::MessageType::TCP_GAME_START) {
            std::cout << "[Lobby] Received TCP_GAME_START! Transitioning to game..." << std::endl;
            _waitingForGameStart = true;
        } else if (msg.msg_type == network::MessageType::TCP_ERROR) {
            std::cerr << "[Lobby] Received TCP_ERROR from server!" << std::endl;
        }
    }

    // Check connection state
    auto state = _networkManager.getConnectionState();
    if (state == network::ConnectionState::ERROR ||
        state == network::ConnectionState::DISCONNECTED) {
        std::cerr << "[Lobby] Connection error or disconnected!" << std::endl;
    }
}

void LobbyMenu::sendReadyMessage() {
    // Send TCP_READY according to RFC:
    // MSG_TYPE = 0x04, DATA_LENGTH = 0 (no payload)
    // Format: 04 00 00 00
    std::cout << "[Lobby] Sending TCP_READY (0x04 0x00 0x00 0x00)..." << std::endl;

    bool success = _networkManager.sendTCP(network::MessageType::TCP_READY, {});

    if (success) {
        _isReady = true;
        _readyButtonText->setString("WAITING...");
        _readyButton->setFillColor(render::Color(100, 100, 100));
        std::cout << "[Lobby] TCP_READY sent successfully!" << std::endl;
    } else {
        std::cerr << "[Lobby] Failed to send TCP_READY!" << std::endl;
    }
}

void LobbyMenu::render() {
    _window.clear();

    // Draw scrolling background
    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);
    _window.draw(*_playerIdText);

    _window.draw(*_readyButton);
    _window.draw(*_readyButtonText);
    _window.draw(*_disconnectButton);
    _window.draw(*_disconnectButtonText);

    _window.display();
}

LobbyResult LobbyMenu::run() {
    std::cout << "[Lobby] Entering lobby as Player " << static_cast<int>(_myPlayerId) << std::endl;

    while (_window.isOpen()) {
        // Calculate delta time
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - _lastTime).count();
        _lastTime = currentTime;

        // Handle network messages
        handleNetworkMessages();

        // Check if game is starting
        if (_waitingForGameStart && _networkManager.getConnectionState() == network::ConnectionState::GAME_STARTING) {
            std::cout << "[Lobby] Game starting! Exiting lobby..." << std::endl;
            return LobbyResult::StartGame;
        }

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

        // Handle events
        render::Event event;
        while (_window.pollEvent(event)) {
            if (event.type == render::EventType::Closed) {
                _window.close();
                return LobbyResult::Disconnect;
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

                    // Check ready button (centerY + 20, size 350x70)
                    if (!_isReady && mouseX >= centerX - 175 && mouseX <= centerX + 175 &&
                        mouseY >= centerY + 20 && mouseY <= centerY + 90) {
                        sendReadyMessage();
                    }

                    // Check disconnect button (centerY + 110, size 350x70)
                    if (mouseX >= centerX - 175 && mouseX <= centerX + 175 &&
                        mouseY >= centerY + 110 && mouseY <= centerY + 180) {
                        _networkManager.disconnect();
                        return LobbyResult::Disconnect;
                    }
                }
            }

            // Keyboard handling
            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape) {
                    _networkManager.disconnect();
                    return LobbyResult::Disconnect;
                }

                // Space to ready up
                if (event.key.code == render::Key::Space && !_isReady) {
                    sendReadyMessage();
                }
            }
        }

        render();
    }

    return LobbyResult::Disconnect;
}
