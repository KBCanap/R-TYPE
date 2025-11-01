/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_controls - Input handling and controls
*/

#include "mario_game.hpp"
#include "systems.hpp"

void MarioGame::handleEvents(bool &running, float /*dt*/) {
    render::Event event;
    while (_window.pollEvent(event)) {
        systems::update_key_state(event);

        if (event.type == render::EventType::Closed) {
            running = false;
            _shouldExit = true;
            return;
        }

        if (event.type == render::EventType::KeyPressed) {
            if (event.key.code == render::Key::Escape) {
                running = false;
                _shouldExit = true;
                return;
            }
        }
    }
}

void MarioGame::updateControls(float /*dt*/) {
    auto &inputs = _registry.get_components<component::input>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &controllables = _registry.get_components<component::controllable>();
    auto &gravities = _registry.get_components<component::gravity>();

    // Input system
    systems::input_system(_registry, inputs, _window, nullptr);

    // Manual control for Mario - ONLY horizontal movement
    for (size_t i = 0; i < inputs.size() && i < velocities.size() &&
                       i < controllables.size();
         ++i) {
        auto &input = inputs[i];
        auto &vel = velocities[i];
        auto &ctrl = controllables[i];

        if (!input || !vel || !ctrl)
            continue;

        // Horizontal movement only
        vel->vx = 0.0f;
        if (input->left) {
            vel->vx = -ctrl->speed;
        } else if (input->right) {
            vel->vx = ctrl->speed;
        }
    }

    // Handle jump input
    if (_player) {
        auto &player_input = inputs[*_player];
        auto &player_gravity = gravities[*_player];
        auto &player_vel = velocities[*_player];

        if (player_input && player_gravity && player_vel) {
            // Jump when up is pressed and player is on ground
            if (player_input->up && player_gravity->on_ground) {
                player_vel->vy = -player_gravity->jump_strength;
                player_gravity->on_ground = false;
            }
        }
    }
}
