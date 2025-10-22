/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_game
*/

#pragma once

#include "audio_manager.hpp"
#include "components.hpp"
#include "entity.hpp"
#include "registery.hpp"
#include "render/IRenderWindow.hpp"
#include <memory>

class MarioGame {
  public:
    MarioGame(registry &reg, render::IRenderWindow &win,
              AudioManager &audioMgr);
    ~MarioGame() = default;

    void run();

  private:
    // Event handling (mario_controls.cpp)
    void handleEvents(bool &running, float dt);
    void updateControls(float dt);

    // Game loop (mario_game.cpp)
    void update(float dt);

    // Rendering (mario_render.cpp)
    void render();
    void renderDebugInfo();

    // Entity creation (mario_platforms.cpp, mario_player.cpp, mario_enemies.cpp)
    void createPlatforms();
    void createPlayer();
    void spawnEnemies();
    void spawnEnemy(float x, float y, bool moving_right);
    void updateEnemySpawning(float dt);

    // Animation (mario_animation.cpp)
    void setupPlayerAnimation();
    void updatePlayerAnimation(float dt);

    registry &_registry;
    render::IRenderWindow &_window;
    AudioManager &_audioManager;

    std::optional<entity> _background;
    std::optional<entity> _player;

    float _gameTime = 0.0f;
    bool _shouldExit = false;
    bool _playerFacingRight = true;

    std::shared_ptr<render::IFont> _debugFont;

    // Enemy spawning
    float _enemySpawnTimer = 0.0f;
    int _enemiesSpawned = 0;
    int _totalEnemiesToSpawn = 3;
    float _enemySpawnInterval = 1.0f;
};
