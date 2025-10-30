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
      _showCreateDialog(false), _newLobbyName(""), _newLobbyMaxPlayers(4),
      _isTypingLobbyName(false), _bgScrollSpeed(100.f) {
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

    _statusText = _window.createText();
    _statusText->setFont(*_font);
    _statusText->setString("");
    _statusText->setCharacterSize(18);
    _statusText->setFillColor(
        settings.applyColorblindFilter(render::Color(255, 200, 100)));

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

    // Create dialog UI elements
    _dialogBackground = _window.createRectangleShape(render::Vector2f(600, 400));
    _dialogBackground->setFillColor(render::Color(30, 30, 50, 240));
    _dialogBackground->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _dialogBackground->setOutlineThickness(3);

    _dialogTitle = _window.createText();
    _dialogTitle->setFont(*_font);
    _dialogTitle->setString("CREATE NEW LOBBY");
    _dialogTitle->setCharacterSize(32);
    _dialogTitle->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _lobbyNameLabel = _window.createText();
    _lobbyNameLabel->setFont(*_font);
    _lobbyNameLabel->setString("Lobby Name:");
    _lobbyNameLabel->setCharacterSize(24);
    _lobbyNameLabel->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _lobbyNameInputBox = _window.createRectangleShape(render::Vector2f(400, 50));
    _lobbyNameInputBox->setFillColor(render::Color(40, 40, 40));
    _lobbyNameInputBox->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _lobbyNameInputBox->setOutlineThickness(2);

    _lobbyNameInputText = _window.createText();
    _lobbyNameInputText->setFont(*_font);
    _lobbyNameInputText->setString("");
    _lobbyNameInputText->setCharacterSize(22);
    _lobbyNameInputText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _maxPlayersLabel = _window.createText();
    _maxPlayersLabel->setFont(*_font);
    _maxPlayersLabel->setString("Max Players:");
    _maxPlayersLabel->setCharacterSize(24);
    _maxPlayersLabel->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _maxPlayersText = _window.createText();
    _maxPlayersText->setFont(*_font);
    _maxPlayersText->setString("4");
    _maxPlayersText->setCharacterSize(28);
    _maxPlayersText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _decreasePlayersButton = _window.createRectangleShape(render::Vector2f(50, 50));
    _decreasePlayersButton->setFillColor(render::Color(180, 70, 70));
    _decreasePlayersButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _decreasePlayersButton->setOutlineThickness(2);

    _decreasePlayersButtonText = _window.createText();
    _decreasePlayersButtonText->setFont(*_font);
    _decreasePlayersButtonText->setString("-");
    _decreasePlayersButtonText->setCharacterSize(32);
    _decreasePlayersButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _increasePlayersButton = _window.createRectangleShape(render::Vector2f(50, 50));
    _increasePlayersButton->setFillColor(render::Color(70, 180, 70));
    _increasePlayersButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _increasePlayersButton->setOutlineThickness(2);

    _increasePlayersButtonText = _window.createText();
    _increasePlayersButtonText->setFont(*_font);
    _increasePlayersButtonText->setString("+");
    _increasePlayersButtonText->setCharacterSize(32);
    _increasePlayersButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _confirmButton = _window.createRectangleShape(render::Vector2f(180, 60));
    _confirmButton->setFillColor(render::Color(70, 180, 70));
    _confirmButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _confirmButton->setOutlineThickness(3);

    _confirmButtonText = _window.createText();
    _confirmButtonText->setFont(*_font);
    _confirmButtonText->setString("CREATE");
    _confirmButtonText->setCharacterSize(24);
    _confirmButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    _cancelButton = _window.createRectangleShape(render::Vector2f(180, 60));
    _cancelButton->setFillColor(render::Color(180, 70, 70));
    _cancelButton->setOutlineColor(
        settings.applyColorblindFilter(render::Color::White()));
    _cancelButton->setOutlineThickness(3);

    _cancelButtonText = _window.createText();
    _cancelButtonText->setFont(*_font);
    _cancelButtonText->setString("CANCEL");
    _cancelButtonText->setCharacterSize(24);
    _cancelButtonText->setFillColor(
        settings.applyColorblindFilter(render::Color::White()));

    updateButtonScale();
}

