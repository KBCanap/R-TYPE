/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** lobby_menu - Waiting room menu when in a lobby
*/

#include "../include/lobby_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

LobbyMenu::LobbyMenu(render::IRenderWindow &win, AudioManager &audioMgr,
                     network::INetwork &netMgr)
    : _window(win), _audioManager(audioMgr), _networkManager(netMgr),
      _isReady(false), _waitingForLeaveAck(false), _leftSuccessfully(false),
      _gameStarting(false), _lobbyId(0), _myPlayerId(0), _lobbyName("Lobby"),
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

void LobbyMenu::createUI() {
    Settings &settings = Settings::getInstance();

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("LOBBY");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _lobbyNameText = _window.createText();
    _lobbyNameText->setFont(*_font);
    _lobbyNameText->setString("");
    _lobbyNameText->setCharacterSize(28);
    _lobbyNameText->setFillColor(
        settings.applyColorblindFilter(render::Color(200, 200, 100)));

    _statusText = _window.createText();
    _statusText->setFont(*_font);
    _statusText->setString("Waiting for players...");
    _statusText->setCharacterSize(20);
    _statusText->setFillColor(
        settings.applyColorblindFilter(render::Color(150, 200, 255)));

    _readyButton = _window.createRectangleShape(render::Vector2f(300, 70));
    _readyButton->setFillColor(render::Color(70, 180, 70));
    _readyButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _readyButton->setOutlineThickness(3);

    _readyButtonText = _window.createText();
    _readyButtonText->setFont(*_font);
    _readyButtonText->setString("READY");
    _readyButtonText->setCharacterSize(28);
    _readyButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _leaveButton = _window.createRectangleShape(render::Vector2f(300, 70));
    _leaveButton->setFillColor(render::Color(180, 70, 70));
    _leaveButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _leaveButton->setOutlineThickness(3);

    _leaveButtonText = _window.createText();
    _leaveButtonText->setFont(*_font);
    _leaveButtonText->setString("LEAVE LOBBY");
    _leaveButtonText->setCharacterSize(28);
    _leaveButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void LobbyMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;

    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, 30);

    render::FloatRect lobbyNameBounds = _lobbyNameText->getLocalBounds();
    _lobbyNameText->setPosition(centerX - lobbyNameBounds.width / 2, 90);

    render::FloatRect statusBounds = _statusText->getLocalBounds();
    _statusText->setPosition(centerX - statusBounds.width / 2, 130);

    // Player list positioning
    float playerStartY = 200;
    float playerSpacing = 80;

    for (size_t i = 0; i < _playerBoxes.size(); ++i) {
        float yPos = playerStartY + i * playerSpacing;
        _playerBoxes[i]->setPosition(centerX - 300, yPos);
        _playerTexts[i]->setPosition(centerX - 280, yPos + 15);
    }

    // Buttons at bottom
    _readyButton->setPosition(centerX - 310, _windowSize.y - 150);
    render::FloatRect readyBounds = _readyButtonText->getLocalBounds();
    _readyButtonText->setPosition(centerX - 310 + 150 - readyBounds.width / 2,
                                  _windowSize.y - 127);

    _leaveButton->setPosition(centerX + 10, _windowSize.y - 150);
    render::FloatRect leaveBounds = _leaveButtonText->getLocalBounds();
    _leaveButtonText->setPosition(centerX + 10 + 150 - leaveBounds.width / 2,
                                  _windowSize.y - 127);

    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    _bgScrollSpeed = _windowSize.x * 0.125f;
}

void LobbyMenu::sendReady() {
    std::cout << "[LobbyMenu] Sending READY signal..." << std::endl;

    bool success = _networkManager.sendTCP(network::MessageType::TCP_READY, {});

    if (!success) {
        std::cerr << "[LobbyMenu] Failed to send TCP_READY!" << std::endl;
        _errorMessage = "Failed to send ready signal";
        _errorMessageTime = std::chrono::steady_clock::now();
    } else {
        _isReady = !_isReady; // Toggle ready state
        _readyButtonText->setString(_isReady ? "UNREADY" : "READY");
        _readyButton->setFillColor(_isReady ? render::Color(220, 150, 70)
                                            : render::Color(70, 180, 70));
        _statusText->setString(_isReady ? "You are ready!"
                                        : "Waiting for players...");
        std::cout << "[LobbyMenu] Ready state: "
                  << (_isReady ? "READY" : "NOT READY") << std::endl;
    }
}

void LobbyMenu::leaveLobby() {
    std::cout << "[LobbyMenu] Leaving lobby..." << std::endl;

    bool success =
        _networkManager.sendTCP(network::MessageType::LEAVE_LOBBY, {});

    if (!success) {
        std::cerr << "[LobbyMenu] Failed to send LEAVE_LOBBY!" << std::endl;
        _errorMessage = "Failed to leave lobby";
        _errorMessageTime = std::chrono::steady_clock::now();
    } else {
        std::cout << "[LobbyMenu] LEAVE_LOBBY sent, waiting for ACK..."
                  << std::endl;
        _waitingForLeaveAck = true;
        _statusText->setString("Leaving lobby...");
    }
}

