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
#include <memory>
#include <optional>
#include <vector>

namespace network {
class NetworkManager;
}

class Game {
  public:
    Game(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
         KeyBindings &keyBindings, network::NetworkManager *netMgr = nullptr);

    void run();

  private:
    void handleEvents(bool &running, float dt);
    void update(float dt);
    void updateMultiplayer(float dt);
    void render(float dt);
    void resetGame();
    bool isPlayerAlive() const;

    void sendPlayerInput(float dt);
    void processNetworkEntities();

  private:
    registry &_registry;
    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    KeyBindings &_keyBindings;
    network::NetworkManager *_networkManager;
    bool _isMultiplayer;

    PlayerManager _playerManager;
    EnemyManager _enemyManager;
    BossManager _bossManager;

    std::optional<entity> _player;
    std::optional<entity> _background;
    std::vector<entity> _enemies;
    std::optional<entity> _boss;

    float _playerSpeed = 300.f;
    float _enemySpawnTimer = 0.f;
    float _enemySpawnInterval = 2.f;
    float _gameTime = 0.f;

    float _playerRelativeX = 0.125f;
    float _playerRelativeY = 0.5f;

    GameOverMenu _gameOverMenu;
    VictoryMenu _victoryMenu;
    bool _gameOver = false;
    bool _victory = false;

    TickSystem _tickSystem;
    bool _shouldExit = false;

    std::unique_ptr<render::IFont> _scoreFont;

    std::unordered_map<uint32_t, std::optional<entity>> _networkEntities;
    float _inputSendTimer = 0.0f;
    float _inputSendInterval = 0.016f;
};
