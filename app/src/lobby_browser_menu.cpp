/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** lobby_browser_menu - Menu for creating, browsing, and joining lobbies
*/

#include "../include/lobby_browser_menu.hpp"
#include "../include/settings.hpp"
#include <iostream>

LobbyBrowserMenu::LobbyBrowserMenu(render::IRenderWindow &win,
                                   AudioManager &audioMgr,
                                   network::INetwork &netMgr)
    : _window(win), _audioManager(audioMgr), _networkManager(netMgr),
      _scrollOffset(0), _waitingForJoinAck(false), _joinedSuccessfully(false),
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

    // Request initial lobby list
    requestLobbyList();
}

void LobbyBrowserMenu::createUI() {
    Settings &settings = Settings::getInstance();

    _titleText = _window.createText();
    _titleText->setFont(*_font);
    _titleText->setString("LOBBY BROWSER");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _createLobbyButton = _window.createRectangleShape(render::Vector2f(250, 60));
    _createLobbyButton->setFillColor(render::Color(70, 150, 220));
    _createLobbyButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _createLobbyButton->setOutlineThickness(3);

    _createLobbyButtonText = _window.createText();
    _createLobbyButtonText->setFont(*_font);
    _createLobbyButtonText->setString("CREATE LOBBY");
    _createLobbyButtonText->setCharacterSize(22);
    _createLobbyButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _refreshButton = _window.createRectangleShape(render::Vector2f(250, 60));
    _refreshButton->setFillColor(render::Color(100, 180, 100));
    _refreshButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _refreshButton->setOutlineThickness(3);

    _refreshButtonText = _window.createText();
    _refreshButtonText->setFont(*_font);
    _refreshButtonText->setString("REFRESH");
    _refreshButtonText->setCharacterSize(22);
    _refreshButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _backButton = _window.createRectangleShape(render::Vector2f(250, 60));
    _backButton->setFillColor(render::Color(180, 70, 70));
    _backButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _backButton->setOutlineThickness(3);

    _backButtonText = _window.createText();
    _backButtonText->setFont(*_font);
    _backButtonText->setString("BACK");
    _backButtonText->setCharacterSize(22);
    _backButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void LobbyBrowserMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;

    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, 30);

    // Top buttons row
    float buttonY = 110;
    _createLobbyButton->setPosition(centerX - 380, buttonY);
    render::FloatRect createBounds = _createLobbyButtonText->getLocalBounds();
    _createLobbyButtonText->setPosition(
        centerX - 380 + 125 - createBounds.width / 2, buttonY + 18);

    _refreshButton->setPosition(centerX - 125, buttonY);
    render::FloatRect refreshBounds = _refreshButtonText->getLocalBounds();
    _refreshButtonText->setPosition(centerX - 125 + 125 - refreshBounds.width / 2,
                                    buttonY + 18);

    _backButton->setPosition(centerX + 130, buttonY);
    render::FloatRect backBounds = _backButtonText->getLocalBounds();
    _backButtonText->setPosition(centerX + 130 + 125 - backBounds.width / 2,
                                 buttonY + 18);

    // Update lobby list positions
    float lobbyStartY = 200;
    float lobbySpacing = 70;

    for (size_t i = 0; i < _lobbyTexts.size(); ++i) {
        float yPos = lobbyStartY + i * lobbySpacing - _scrollOffset * lobbySpacing;
        _lobbyTexts[i]->setPosition(centerX - 350, yPos);

        if (i < _lobbyJoinButtons.size()) {
            _lobbyJoinButtons[i]->setPosition(centerX + 200, yPos - 5);
            render::FloatRect joinBounds =
                _lobbyJoinButtonTexts[i]->getLocalBounds();
            _lobbyJoinButtonTexts[i]->setPosition(
                centerX + 200 + 75 - joinBounds.width / 2, yPos + 10);
        }
    }

    float scaleX = static_cast<float>(_windowSize.x) / _bgTexture->getSize().x;
    float scaleY = static_cast<float>(_windowSize.y) / _bgTexture->getSize().y;
    _bgSprite1->setScale(scaleX, scaleY);
    _bgSprite2->setScale(scaleX, scaleY);

    _bgScrollSpeed = _windowSize.x * 0.125f;
}

void LobbyBrowserMenu::requestLobbyList() {
    std::cout << "[LobbyBrowser] Requesting lobby list..." << std::endl;
    bool success = _networkManager.sendTCP(
        network::MessageType::LOBBY_LIST_REQUEST, {});

    if (!success) {
        std::cerr << "[LobbyBrowser] Failed to send LOBBY_LIST_REQUEST!"
                  << std::endl;
    }
}

