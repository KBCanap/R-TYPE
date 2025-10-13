#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include "boss_manager.hpp"
#include "components.hpp"
#include "enemy_manager.hpp"
#include "game_menu.hpp"
#include "key_bindings.hpp"
#include "player_manager.hpp"
#include "registery.hpp"
#include "tick_system.hpp"
#include "network/NetworkCommandHandler.hpp"
#include <memory>
#include <optional>
#include <vector>

// Forward declaration
namespace network {
class NetworkManager;
}

class Game {
  public:
    Game(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
         KeyBindings &keyBindings, network::NetworkManager *netMgr = nullptr);

    void run(); // Boucle principale du jeu avec système de tick

  private:
    void handleEvents(bool &running, float dt);
    void update(float dt);
    void updateMultiplayer(float dt); // New: multiplayer-specific update
    void render(float dt);
    void resetGame();
    bool isPlayerAlive() const;

    // Network helpers
    void sendPlayerInput(float dt);
    void checkGameEndConditions();
    void renderPlayerInfo(entity player_entity);
    void renderScore(uint32_t score);

  private:
    registry &_registry;
    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    KeyBindings &_keyBindings;
    network::NetworkManager *_networkManager; // nullptr for solo mode
    bool _isMultiplayer;

    // Managers for different game aspects
    PlayerManager _playerManager;
    EnemyManager _enemyManager;
    BossManager _bossManager;

    std::optional<entity> _player;     // Player créé via spawn_entity()
    std::optional<entity> _background; // Background entity
    std::vector<entity> _enemies;      // Ennemis
    std::optional<entity> _boss;       // Boss entity

    float _playerSpeed = 300.f;
    float _enemySpawnTimer = 0.f;
    float _enemySpawnInterval = 2.f;
    float _gameTime = 0.f;

    // Relative positions (0.0 to 1.0)
    float _playerRelativeX = 0.125f; // 100/800 = 0.125
    float _playerRelativeY = 0.5f;   // 300/600 = 0.5

    GameOverMenu _gameOverMenu;
    VictoryMenu _victoryMenu;
    bool _gameOver = false;
    bool _victory = false;

    TickSystem _tickSystem;
    bool _shouldExit = false;

    std::unique_ptr<render::IFont>
        _scoreFont; // Font for displaying score using r-type.otf

    std::unique_ptr<NetworkCommandHandler> _networkCommandHandler;
    float _inputSendTimer = 0.0f;
    float _inputSendInterval = 0.016f; // Send input ~60 times per second
};
