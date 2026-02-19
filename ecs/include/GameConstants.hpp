/*
** EPITECH PROJECT, 2025
** GameConstants.hpp
** File description:
** Shared game constants for R-Type (solo & multiplayer)
*/

#pragma once

namespace game {

// Damage values
constexpr int CONTACT_DAMAGE = 50;       // Player-enemy contact damage
constexpr int PLAYER_PROJECTILE_DAMAGE = 25;
constexpr int ENEMY_PROJECTILE_DAMAGE = 25;
constexpr int BOSS_PROJECTILE_DAMAGE = 30;

// Health values
constexpr int PLAYER_MAX_HP = 100;
constexpr int ENEMY_BASE_HP = 25;
constexpr int ENEMY_SPREAD_HP = 35;
constexpr int ENEMY_KAMIKAZE_HP = 15;
constexpr int KAMIKAZE_CONTACT_DAMAGE = 50; // double enemy shot damage (25)
constexpr int BOSS_BASE_HP = 1000;

// Shield values
constexpr int SHIELD_MAX = 50;
constexpr int SHIELD_POWERUP_AMOUNT = 50;

// Scoring
constexpr int SCORE_PER_SECOND = 1;
constexpr int SCORE_ENEMY_KILL = 5;
constexpr int SCORE_SPREAD_ENEMY_KILL = 10;
constexpr int BOSS_SPAWN_THRESHOLD = 300;  // Boss every 300 points in endless mode

// Laser beam powerup
constexpr float BEAM_DPS = 1000.0f;      // damage per second (5x normal weapon DPS)
constexpr float BEAM_HEIGHT = 30.0f;     // visual height in pixels
constexpr float LASER_DURATION = 5.0f;

// Weapon values
constexpr float PLAYER_FIRE_RATE = 8.0f;    // shots/sec
constexpr int SPREAD_PROJECTILE_COUNT = 3;
constexpr float SPREAD_ANGLE = 15.0f;       // degrees

// Speed values (normalized 0.0-1.0)
constexpr float PLAYER_PROJECTILE_SPEED = 0.75f;
constexpr float ENEMY_PROJECTILE_SPEED = 0.375f;
constexpr float ENEMY_BASE_SPEED = 0.15f;
constexpr float POWERUP_SPEED = 0.125f;

// Timing
constexpr float PROJECTILE_LIFETIME = 5.0f;
constexpr float INVULNERABILITY_DURATION = 0.5f;
constexpr float ENEMY_SPAWN_INTERVAL = 2.0f;
constexpr float POWERUP_SPAWN_INTERVAL = 10.0f;

// Multiplayer scaling
constexpr float BOSS_HP_SCALE_PER_PLAYER = 0.75f;  // +75% HP per additional player

} // namespace game
