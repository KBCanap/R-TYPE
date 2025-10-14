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

namespace network {
class NetworkManager;
}

/**
 * @class Game
 * @brief Main game class that handles the game loop, rendering, and logic
 * 
 * This class manages the entire game state including entities, systems,
 * and both single-player and multiplayer modes.
 */
class Game {
  public:
    /**
     * @brief Construct a new Game object
     * @param reg The entity-component registry
     * @param win The render window interface
     * @param audioMgr The audio manager for sounds and music
     * @param keyBindings The key bindings configuration
     * @param netMgr Optional network manager for multiplayer mode (nullptr for solo)
     */
    Game(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
         KeyBindings &keyBindings, network::NetworkManager *netMgr = nullptr);

    /**
     * @brief Run the main game loop
     * 
     * Initializes the game music and starts the tick-based game loop
     * with separate update and render phases.
     */
    void run();

  private:
    /**
     * @brief Handle window and input events
     * @param running Reference to running flag to control game loop
     * @param dt Delta time since last frame
     */
    void handleEvents(bool &running, float dt);
    
    /**
     * @brief Update game logic for single-player mode
     * @param dt Delta time since last frame
     */
    void update(float dt);
    
    /**
     * @brief Update game logic for multiplayer mode
     * @param dt Delta time since last frame
     */
    void updateMultiplayer(float dt);
    
    /**
     * @brief Render the game frame
     * @param dt Delta time since last frame
     */
    void render(float dt);
    
    /**
     * @brief Reset the game to initial state
     * 
     * Clears all entities and recreates the initial game state.
     * Used when restarting the game.
     */
    void resetGame();
    
    /**
     * @brief Check if the player entity is still alive
     * @return true if player is alive, false otherwise
     */
    bool isPlayerAlive() const;

    /**
     * @brief Send player input to the server in multiplayer mode
     * @param dt Delta time since last frame
     */
    void sendPlayerInput(float dt);
    
    /**
     * @brief Check game end conditions in multiplayer mode
     * 
     * Handles player death detection and game over state transitions.
     */
    void checkGameEndConditions();
    
    /**
     * @brief Render player health information
     * @param player_entity The player entity to render info for
     */
    void renderPlayerInfo(entity player_entity);
    
    /**
     * @brief Render the current score
     * @param score The score value to display
     */
    void renderScore(uint32_t score);

  private:
    registry &_registry;                          ///< Entity-component registry
    render::IRenderWindow &_window;               ///< Render window interface
    AudioManager &_audioManager;                 ///< Audio system manager
    KeyBindings &_keyBindings;                   ///< Key bindings configuration
    network::NetworkManager *_networkManager;    ///< Network manager (nullptr for solo)
    bool _isMultiplayer;                         ///< Whether game is in multiplayer mode

    PlayerManager _playerManager;                ///< Player entity management
    EnemyManager _enemyManager;                  ///< Enemy entity management
    BossManager _bossManager;                    ///< Boss entity management

    std::optional<entity> _player;               ///< Player entity
    std::optional<entity> _background;           ///< Background entity
    std::vector<entity> _enemies;                ///< List of enemy entities
    std::optional<entity> _boss;                 ///< Boss entity

    float _playerSpeed = 300.f;                  ///< Player movement speed
    float _enemySpawnTimer = 0.f;                ///< Timer for enemy spawning
    float _enemySpawnInterval = 2.f;             ///< Interval between enemy spawns
    float _gameTime = 0.f;                       ///< Total game time elapsed

    float _playerRelativeX = 0.125f;             ///< Player relative X position (0.0-1.0)
    float _playerRelativeY = 0.5f;               ///< Player relative Y position (0.0-1.0)

    GameOverMenu _gameOverMenu;                  ///< Game over menu handler
    VictoryMenu _victoryMenu;                    ///< Victory menu handler
    bool _gameOver = false;                      ///< Game over state flag
    bool _victory = false;                       ///< Victory state flag

    TickSystem _tickSystem;                      ///< Fixed timestep game loop system
    bool _shouldExit = false;                    ///< Flag to exit the game

    std::unique_ptr<render::IFont> _scoreFont;   ///< Font for score display

    std::unique_ptr<NetworkCommandHandler> _networkCommandHandler;  ///< Network command processor
    float _inputSendTimer = 0.0f;               ///< Timer for input transmission
    float _inputSendInterval = 0.016f;          ///< Input send interval (~60 FPS)
};