void LobbyBrowserMenu::createLobby() {
    std::cout << "[LobbyBrowser] Creating new lobby..." << std::endl;

    // For now, create a lobby with default parameters
    // In a full implementation, you'd have a dialog to get name and max players
    std::string lobbyName = "New Lobby";
    uint8_t maxPlayers = 4;

    std::vector<uint8_t> data;
    data.push_back(maxPlayers);

    // Add lobby name length (2 bytes, big endian)
    uint16_t nameLen = lobbyName.length();
    data.push_back(static_cast<uint8_t>((nameLen >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(nameLen & 0xFF));

    // Add lobby name
    for (char c : lobbyName) {
        data.push_back(static_cast<uint8_t>(c));
    }

    bool success =
        _networkManager.sendTCP(network::MessageType::CREATE_LOBBY, data);

    if (!success) {
        std::cerr << "[LobbyBrowser] Failed to send CREATE_LOBBY!" << std::endl;
    } else {
        std::cout << "[LobbyBrowser] CREATE_LOBBY sent, waiting for response..."
                  << std::endl;
        _waitingForJoinAck = true;
    }
}

void LobbyBrowserMenu::joinLobby(uint16_t lobby_id) {
    std::cout << "[LobbyBrowser] Joining lobby " << lobby_id << "..."
              << std::endl;

    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>((lobby_id >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(lobby_id & 0xFF));

    bool success = _networkManager.sendTCP(network::MessageType::JOIN_LOBBY, data);

    if (!success) {
        std::cerr << "[LobbyBrowser] Failed to send JOIN_LOBBY!" << std::endl;
    } else {
        std::cout << "[LobbyBrowser] JOIN_LOBBY sent, waiting for response..."
                  << std::endl;
        _waitingForJoinAck = true;
    }
}

void LobbyBrowserMenu::handleNetworkMessages() {
    auto tcpMessages = _networkManager.pollTCP();
    for (const auto &msg : tcpMessages) {
        if (msg.msg_type == network::MessageType::LOBBY_LIST_RESPONSE) {
            std::cout << "[LobbyBrowser] Received LOBBY_LIST_RESPONSE" << std::endl;

            // Parse lobby count (2 bytes)
            if (msg.data.size() >= 2) {
                uint16_t lobbyCount =
                    (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
                std::cout << "[LobbyBrowser] Found " << lobbyCount << " lobbies"
                          << std::endl;

                _lobbies.clear();
                _lobbyTexts.clear();
                _lobbyJoinButtons.clear();
                _lobbyJoinButtonTexts.clear();

                size_t offset = 2;
                for (uint16_t i = 0; i < lobbyCount; ++i) {
                    if (offset + 8 > msg.data.size())
                        break;

                    LobbyDisplayInfo lobby;
                    lobby.lobby_id = (static_cast<uint16_t>(msg.data[offset]) << 8) |
                                     msg.data[offset + 1];
                    lobby.player_count = msg.data[offset + 2];
                    lobby.max_players = msg.data[offset + 3];

                    uint16_t nameLen =
                        (static_cast<uint16_t>(msg.data[offset + 4]) << 8) |
                        msg.data[offset + 5];

                    offset += 6;

                    if (offset + nameLen > msg.data.size())
                        break;

                    lobby.lobby_name =
                        std::string(msg.data.begin() + offset,
                                    msg.data.begin() + offset + nameLen);
                    offset += 32; // Fixed size for lobby name in protocol

                    if (offset >= msg.data.size())
                        break;

                    lobby.status = msg.data[offset];
                    offset += 4; // status + 3 reserved bytes

                    _lobbies.push_back(lobby);

                    // Create UI elements for this lobby
                    Settings &settings = Settings::getInstance();
                    auto lobbyText = _window.createText();
                    lobbyText->setFont(*_font);
                    std::string displayText = lobby.lobby_name + " [" +
                                              std::to_string(lobby.player_count) +
                                              "/" +
                                              std::to_string(lobby.max_players) + "]";
                    lobbyText->setString(displayText);
                    lobbyText->setCharacterSize(24);
                    lobbyText->setFillColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    _lobbyTexts.push_back(std::move(lobbyText));

                    // Create join button for this lobby
                    auto joinButton =
                        _window.createRectangleShape(render::Vector2f(150, 50));
                    joinButton->setFillColor(render::Color(70, 180, 70));
                    joinButton->setOutlineColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    joinButton->setOutlineThickness(2);
                    _lobbyJoinButtons.push_back(std::move(joinButton));

                    auto joinButtonText = _window.createText();
                    joinButtonText->setFont(*_font);
                    joinButtonText->setString("JOIN");
                    joinButtonText->setCharacterSize(20);
                    joinButtonText->setFillColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    _lobbyJoinButtonTexts.push_back(std::move(joinButtonText));
                }

                updateButtonScale();
            }
        } else if (msg.msg_type == network::MessageType::CREATE_LOBBY_ACK) {
            std::cout << "[LobbyBrowser] Received CREATE_LOBBY_ACK" << std::endl;
            if (msg.data.size() >= 2) {
                uint16_t lobbyId =
                    (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
                std::cout << "[LobbyBrowser] Created lobby with ID: " << lobbyId
                          << std::endl;
                _waitingForJoinAck = false;
                _joinedSuccessfully = true;
            }
        } else if (msg.msg_type == network::MessageType::JOIN_LOBBY_ACK) {
            std::cout << "[LobbyBrowser] Received JOIN_LOBBY_ACK" << std::endl;
            _waitingForJoinAck = false;
            _joinedSuccessfully = true;
        } else if (msg.msg_type == network::MessageType::JOIN_LOBBY_NAK) {
            std::cout << "[LobbyBrowser] Received JOIN_LOBBY_NAK (join failed)"
                      << std::endl;
            _waitingForJoinAck = false;
            _joinedSuccessfully = false;
        } else if (msg.msg_type == network::MessageType::TCP_ERROR) {
            std::cerr << "[LobbyBrowser] Received TCP_ERROR from server!"
                      << std::endl;
            if (msg.data.size() >= 1) {
                std::cerr << "[LobbyBrowser] Error code: "
                          << static_cast<int>(msg.data[0]) << std::endl;
            }
            _waitingForJoinAck = false;
        }
    }

    auto state = _networkManager.getConnectionState();
    if (state == network::ConnectionState::ERROR ||
        state == network::ConnectionState::DISCONNECTED) {
        std::cerr << "[LobbyBrowser] Connection error or disconnected!"
                  << std::endl;
    }
}

void LobbyBrowserMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);

    _window.draw(*_createLobbyButton);
    _window.draw(*_createLobbyButtonText);
    _window.draw(*_refreshButton);
    _window.draw(*_refreshButtonText);
    _window.draw(*_backButton);
    _window.draw(*_backButtonText);

    // Draw lobby list
    float lobbyStartY = 200;
    float lobbySpacing = 70;

    for (size_t i = 0; i < _lobbyTexts.size(); ++i) {
        float yPos = lobbyStartY + i * lobbySpacing - _scrollOffset * lobbySpacing;
        if (yPos >= 180 && yPos <= _windowSize.y - 50) {
            _window.draw(*_lobbyTexts[i]);
            if (i < _lobbyJoinButtons.size()) {
                _window.draw(*_lobbyJoinButtons[i]);
                _window.draw(*_lobbyJoinButtonTexts[i]);
            }
        }
    }

    _window.display();
}

LobbyBrowserResult LobbyBrowserMenu::run() {
    std::cout << "[LobbyBrowser] Entering lobby browser..." << std::endl;

    while (_window.isOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt =
            std::chrono::duration<float>(currentTime - _lastTime).count();
        _lastTime = currentTime;

        handleNetworkMessages();

        // Check if we successfully joined a lobby
        if (_joinedSuccessfully) {
            std::cout << "[LobbyBrowser] Successfully joined lobby, transitioning..."
                      << std::endl;
            return LobbyBrowserResult::JoinedLobby;
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
                return LobbyBrowserResult::Disconnect;
            }

            if (event.type == render::EventType::Resized) {
                updateButtonScale();
            }

            if (event.type == render::EventType::MouseButtonPressed) {
                if (event.mouseButton.button == render::Mouse::Left) {
                    float mouseX = static_cast<float>(event.mouseButton.x);
                    float mouseY = static_cast<float>(event.mouseButton.y);
                    float centerX = _windowSize.x / 2.0f;

                    // Create Lobby button
                    if (mouseX >= centerX - 380 && mouseX <= centerX - 130 &&
                        mouseY >= 110 && mouseY <= 170) {
                        createLobby();
                    }

                    // Refresh button
                    if (mouseX >= centerX - 125 && mouseX <= centerX + 125 &&
                        mouseY >= 110 && mouseY <= 170) {
                        requestLobbyList();
                    }

                    // Back button
                    if (mouseX >= centerX + 130 && mouseX <= centerX + 380 &&
                        mouseY >= 110 && mouseY <= 170) {
                        _networkManager.disconnect();
                        return LobbyBrowserResult::Disconnect;
                    }

                    // Check lobby join buttons
                    for (size_t i = 0; i < _lobbyJoinButtons.size(); ++i) {
                        float yPos = 200 + i * 70 - _scrollOffset * 70;
                        if (yPos >= 180 && yPos <= _windowSize.y - 50) {
                            if (mouseX >= centerX + 200 &&
                                mouseX <= centerX + 350 && mouseY >= yPos - 5 &&
                                mouseY <= yPos + 45) {
                                joinLobby(_lobbies[i].lobby_id);
                            }
                        }
                    }
                }
            }

            if (event.type == render::EventType::KeyPressed) {
                if (event.key.code == render::Key::Escape) {
                    _networkManager.disconnect();
                    return LobbyBrowserResult::Disconnect;
                }

                // Scroll with arrow keys
                if (event.key.code == render::Key::Up && _scrollOffset > 0) {
                    _scrollOffset--;
                    updateButtonScale();
                }
                if (event.key.code == render::Key::Down &&
                    _scrollOffset < static_cast<int>(_lobbies.size()) -
                                        MAX_VISIBLE_LOBBIES) {
                    _scrollOffset++;
                    updateButtonScale();
                }

                // Refresh with R key
                if (event.key.code == render::Key::R) {
                    requestLobbyList();
                }
            }
        }

        render();
    }

    return LobbyBrowserResult::Disconnect;
}