void LobbyMenu::handleNetworkMessages() {
    auto tcpMessages = _networkManager.pollTCP();
    for (const auto &msg : tcpMessages) {
        if (msg.msg_type == network::MessageType::JOIN_LOBBY_ACK) {
            std::cout
                << "[LobbyMenu] Received JOIN_LOBBY_ACK (initial lobby info)"
                << std::endl;

            if (msg.data.size() >= 5) {
                // Parse: LOBBY_ID (2) + YOUR_PLAYER_ID (1) + PLAYER_COUNT (2)
                _lobbyId =
                    (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
                _myPlayerId = msg.data[2];
                uint16_t playerCount =
                    (static_cast<uint16_t>(msg.data[3]) << 8) | msg.data[4];

                std::cout << "[LobbyMenu] Lobby ID: " << _lobbyId
                          << ", My Player ID: " << static_cast<int>(_myPlayerId)
                          << ", Players: " << playerCount << std::endl;

                _players.clear();
                _playerTexts.clear();
                _playerBoxes.clear();

                size_t offset = 5;
                Settings &settings = Settings::getInstance();

                // Parse PlayerInfo structures (each 66 bytes)
                for (uint16_t i = 0; i < playerCount; ++i) {
                    if (offset + 66 > msg.data.size())
                        break;

                    PlayerDisplayInfo player;
                    player.player_id = msg.data[offset];
                    player.ready = msg.data[offset + 1];

                    uint16_t usernameLen =
                        (static_cast<uint16_t>(msg.data[offset + 2]) << 8) |
                        msg.data[offset + 3];

                    offset += 4;

                    if (offset + 60 > msg.data.size())
                        break;

                    player.username = std::string(
                        msg.data.begin() + offset,
                        msg.data.begin() + offset +
                            std::min(static_cast<size_t>(usernameLen),
                                     size_t(60)));
                    offset += 60;

                    _players.push_back(player);

                    // Create UI for this player
                    auto playerBox =
                        _window.createRectangleShape(render::Vector2f(600, 60));
                    playerBox->setFillColor(render::Color(40, 40, 60, 200));
                    playerBox->setOutlineColor(
                        player.ready ? settings.applyColorblindFilter(
                                           render::Color(100, 255, 100))
                                     : settings.applyColorblindFilter(
                                           render::Color::White()));
                    playerBox->setOutlineThickness(player.ready ? 3 : 2);
                    _playerBoxes.push_back(std::move(playerBox));

                    auto playerText = _window.createText();
                    playerText->setFont(*_font);
                    std::string displayText = player.username;
                    if (player.player_id == _myPlayerId) {
                        displayText += " (You)";
                    }
                    if (player.ready) {
                        displayText += " - READY";
                    }
                    playerText->setString(displayText);
                    playerText->setCharacterSize(24);
                    playerText->setFillColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    _playerTexts.push_back(std::move(playerText));
                }

                updateButtonScale();
            }
        } else if (msg.msg_type == network::MessageType::PLAYER_JOINED) {
            std::cout << "[LobbyMenu] Received PLAYER_JOINED" << std::endl;

            if (msg.data.size() >= 66) {
                PlayerDisplayInfo player;
                player.player_id = msg.data[0];
                player.ready = msg.data[1];

                uint16_t usernameLen =
                    (static_cast<uint16_t>(msg.data[2]) << 8) | msg.data[3];

                player.username = std::string(
                    msg.data.begin() + 4,
                    msg.data.begin() + 4 +
                        std::min(static_cast<size_t>(usernameLen), size_t(60)));

                _players.push_back(player);
                std::cout << "[LobbyMenu] Player joined: " << player.username
                          << std::endl;

                Settings &settings = Settings::getInstance();
                auto playerBox =
                    _window.createRectangleShape(render::Vector2f(600, 60));
                playerBox->setFillColor(render::Color(40, 40, 60, 200));
                playerBox->setOutlineColor(
                    settings.applyColorblindFilter(render::Color::White()));
                playerBox->setOutlineThickness(2);
                _playerBoxes.push_back(std::move(playerBox));

                auto playerText = _window.createText();
                playerText->setFont(*_font);
                playerText->setString(player.username);
                playerText->setCharacterSize(24);
                playerText->setFillColor(
                    settings.applyColorblindFilter(render::Color::White()));
                _playerTexts.push_back(std::move(playerText));

                updateButtonScale();
            }
        } else if (msg.msg_type == network::MessageType::PLAYER_LEFT) {
            std::cout << "[LobbyMenu] Received PLAYER_LEFT" << std::endl;

            if (msg.data.size() >= 1) {
                uint8_t leftPlayerId = msg.data[0];

                for (size_t i = 0; i < _players.size(); ++i) {
                    if (_players[i].player_id == leftPlayerId) {
                        std::cout << "[LobbyMenu] Player "
                                  << _players[i].username << " left the lobby"
                                  << std::endl;
                        _players.erase(_players.begin() + i);
                        _playerTexts.erase(_playerTexts.begin() + i);
                        _playerBoxes.erase(_playerBoxes.begin() + i);
                        updateButtonScale();
                        break;
                    }
                }
            }
        } else if (msg.msg_type == network::MessageType::LEAVE_LOBBY_ACK) {
            std::cout
                << "[LobbyMenu] Received LEAVE_LOBBY_ACK, returning to browser"
                << std::endl;
            _waitingForLeaveAck = false;
            _leftSuccessfully = true;
        } else if (msg.msg_type == network::MessageType::TCP_GAME_START) {
            std::cout
                << "[LobbyMenu] Received TCP_GAME_START! Game is starting!"
                << std::endl;
            _gameStarting = true;
            _statusText->setString("Starting game...");

            // Parse game server info if needed
            if (msg.data.size() >= 8) {
                uint16_t udpPort =
                    (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
                uint16_t serverId =
                    (static_cast<uint16_t>(msg.data[2]) << 8) | msg.data[3];
                std::cout << "[LobbyMenu] Game server - Port: " << udpPort
                          << ", Server ID: " << serverId << std::endl;
            }
        } else if (msg.msg_type == network::MessageType::TCP_ERROR) {
            std::cerr << "[LobbyMenu] Received TCP_ERROR from server!"
                      << std::endl;
            if (msg.data.size() >= 1) {
                uint8_t errorCode = msg.data[0];
                std::cerr << "[LobbyMenu] Error code: "
                          << static_cast<int>(errorCode) << std::endl;

                switch (errorCode) {
                case 0x09:
                    _errorMessage = "You are not in a lobby!";
                    _leftSuccessfully = true; // Force return to browser
                    break;
                default:
                    _errorMessage = "Server error occurred!";
                    break;
                }
                _errorMessageTime = std::chrono::steady_clock::now();
                _statusText->setString(_errorMessage);
            }
        }
    }

    // Check connection state
    auto state = _networkManager.getConnectionState();
    if (state == network::ConnectionState::ERROR ||
        state == network::ConnectionState::DISCONNECTED) {
        std::cerr << "[LobbyMenu] Connection error or disconnected!"
                  << std::endl;
        _errorMessage = "Connection lost!";
        _errorMessageTime = std::chrono::steady_clock::now();
    }
}

void LobbyMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);
    _window.draw(*_lobbyNameText);

    // Show status or error message
    auto now = std::chrono::steady_clock::now();
    if (!_errorMessage.empty() &&
        std::chrono::duration_cast<std::chrono::seconds>(now -
                                                         _errorMessageTime)
                .count() < 5) {
        _statusText->setString(_errorMessage);
        _statusText->setFillColor(render::Color(255, 100, 100));
    } else if (_errorMessage.empty()) {
        _statusText->setFillColor(render::Color(150, 200, 255));
    }

    _window.draw(*_statusText);

    // Draw player list
    for (size_t i = 0; i < _playerBoxes.size(); ++i) {
        _window.draw(*_playerBoxes[i]);
        _window.draw(*_playerTexts[i]);
    }

    _window.draw(*_readyButton);
    _window.draw(*_readyButtonText);
    _window.draw(*_leaveButton);
    _window.draw(*_leaveButtonText);

    _window.display();
}