void LobbyBrowserMenu::updateButtonScale() {
    _windowSize = _window.getSize();
    float centerX = _windowSize.x / 2.0f;
    float centerY = _windowSize.y / 2.0f;

    render::FloatRect titleBounds = _titleText->getLocalBounds();
    _titleText->setPosition(centerX - titleBounds.width / 2, 30);

    render::FloatRect statusBounds = _statusText->getLocalBounds();
    _statusText->setPosition(centerX - statusBounds.width / 2, 85);

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

    // Dialog positioning
    _dialogBackground->setPosition(centerX - 300, centerY - 200);
    
    render::FloatRect dialogTitleBounds = _dialogTitle->getLocalBounds();
    _dialogTitle->setPosition(centerX - dialogTitleBounds.width / 2, centerY - 170);

    _lobbyNameLabel->setPosition(centerX - 200, centerY - 100);
    _lobbyNameInputBox->setPosition(centerX - 200, centerY - 65);
    _lobbyNameInputText->setPosition(centerX - 190, centerY - 55);

    _maxPlayersLabel->setPosition(centerX - 200, centerY + 10);
    
    _decreasePlayersButton->setPosition(centerX - 100, centerY + 50);
    render::FloatRect decBounds = _decreasePlayersButtonText->getLocalBounds();
    _decreasePlayersButtonText->setPosition(
        centerX - 100 + 25 - decBounds.width / 2, centerY + 55);

    render::FloatRect maxPlayersBounds = _maxPlayersText->getLocalBounds();
    _maxPlayersText->setPosition(centerX - maxPlayersBounds.width / 2, centerY + 58);

    _increasePlayersButton->setPosition(centerX + 50, centerY + 50);
    render::FloatRect incBounds = _increasePlayersButtonText->getLocalBounds();
    _increasePlayersButtonText->setPosition(
        centerX + 50 + 25 - incBounds.width / 2, centerY + 55);

    _confirmButton->setPosition(centerX - 200, centerY + 120);
    render::FloatRect confirmBounds = _confirmButtonText->getLocalBounds();
    _confirmButtonText->setPosition(
        centerX - 200 + 90 - confirmBounds.width / 2, centerY + 137);

    _cancelButton->setPosition(centerX + 20, centerY + 120);
    render::FloatRect cancelBounds = _cancelButtonText->getLocalBounds();
    _cancelButtonText->setPosition(
        centerX + 20 + 90 - cancelBounds.width / 2, centerY + 137);

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
        _errorMessage = "Failed to request lobby list";
        _errorMessageTime = std::chrono::steady_clock::now();
    } else {
        _statusText->setString("Refreshing lobby list...");
    }
}

void LobbyBrowserMenu::showCreateLobbyDialog() {
    _showCreateDialog = true;
    _newLobbyName = "";
    _newLobbyMaxPlayers = 4;
    _isTypingLobbyName = false;
    _lobbyNameInputText->setString("");
    _maxPlayersText->setString("4");
    updateButtonScale();
}

