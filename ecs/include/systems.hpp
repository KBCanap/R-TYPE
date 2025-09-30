#pragma once
#include "registery.hpp"
#include "components.hpp"
#include <SFML/Graphics.hpp>

namespace systems {
    void position_system(registry& r,
                         sparse_array<component::position>& positions,
                         sparse_array<component::velocity>& velocities,
                         sparse_array<component::input>& inputs,
                         sf::RenderWindow& window,
                         float current_time,
                         float dt);

    void control_system(registry& r,
                        sparse_array<component::controllable>& controllables,
                        sparse_array<component::velocity>& velocities,
                        sparse_array<component::input>& inputs,
                        float dt);

    void render_system(registry& r,
                       sparse_array<component::position>& positions,
                       sparse_array<component::drawable>& drawables,
                       sf::RenderWindow& window,
                       float dt);

    void collision_system(registry& r,
                          sparse_array<component::position>& positions,
                          sparse_array<component::drawable>& drawables,
                          sparse_array<component::projectile>& projectiles,
                          sparse_array<component::hitbox>& hitboxes);

    void audio_system(registry& r,
                      sparse_array<component::sound_effect>& sound_effects,
                      sparse_array<component::music>& musics,
                      sparse_array<component::audio_trigger>& triggers);

    void input_system(registry& r,
                      sparse_array<component::input>& inputs);

    void weapon_system(registry& r,
                       sparse_array<component::weapon>& weapons,
                       sparse_array<component::position>& positions,
                       sparse_array<component::input>& inputs,
                       sparse_array<component::ai_input>& ai_inputs,
                       float current_time);

    void projectile_system(registry& r,
                           sparse_array<component::projectile>& projectiles,
                           sparse_array<component::position>& positions,
                           sf::RenderWindow& window,
                           float dt);

    void ai_input_system(registry& r,
                         sparse_array<component::ai_input>& ai_inputs,
                         float dt);

    void score_system(registry& r,
                      sparse_array<component::score>& scores,
                      float dt);

    void health_system(registry& r,
                       sparse_array<component::health>& healths,
                       float dt);
}
