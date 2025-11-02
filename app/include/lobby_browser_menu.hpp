/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** lobby_browser_menu - Menu for creating, browsing, and joining lobbies
*/

#pragma once
#include "../../ecs/include/network/INetwork.hpp"
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

enum class LobbyBrowserResult {
    JoinedLobby, // Successfully joined a lobby, proceed to LobbyMenu
    Disconnect,  // User wants to disconnect
    BackToMenu,  // User wants to go back without disconnecting
    Error        // An error occurred
};

struct LobbyDisplayInfo {
    uint16_t lobby_id;
    std::string lobby_name;
    uint8_t player_count;
    uint8_t max_players;
    uint8_t status; // 0=WAITING, 1=READY, 2=IN_GAME
};

class LobbyBrowserMenu {
  public:
    LobbyBrowserMenu(render::IRenderWindow &win, AudioManager &audioMgr,
                     network::INetwork &netMgr);

    LobbyBrowserResult run();

  private:
    void createUI();
    void updateButtonScale();
    void render();
    void handleNetworkMessages();
    void requestLobbyList();
    void createLobby();
    void joinLobby(uint16_t lobby_id);

    // Dialog for creating lobby
    void showCreateLobbyDialog();
    void updateCreateLobbyDialog();
    void renderCreateLobbyDialog();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    network::INetwork &_networkManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;

    // UI elements
    std::unique_ptr<render::IText> _titleText;
    std::unique_ptr<render::IText> _statusText;

    // Buttons
    std::unique_ptr<render::IShape> _createLobbyButton;
    std::unique_ptr<render::IText> _createLobbyButtonText;
    std::unique_ptr<render::IShape> _refreshButton;
    std::unique_ptr<render::IText> _refreshButtonText;
    std::unique_ptr<render::IShape> _backButton;
    std::unique_ptr<render::IText> _backButtonText;

    // Lobby list display
    std::vector<LobbyDisplayInfo> _lobbies;
    std::vector<std::unique_ptr<render::IText>> _lobbyTexts;
    std::vector<std::unique_ptr<render::IShape>> _lobbyJoinButtons;
    std::vector<std::unique_ptr<render::IText>> _lobbyJoinButtonTexts;

    int _scrollOffset;
    static constexpr int MAX_VISIBLE_LOBBIES = 5;

    bool _waitingForJoinAck;
    bool _joinedSuccessfully;
    bool _showCreateDialog;

    // Create lobby dialog
    std::string _newLobbyName;
    int _newLobbyMaxPlayers;
    bool _isTypingLobbyName;

    // Dialog UI elements
    std::unique_ptr<render::IShape> _dialogBackground;
    std::unique_ptr<render::IText> _dialogTitle;
    std::unique_ptr<render::IText> _lobbyNameLabel;
    std::unique_ptr<render::IShape> _lobbyNameInputBox;
    std::unique_ptr<render::IText> _lobbyNameInputText;
    std::unique_ptr<render::IText> _maxPlayersLabel;
    std::unique_ptr<render::IText> _maxPlayersText;
    std::unique_ptr<render::IShape> _decreasePlayersButton;
    std::unique_ptr<render::IText> _decreasePlayersButtonText;
    std::unique_ptr<render::IShape> _increasePlayersButton;
    std::unique_ptr<render::IText> _increasePlayersButtonText;
    std::unique_ptr<render::IShape> _confirmButton;
    std::unique_ptr<render::IText> _confirmButtonText;
    std::unique_ptr<render::IShape> _cancelButton;
    std::unique_ptr<render::IText> _cancelButtonText;

    std::string _errorMessage;
    std::chrono::steady_clock::time_point _errorMessageTime;

    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;
    float _bgScrollSpeed;
    std::chrono::steady_clock::time_point _lastTime;
};