void LobbyBrowserMenu::createLobby() {
    if (_newLobbyName.empty()) {
        _errorMessage = "Lobby name cannot be empty!";
        _errorMessageTime = std::chrono::steady_clock::now();
        return;
    }

    if (_newLobbyName.length() > 32) {
        _errorMessage = "Lobby name too long (max 32 chars)!";
        _errorMessageTime = std::chrono::steady_clock::now();
        return;
    }

    std::cout << "[LobbyBrowser] Creating lobby: " << _newLobbyName 
              << " (max " << static_cast<int>(_newLobbyMaxPlayers) << " players)" << std::endl;

    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>(_newLobbyMaxPlayers));

    // Add lobby name length (2 bytes, big endian)
    uint16_t nameLen = _newLobbyName.length();
    data.push_back(static_cast<uint8_t>((nameLen >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(nameLen & 0xFF));

    // Add lobby name
    for (char c : _newLobbyName) {
        data.push_back(static_cast<uint8_t>(c));
    }

    bool success =
        _networkManager.sendTCP(network::MessageType::CREATE_LOBBY, data);

    if (!success) {
        std::cerr << "[LobbyBrowser] Failed to send CREATE_LOBBY!" << std::endl;
        _errorMessage = "Failed to send create lobby request";
        _errorMessageTime = std::chrono::steady_clock::now();
    } else {
        std::cout << "[LobbyBrowser] CREATE_LOBBY sent, waiting for response..."
                  << std::endl;
        _waitingForJoinAck = true;
        _showCreateDialog = false;
        _statusText->setString("Creating lobby...");
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
        _errorMessage = "Failed to send join lobby request";
        _errorMessageTime = std::chrono::steady_clock::now();
    } else {
        std::cout << "[LobbyBrowser] JOIN_LOBBY sent, waiting for response..."
                  << std::endl;
        _waitingForJoinAck = true;
        _statusText->setString("Joining lobby...");
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

                    if (offset + 32 > msg.data.size())
                        break;

                    // Read actual name bytes (up to nameLen)
                    std::string actualName(msg.data.begin() + offset,
                                          msg.data.begin() + offset + 
                                          std::min(static_cast<size_t>(nameLen), size_t(32)));
                    lobby.lobby_name = actualName;
                    
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
                    
                    std::string statusStr = "";
                    if (lobby.status == 0) statusStr = "[WAITING]";
                    else if (lobby.status == 1) statusStr = "[READY]";
                    else if (lobby.status == 2) statusStr = "[IN GAME]";
                    
                    std::string displayText = lobby.lobby_name + " " + statusStr + 
                                              " [" + std::to_string(lobby.player_count) +
                                              "/" + std::to_string(lobby.max_players) + "]";
                    lobbyText->setString(displayText);
                    lobbyText->setCharacterSize(24);
                    
                    // Color based on status
                    render::Color textColor = render::Color::White();
                    if (lobby.status == 2 || lobby.player_count >= lobby.max_players) {
                        textColor = render::Color(150, 150, 150); // Gray for full/in game
                    }
                    
                    lobbyText->setFillColor(settings.applyColorblindFilter(textColor));
                    _lobbyTexts.push_back(std::move(lobbyText));

                    // Create join button for this lobby
                    auto joinButton =
                        _window.createRectangleShape(render::Vector2f(150, 50));
                    
                    // Disable button if lobby is full or in game
                    bool canJoin = (lobby.status != 2 && lobby.player_count < lobby.max_players);
                    joinButton->setFillColor(canJoin ? render::Color(70, 180, 70) : 
                                                       render::Color(100, 100, 100));
                    joinButton->setOutlineColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    joinButton->setOutlineThickness(2);
                    _lobbyJoinButtons.push_back(std::move(joinButton));

                    auto joinButtonText = _window.createText();
                    joinButtonText->setFont(*_font);
                    joinButtonText->setString(canJoin ? "JOIN" : "FULL");
                    joinButtonText->setCharacterSize(20);
                    joinButtonText->setFillColor(
                        settings.applyColorblindFilter(render::Color::White()));
                    _lobbyJoinButtonTexts.push_back(std::move(joinButtonText));
                }

                _statusText->setString(lobbyCount > 0 ? "" : "No lobbies available. Create one!");
                updateButtonScale();
            }
        } else if (msg.msg_type == network::MessageType::CREATE_LOBBY_ACK) {
            std::cout << "[LobbyBrowser] Received CREATE_LOBBY_ACK" << std::endl;
            if (msg.data.size() >= 2) {
                uint16_t lobbyId =
                    (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
                std::cout << "[LobbyBrowser] Created lobby with ID: " << lobbyId
                          << ", automatically joined!" << std::endl;
                // When we create a lobby, we are automatically in it
                _waitingForJoinAck = false;
                _joinedSuccessfully = true;
                _statusText->setString("Lobby created! You are now in the lobby.");
            }
        } else if (msg.msg_type == network::MessageType::JOIN_LOBBY_ACK) {
            std::cout << "[LobbyBrowser] Received JOIN_LOBBY_ACK" << std::endl;
            _waitingForJoinAck = false;
            _joinedSuccessfully = true;
            _statusText->setString("Successfully joined lobby!");
        } else if (msg.msg_type == network::MessageType::TCP_ERROR) {
            std::cerr << "[LobbyBrowser] Received TCP_ERROR from server!"
                      << std::endl;
            if (msg.data.size() >= 1) {
                uint8_t errorCode = msg.data[0];
                std::cerr << "[LobbyBrowser] Error code: "
                          << static_cast<int>(errorCode) << std::endl;
                
                // Handle specific error codes
                switch (errorCode) {
                    case 0x01:
                        _errorMessage = "Lobby is full!";
                        break;
                    case 0x02:
                        _errorMessage = "Lobby not found!";
                        break;
                    case 0x03:
                        _errorMessage = "Game already started!";
                        break;
                    case 0x04:
                        _errorMessage = "Invalid username!";
                        break;
                    case 0x0A:
                        _errorMessage = "Already in a lobby!";
                        break;
                    default:
                        _errorMessage = "Server error occurred!";
                        break;
                }
                _errorMessageTime = std::chrono::steady_clock::now();
                _statusText->setString(_errorMessage);
            }
            _waitingForJoinAck = false;
        }
    }

    // Check connection state
    auto state = _networkManager.getConnectionState();
    if (state == network::ConnectionState::ERROR ||
        state == network::ConnectionState::DISCONNECTED) {
        std::cerr << "[LobbyBrowser] Connection error or disconnected!"
                  << std::endl;
        _errorMessage = "Connection lost!";
        _errorMessageTime = std::chrono::steady_clock::now();
    }
}

