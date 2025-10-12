#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <chrono>
#include <memory>
#include <string>

enum class ConnectionMenuResult {
    Solo,        // Play solo (offline)
    Multiplayer, // Play multiplayer (online)
    Back         // Go back to main menu
};

struct ConnectionInfo {
    std::string serverHost;
    uint16_t serverPort;
};

class ConnectionMenu {
  public:
    ConnectionMenu(render::IRenderWindow &win, AudioManager &audioMgr);

    /**
     * @brief Run the connection menu
     * @param outConnectionInfo Output parameter for server connection info
     * @return ConnectionMenuResult indicating user choice
     */
    ConnectionMenuResult run(ConnectionInfo &outConnectionInfo);

  private:
    void createUI();
    void updateButtonScale();
    void render();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;

    // UI elements
    std::unique_ptr<render::IText> _titleText;
    std::unique_ptr<render::IText> _instructionText;

    // Buttons
    std::unique_ptr<render::IShape> _soloButton;
    std::unique_ptr<render::IText> _soloButtonText;
    std::unique_ptr<render::IShape> _multiplayerButton;
    std::unique_ptr<render::IText> _multiplayerButtonText;
    std::unique_ptr<render::IShape> _backButton;
    std::unique_ptr<render::IText> _backButtonText;

    // Server settings (hardcoded)
    std::string _serverHost;
    std::string _serverPort;

    // Background scrolling
    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;
    float _bgScrollSpeed;
    std::chrono::steady_clock::time_point _lastTime;
};
