/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** keybindings_menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include "key_bindings.hpp"
#include <memory>
#include <vector>

enum class KeyBindingsResult { None, Back };

class KeyBindingsMenu {
  public:
    KeyBindingsMenu(render::IRenderWindow &win, AudioManager &audioMgr,
                    KeyBindings &keyBindings);

    KeyBindingsResult run();
    void render();

  private:
    void createButtons();
    void updateButtonScale();
    void startRebinding(GameAction action);
    void handleKeyPress(render::Key key);
    void resetToDefaults();

  private:
    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    KeyBindings &_keyBindings;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;

    struct KeyBindingEntry {
        GameAction action;
        std::string actionName;
        std::unique_ptr<render::IText> label;
        std::unique_ptr<render::IText> keyText;
        std::unique_ptr<render::IShape> button;
        std::unique_ptr<render::IText> buttonText;
    };
    std::vector<KeyBindingEntry> _bindings;

    std::unique_ptr<render::IShape> _resetButton;
    std::unique_ptr<render::IText> _resetButtonText;

    std::unique_ptr<render::IShape> _backButton;
    std::unique_ptr<render::IText> _backButtonText;

    bool _waitingForKey;
    GameAction _currentAction;
    std::unique_ptr<render::IText> _waitingText;

    render::Vector2u _windowSize;

    // Background (scrolling)
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;
    float _bgScrollSpeed;
};
