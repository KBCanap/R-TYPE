/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** lobby_menu - Waiting room menu when in a lobby
*/

#pragma once
#include "../../ecs/include/network/INetwork.hpp"
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

enum class LobbyResult {
    GameStarting,   // Game is starting, proceed to game
    LeftLobby,      // User left the lobby, return to browser
    Disconnect,     // Connection error/window closed
    Error           // An error occurred
};

struct PlayerDisplayInfo {
    uint8_t player_id;
    std::string username;
    bool ready;
};

class LobbyMenu {
  public:
    LobbyMenu(render::IRenderWindow &win, AudioManager &audioMgr,
              network::INetwork &netMgr);

    LobbyResult run();

  private:
    void createUI();
    void updateButtonScale();
    void render();
    void handleNetworkMessages();
    void sendReady();
    void leaveLobby();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    network::INetwork &_networkManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;

    // UI elements
    std::unique_ptr<render::IText> _titleText;
    std::unique_ptr<render::IText> _lobbyNameText;
    std::unique_ptr<render::IText> _statusText;

    // Buttons
    std::unique_ptr<render::IShape> _readyButton;
    std::unique_ptr<render::IText> _readyButtonText;
    std::unique_ptr<render::IShape> _leaveButton;
    std::unique_ptr<render::IText> _leaveButtonText;

    // Player list display
    std::vector<PlayerDisplayInfo> _players;
    std::vector<std::unique_ptr<render::IText>> _playerTexts;
    std::vector<std::unique_ptr<render::IShape>> _playerBoxes;

    bool _isReady;
    bool _waitingForLeaveAck;
    bool _leftSuccessfully;
    bool _gameStarting;
    
    uint16_t _lobbyId;
    uint8_t _myPlayerId;
    std::string _lobbyName;

    std::string _errorMessage;
    std::chrono::steady_clock::time_point _errorMessageTime;

    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;
    float _bgScrollSpeed;
    std::chrono::steady_clock::time_point _lastTime;
};