void LobbyBrowserMenu::updateCreateLobbyDialog() {
    _maxPlayersText->setString(std::to_string(_newLobbyMaxPlayers));
    _lobbyNameInputText->setString(_newLobbyName);
    
    // Update outline based on typing state
    if (_isTypingLobbyName) {
        _lobbyNameInputBox->setOutlineThickness(3);
    } else {
        _lobbyNameInputBox->setOutlineThickness(2);
    }
}

void LobbyBrowserMenu::renderCreateLobbyDialog() {
    _window.draw(*_dialogBackground);
    _window.draw(*_dialogTitle);
    _window.draw(*_lobbyNameLabel);
    _window.draw(*_lobbyNameInputBox);
    _window.draw(*_lobbyNameInputText);
    _window.draw(*_maxPlayersLabel);
    _window.draw(*_decreasePlayersButton);
    _window.draw(*_decreasePlayersButtonText);
    _window.draw(*_maxPlayersText);
    _window.draw(*_increasePlayersButton);
    _window.draw(*_increasePlayersButtonText);
    _window.draw(*_confirmButton);
    _window.draw(*_confirmButtonText);
    _window.draw(*_cancelButton);
    _window.draw(*_cancelButtonText);
}

void LobbyBrowserMenu::render() {
    _window.clear();

    _window.draw(*_bgSprite1);
    _window.draw(*_bgSprite2);

    _window.draw(*_titleText);
    
    // Show status or error message
    auto now = std::chrono::steady_clock::now();
    if (!_errorMessage.empty() &&
        std::chrono::duration_cast<std::chrono::seconds>(now - _errorMessageTime).count() < 5) {
        _statusText->setString(_errorMessage);
        _statusText->setFillColor(render::Color(255, 100, 100));
    } else if (_errorMessage.empty()) {
        _statusText->setFillColor(render::Color(255, 200, 100));
    }
    
    _window.draw(*_statusText);

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

    // Draw create lobby dialog if shown
    if (_showCreateDialog) {
        renderCreateLobbyDialog();
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
                    float centerY = _windowSize.y / 2.0f;

                    if (_showCreateDialog) {
                        // Handle dialog interactions
                        
                        // Check if lobby name input box is clicked
                        if (mouseX >= centerX - 200 && mouseX <= centerX + 200 &&
                            mouseY >= centerY - 65 && mouseY <= centerY - 15) {
                            _isTypingLobbyName = true;
                            updateCreateLobbyDialog();
                        } else {
                            _isTypingLobbyName = false;
                            updateCreateLobbyDialog();
                        }

                        // Decrease players button
                        if (mouseX >= centerX - 100 && mouseX <= centerX - 50 &&
                            mouseY >= centerY + 50 && mouseY <= centerY + 100) {
                            if (_newLobbyMaxPlayers > 2) {
                                _newLobbyMaxPlayers--;
                                updateCreateLobbyDialog();
                            }
                        }

                        // Increase players button
                        if (mouseX >= centerX + 50 && mouseX <= centerX + 100 &&
                            mouseY >= centerY + 50 && mouseY <= centerY + 100) {
                            if (_newLobbyMaxPlayers < 4) {
                                _newLobbyMaxPlayers++;
                                updateCreateLobbyDialog();
                            }
                        }

                        // Confirm button
                        if (mouseX >= centerX - 200 && mouseX <= centerX - 20 &&
                            mouseY >= centerY + 120 && mouseY <= centerY + 180) {
                            createLobby();
                        }

                        // Cancel button
                        if (mouseX >= centerX + 20 && mouseX <= centerX + 200 &&
                            mouseY >= centerY + 120 && mouseY <= centerY + 180) {
                            _showCreateDialog = false;
                        }
                    } else {
                        // Main menu interactions
                        
                        // Create Lobby button
                        if (mouseX >= centerX - 380 && mouseX <= centerX - 130 &&
                            mouseY >= 110 && mouseY <= 170) {
                            showCreateLobbyDialog();
                        }

                        // Refresh button
                        if (mouseX >= centerX - 125 && mouseX <= centerX + 125 &&
                            mouseY >= 110 && mouseY <= 170) {
                            requestLobbyList();
                        }

                        // Back button - disconnect and return to menu
                        if (mouseX >= centerX + 130 && mouseX <= centerX + 380 &&
                            mouseY >= 110 && mouseY <= 170) {
                            std::cout << "[LobbyBrowser] Disconnecting from server..." << std::endl;
                            _networkManager.disconnect();
                            return LobbyBrowserResult::BackToMenu;
                        }

                        // Check lobby join buttons
                        for (size_t i = 0; i < _lobbyJoinButtons.size(); ++i) {
                            float yPos = 200 + i * 70 - _scrollOffset * 70;
                            if (yPos >= 180 && yPos <= _windowSize.y - 50) {
                                if (mouseX >= centerX + 200 &&
                                    mouseX <= centerX + 350 && mouseY >= yPos - 5 &&
                                    mouseY <= yPos + 45) {
                                    // Check if lobby is joinable
                                    if (_lobbies[i].status != 2 && 
                                        _lobbies[i].player_count < _lobbies[i].max_players) {
                                        joinLobby(_lobbies[i].lobby_id);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (event.type == render::EventType::KeyPressed) {
                if (_showCreateDialog) {
                    // Handle dialog keyboard input
                    if (_isTypingLobbyName) {
                        if (event.key.code == render::Key::Backspace && !_newLobbyName.empty()) {
                            _newLobbyName.pop_back();
                            updateCreateLobbyDialog();
                        } else if (event.key.code == render::Key::Enter) {
                            createLobby();
                        } else if (event.key.code == render::Key::Escape) {
                            _isTypingLobbyName = false;
                            updateCreateLobbyDialog();
                        } else if (event.key.code >= render::Key::A && event.key.code <= render::Key::Z) {
                            if (_newLobbyName.length() < 32) {
                                char c = 'A' + (static_cast<int>(event.key.code) - static_cast<int>(render::Key::A));
                                _newLobbyName += c;
                                updateCreateLobbyDialog();
                            }
                        } else if (event.key.code >= render::Key::Num0 && event.key.code <= render::Key::Num9) {
                            if (_newLobbyName.length() < 32) {
                                char c = '0' + (static_cast<int>(event.key.code) - static_cast<int>(render::Key::Num0));
                                _newLobbyName += c;
                                updateCreateLobbyDialog();
                            }
                        } else if (event.key.code == render::Key::Space && _newLobbyName.length() < 32) {
                            _newLobbyName += ' ';
                            updateCreateLobbyDialog();
                        }
                    } else {
                        // Dialog shortcuts
                        if (event.key.code == render::Key::Escape) {
                            _showCreateDialog = false;
                        } else if (event.key.code == render::Key::Enter) {
                            createLobby();
                        } else if (event.key.code == render::Key::Left && _newLobbyMaxPlayers > 2) {
                            _newLobbyMaxPlayers--;
                            updateCreateLobbyDialog();
                        } else if (event.key.code == render::Key::Right && _newLobbyMaxPlayers < 4) {
                            _newLobbyMaxPlayers++;
                            updateCreateLobbyDialog();
                        }
                    }
                } else {
                    // Main menu keyboard shortcuts
                    if (event.key.code == render::Key::Escape) {
                        _networkManager.disconnect();
                        return LobbyBrowserResult::BackToMenu;
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

                    // Create lobby with C key
                    if (event.key.code == render::Key::C) {
                        showCreateLobbyDialog();
                    }
                }
            }

        }

        render();
    }

    return LobbyBrowserResult::Disconnect;
}