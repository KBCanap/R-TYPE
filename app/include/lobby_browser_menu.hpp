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
    JoinedLobby,  // Successfully joined a lobby, proceed to LobbyMenu
    Disconnect,   // User wants to disconnect
    Error         // An error occurred
};

struct LobbyDisplayInfo {
    uint16_t lobby_id;
    std::string lobby_name;
    uint8_t player_count;
    uint8_t max_players;
    uint8_t status;  // 0=WAITING, 1=READY, 2=IN_GAME
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

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    network::INetwork &_networkManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;

    // UI elements
    std::unique_ptr<render::IText> _titleText;

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

    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;
    float _bgScrollSpeed;
    std::chrono::steady_clock::time_point _lastTime;
};
