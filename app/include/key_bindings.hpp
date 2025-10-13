/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** key_bindings
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include <map>
#include <string>

enum class GameAction { MoveLeft, MoveRight, MoveUp, MoveDown, Fire };

class KeyBindings {
  public:
    KeyBindings() {
        _bindings[GameAction::MoveLeft] = render::Key::Left;
        _bindings[GameAction::MoveRight] = render::Key::Right;
        _bindings[GameAction::MoveUp] = render::Key::Up;
        _bindings[GameAction::MoveDown] = render::Key::Down;
        _bindings[GameAction::Fire] = render::Key::Space;
    }

    void setBinding(GameAction action, render::Key key) {
        _bindings[action] = key;
    }

    render::Key getBinding(GameAction action) const {
        auto it = _bindings.find(action);
        if (it != _bindings.end()) {
            return it->second;
        }
        return render::Key::Unknown;
    }

    bool isKeyBound(GameAction action, render::Key key) const {
        auto it = _bindings.find(action);
        return it != _bindings.end() && it->second == key;
    }

    void resetToDefaults() {
        _bindings[GameAction::MoveLeft] = render::Key::Left;
        _bindings[GameAction::MoveRight] = render::Key::Right;
        _bindings[GameAction::MoveUp] = render::Key::Up;
        _bindings[GameAction::MoveDown] = render::Key::Down;
        _bindings[GameAction::Fire] = render::Key::Space;
    }

    static std::string getKeyName(render::Key key);

    std::map<GameAction, render::Key> &getBindingsMap() { return _bindings; }
    const std::map<GameAction, render::Key> &getBindingsMap() const {
        return _bindings;
    }

  private:
    std::map<GameAction, render::Key> _bindings;
};
