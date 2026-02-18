/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_game
*/

#pragma once

#include "game/AudioManager.hpp"
#include "components.hpp"
#include "entity.hpp"
#include "registery.hpp"
#include "render/IRenderWindow.hpp"
#include "render/IRenderAudio.hpp"
#include <memory>
#include <unordered_map>
#include <string>

class MarioGame {
  public:
    MarioGame(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr);
    ~MarioGame() = default;

    void run();

  private:
    void handleEvents(bool &running, float dt);
    void updateControls(float dt);
    void update(float dt);
    void render(float dt);

    void createPlatforms();
    void createPowBlock(float scale_x, float scale_y);
    void createPlayer();
    void spawnEnemies();
    void spawnEnemy(float x, float y, bool moving_right);
    void updateEnemySpawning(float dt);

    void setupPlayerAnimation();
    void updatePlayerAnimation(float dt);
    void resetForNextLevel();

    void loadSounds();
    void playSound(const std::string &name);

    registry &_registry;
    render::IRenderWindow &_window;
    AudioManager &_audioManager;

    std::optional<entity> _background;
    std::optional<entity> _player;
    std::optional<entity> _powBlock;

    float _gameTime = 0.0f;
    bool _shouldExit = false;
    bool _gameOver = false;
    bool _victory = false;
    bool _playerFacingRight = true;
    int _level = 1;
    float _enemyBaseSpeed = 75.0f;

    std::shared_ptr<render::IFont> _debugFont;

    float _enemySpawnTimer = 0.0f;
    int _enemiesSpawned = 0;
    int _totalEnemiesToSpawn = 3;
    float _enemySpawnInterval = 4.0f;

    std::unordered_map<std::string, render::ISoundBuffer*> _soundBuffers;
    std::unordered_map<std::string, render::ISound*> _sounds;

    bool _gameOverSoundPlayed = false;
    int _lastStunnedCount = 0;
    int _lastPowHits = 3;
    int _lastEnemyCount = 0;
    float _walkSoundTimer = 0.0f;
    const float _walkSoundInterval = 0.25f;
};
