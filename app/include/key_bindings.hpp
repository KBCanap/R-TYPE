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
        // Bindings par défaut
        _bindings[GameAction::MoveLeft] = render::Key::Left;
        _bindings[GameAction::MoveRight] = render::Key::Right;
        _bindings[GameAction::MoveUp] = render::Key::Up;
        _bindings[GameAction::MoveDown] = render::Key::Down;
        _bindings[GameAction::Fire] = render::Key::Space;
    }

    // Définir une nouvelle touche pour une action
    void setBinding(GameAction action, render::Key key) {
        _bindings[action] = key;
    }

    // Obtenir la touche actuelle pour une action
    render::Key getBinding(GameAction action) const {
        auto it = _bindings.find(action);
        if (it != _bindings.end()) {
            return it->second;
        }
        return render::Key::Unknown;
    }

    // Vérifier si une touche est assignée à une action
    bool isKeyBound(GameAction action, render::Key key) const {
        auto it = _bindings.find(action);
        return it != _bindings.end() && it->second == key;
    }

    // Réinitialiser aux bindings par défaut
    void resetToDefaults() {
        _bindings[GameAction::MoveLeft] = render::Key::Left;
        _bindings[GameAction::MoveRight] = render::Key::Right;
        _bindings[GameAction::MoveUp] = render::Key::Up;
        _bindings[GameAction::MoveDown] = render::Key::Down;
        _bindings[GameAction::Fire] = render::Key::Space;
    }

    // Obtenir le nom d'une touche (pour l'affichage)
    static std::string getKeyName(render::Key key);

    // Méthodes pour la persistance
    std::map<GameAction, render::Key> &getBindingsMap() { return _bindings; }
    const std::map<GameAction, render::Key> &getBindingsMap() const {
        return _bindings;
    }

  private:
    std::map<GameAction, render::Key> _bindings;
};
