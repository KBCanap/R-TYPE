#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "registery.hpp"
#include "components.hpp"
#include "game_menu.hpp"
#include "audio_manager.hpp"
#include "tick_system.hpp"
#include "player_manager.hpp"
#include "enemy_manager.hpp"
#include "boss_manager.hpp"
#include <vector>

class Game {
public:
    Game(registry& reg, sf::RenderWindow& win, AudioManager& audioMgr);

    void run();  // Boucle principale du jeu avec système de tick

private:
    void handleEvents(bool& running, float dt);
    void update(float dt);
    void render(float dt);
    void resetGame();
    bool isPlayerAlive() const;

    // Core game logic only - details moved to managers

private:
    registry& _registry;
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    // Managers for different game aspects
    PlayerManager _playerManager;
    EnemyManager _enemyManager;
    BossManager _bossManager;

    std::optional<entity> _player;       // Player créé via spawn_entity()
    std::optional<entity> _background;   // Background entity
    std::vector<entity> _enemies;        // Ennemis
    std::optional<entity> _boss;         // Boss entity

    float _playerSpeed = 300.f;
    float _enemySpawnTimer = 0.f;
    float _enemySpawnInterval = 2.f;
    float _gameTime = 0.f;

    // Relative positions (0.0 to 1.0)
    float _playerRelativeX = 0.125f;  // 100/800 = 0.125
    float _playerRelativeY = 0.5f;    // 300/600 = 0.5

    GameOverMenu _gameOverMenu;
    bool _gameOver = false;

    TickSystem _tickSystem;
    bool _shouldExit = false;

    sf::Font _scoreFont;  // Font for displaying score using r-type.otf
};
