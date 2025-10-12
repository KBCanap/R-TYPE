#pragma once
#include "../../ecs/include/network/INetwork.hpp"
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <chrono>
#include <memory>

enum class LobbyResult {
    StartGame,  // All players ready, game starting
    Disconnect, // Player wants to disconnect
    Error       // Connection error
};

class LobbyMenu {
  public:
    LobbyMenu(render::IRenderWindow &win, AudioManager &audioMgr,
              network::INetwork &netMgr);

    /**
     * @brief Run the lobby menu
     * @return LobbyResult indicating what happened
     */
    LobbyResult run();

  private:
    void createUI();
    void updateButtonScale();
    void render();
    void handleNetworkMessages();
    void sendReadyMessage();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    network::INetwork &_networkManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;

    // UI elements
    std::unique_ptr<render::IText> _titleText;
    std::unique_ptr<render::IText> _playerIdText;

    // Buttons
    std::unique_ptr<render::IShape> _readyButton;
    std::unique_ptr<render::IText> _readyButtonText;
    std::unique_ptr<render::IShape> _disconnectButton;
    std::unique_ptr<render::IText> _disconnectButtonText;

    // State
    bool _isReady;
    bool _waitingForGameStart;
    uint8_t _myPlayerId;

    // Background scrolling
    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;
    float _bgScrollSpeed;
    std::chrono::steady_clock::time_point _lastTime;
};
