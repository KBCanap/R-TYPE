/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** systems
*/

#pragma once
#include "components.hpp"
#include "registery.hpp"
#include "render/IRenderAudio.hpp"
#include "render/IRenderWindow.hpp"

// Forward declaration
class KeyBindings;

namespace systems {

/**
 * @brief Check if a tag corresponds to an enemy entity
 * @param tag The tag string to check
 * @return true if the tag represents an enemy, false otherwise
 */
bool is_enemy_tag(const std::string &tag);

/**
 * @brief Create an explosion effect at the specified coordinates
 * @param r Registry reference for entity management
 * @param x X coordinate of the explosion
 * @param y Y coordinate of the explosion
 */
void create_explosion(registry &r, float x, float y);

/**
 * @brief Update the state of keyboard inputs based on events
 * @param event The input event to process
 */
void update_key_state(const render::Event &event);

/**
 * @brief Update entity positions based on velocity and input components
 * @param r Registry reference
 * @param positions Sparse array of position components
 * @param velocities Sparse array of velocity components
 * @param inputs Sparse array of input components
 * @param window Render window reference for boundary checks
 * @param current_time Current game time
 * @param dt Delta time for frame-independent movement
 */
void position_system(registry &r, sparse_array<component::position> &positions,
                     sparse_array<component::velocity> &velocities,
                     sparse_array<component::input> &inputs,
                     render::IRenderWindow &window, float current_time,
                     float dt);

/**
 * @brief Process controllable entities and update their velocity based on input
 * @param r Registry reference
 * @param controllables Sparse array of controllable components
 * @param velocities Sparse array of velocity components
 * @param inputs Sparse array of input components
 * @param dt Delta time for consistent movement
 */
void control_system(registry &r,
                    sparse_array<component::controllable> &controllables,
                    sparse_array<component::velocity> &velocities,
                    sparse_array<component::input> &inputs, float dt);

/**
 * @brief Render all drawable entities to the screen
 * @param r Registry reference
 * @param positions Sparse array of position components
 * @param drawables Sparse array of drawable components
 * @param window Render window for drawing operations
 * @param dt Delta time for animations
 */
void render_system(registry &r, sparse_array<component::position> &positions,
                   sparse_array<component::drawable> &drawables,
                   render::IRenderWindow &window, float dt);

/**
 * @brief Handle collision detection between projectiles, players, enemies, and
 * powerups
 * @param r Registry reference for entity management
 * @param positions Sparse array of position components
 * @param drawables Sparse array of drawable components
 * @param projectiles Sparse array of projectile components
 * @param hitboxes Sparse array of hitbox components
 */
void collision_system(registry &r, sparse_array<component::position> &positions,
                      sparse_array<component::drawable> &drawables,
                      sparse_array<component::projectile> &projectiles,
                      sparse_array<component::hitbox> &hitboxes);

/**
 * @brief Process audio effects and music playback
 * @param r Registry reference
 * @param sound_effects Sparse array of sound effect components
 * @param musics Sparse array of music components
 * @param triggers Sparse array of audio trigger components
 * @param audioManager Audio manager for sound operations
 */
void audio_system(registry &r,
                  sparse_array<component::sound_effect> &sound_effects,
                  sparse_array<component::music> &musics,
                  sparse_array<component::audio_trigger> &triggers,
                  render::IRenderAudio &audioManager);

/**
 * @brief Capture and process player input from keyboard and other sources
 * @param r Registry reference
 * @param inputs Sparse array of input components to update
 * @param window Render window for input polling
 * @param keyBindings Optional key binding configuration
 */
void input_system(registry &r, sparse_array<component::input> &inputs,
                  render::IRenderWindow &window,
                  KeyBindings *keyBindings = nullptr);

/**
 * @brief Handle weapon firing and projectile creation
 * @param r Registry reference for creating projectiles
 * @param weapons Sparse array of weapon components
 * @param positions Sparse array of position components
 * @param inputs Sparse array of input components
 * @param ai_inputs Sparse array of AI input components
 * @param current_time Current game time for firing rate control
 */
void weapon_system(registry &r, sparse_array<component::weapon> &weapons,
                   sparse_array<component::position> &positions,
                   sparse_array<component::input> &inputs,
                   sparse_array<component::ai_input> &ai_inputs,
                   float current_time);

/**
 * @brief Update projectile movement, lifetime, and cleanup
 * @param r Registry reference for entity management
 * @param projectiles Sparse array of projectile components
 * @param positions Sparse array of position components
 * @param window Render window for boundary checks
 * @param dt Delta time for movement calculations
 */
void projectile_system(registry &r,
                       sparse_array<component::projectile> &projectiles,
                       sparse_array<component::position> &positions,
                       render::IRenderWindow &window, float dt);

/**
 * @brief Process AI input generation for enemy entities
 * @param r Registry reference
 * @param ai_inputs Sparse array of AI input components
 * @param dt Delta time for AI decision timing
 */
void ai_input_system(registry &r, sparse_array<component::ai_input> &ai_inputs,
                     float dt);

/**
 * @brief Update and manage player scores and statistics
 * @param r Registry reference
 * @param scores Sparse array of score components
 * @param dt Delta time for time-based scoring
 */
void score_system(registry &r, sparse_array<component::score> &scores,
                  float dt);

/**
 * @brief Process health changes, damage application, and entity destruction
 * @param r Registry reference for entity management
 * @param healths Sparse array of health components
 * @param dt Delta time for regeneration effects
 */
void health_system(registry &r, sparse_array<component::health> &healths,
                   float dt);

} // namespace systems