LobbyResult LobbyMenu::run() {
    std::cout << "[LobbyMenu] Entering lobby waiting room..." << std::endl;

    while (_window.isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt =
            std::chrono::duration<float>(currentTime - _lastTime).count();
        _lastTime = currentTime;

        handleNetworkMessages();

        // Check if we left the lobby
        if (_leftSuccessfully) {
            std::cout << "[LobbyMenu] Successfully left lobby, returning to "
                         "browser..."
                      << std::endl;
            return LobbyResult::LeftLobby;
        }

        // Check if game is starting
        if (_gameStarting) {
            std::cout << "[LobbyMenu] Game starting, transitioning to game..."
                      << std::endl;
            return LobbyResult::GameStarting;
        }

        // Scrolling background
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
                return LobbyResult::Disconnect;
            }

            if (event.type == render::EventType::Resized) {
                updateButtonScale();
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                if (event.mouseButton.button == render::Mouse::Left) {
                    float mouseX = static_cast<float>(event.mouseButton.x);
                    float mouseY = static_cast<float>(event.mouseButton.y);
                    float centerX = _windowSize.x / 2.0f;

                    // Ready button
                    if (mouseX >= centerX - 310 && mouseX <= centerX - 10 &&
                        mouseY >= _windowSize.y - 150 &&
                        mouseY <= _windowSize.y - 80) {
                        sendReady();
                    }

                    // Leave button
                    if (mouseX >= centerX + 10 && mouseX <= centerX + 310 &&
                        mouseY >= _windowSize.y - 150 &&
                        mouseY <= _windowSize.y - 80) {
                        leaveLobby();
                    }
                }
            }

            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape) {
                    leaveLobby();
                }

                // Ready with R or Space
                if (event.key.code == render::Key::R ||
                    event.key.code == render::Key::Space) {
                    sendReady();
                }
            }
        }

        render();
    }

    return LobbyResult::Disconnect;
}
