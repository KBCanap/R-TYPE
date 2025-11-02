/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** input_system
*/

#include "../../app/include/key_bindings.hpp"
#include "../../include/systems.hpp"
#include "../include/render/IRenderWindow.hpp"
#include <set>

namespace systems {

static std::set<render::Key> g_pressed_keys;

void update_key_state(const render::Event &event) {
    // Branchless key state update using function pointers
    bool is_pressed = (event.type == render::EventType::KeyPressed);
    bool is_released = (event.type == render::EventType::KeyReleased);

    if (is_pressed) {
        g_pressed_keys.insert(event.key.code);
    }
    if (is_released) {
        g_pressed_keys.erase(event.key.code);
    }
}

void input_system(registry & /*r*/, sparse_array<component::input> &inputs,
                  render::IRenderWindow & /*window*/,
                  KeyBindings *keyBindings) {
    // Branchless key selection using ternary operator chain
    bool has_bindings = (keyBindings != nullptr);

    render::Key key_left = has_bindings
                               ? keyBindings->getBinding(GameAction::MoveLeft)
                               : render::Key::Left;
    render::Key key_right = has_bindings
                                ? keyBindings->getBinding(GameAction::MoveRight)
                                : render::Key::Right;
    render::Key key_up = has_bindings
                             ? keyBindings->getBinding(GameAction::MoveUp)
                             : render::Key::Up;
    render::Key key_down = has_bindings
                               ? keyBindings->getBinding(GameAction::MoveDown)
                               : render::Key::Down;
    render::Key key_fire = has_bindings
                               ? keyBindings->getBinding(GameAction::Fire)
                               : render::Key::Space;

    struct KeyState {
        render::Key key;
        bool component::input::*current_field;
        bool component::input::*pressed_field;
    };

    static const KeyState key_states[] = {
        {key_left, &component::input::left, &component::input::left_pressed},
        {key_right, &component::input::right, &component::input::right_pressed},
        {key_up, &component::input::up, &component::input::up_pressed},
        {key_down, &component::input::down, &component::input::down_pressed},
        {key_fire, &component::input::fire, &component::input::fire_pressed}};

    for (size_t i = 0; i < inputs.size(); ++i) {
        std::optional<component::input> &input = inputs[i];
        if (!input)
            continue;

        // Process all keys in a loop to reduce branching
        for (const KeyState &ks : key_states) {
            bool prev_state = (*input).*(ks.current_field);
            bool curr_state = g_pressed_keys.count(ks.key) > 0;

            (*input).*(ks.current_field) = curr_state;
            (*input).*(ks.pressed_field) = (!prev_state) & curr_state;
        }
    }
}

} // namespace systems
