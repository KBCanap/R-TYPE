#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "registery.hpp"
#include "components.hpp"
#include "game_menu.hpp"
#include "audio_manager.hpp"
#include "tick_system.hpp"
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

    // Utility methods for responsive positioning
    float getRelativeX(float relativeX) const;
    float getRelativeY(float relativeY) const;
    void updatePlayerPosition();
    void updateEnemyPositions();
    void changePlayerWeaponToSingle();
    void changePlayerWeaponToRapid();
    void changePlayerWeaponToBurst();
    void changePlayerWeaponToSpread();

    // Enemy weapon creation functions
    component::weapon createEnemySingleWeapon();
    component::weapon createEnemyBurstWeapon();
    component::weapon createEnemySpreadWeapon();
    component::weapon createEnemyZigzagSpreadWeapon();

    // Function pointer type for enemy weapon creators
    using EnemyWeaponCreator = component::weapon (Game::*)();

    // Array of function pointers for enemy weapons
    static constexpr int NUM_ENEMY_WEAPON_TYPES = 4;
    EnemyWeaponCreator enemyWeaponCreators[NUM_ENEMY_WEAPON_TYPES];

private:
    registry& _registry;
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    std::optional<entity> _player;       // Player créé via spawn_entity()
    std::optional<entity> _background;   // Background entity
    std::vector<entity> _enemies;        // Ennemis

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
