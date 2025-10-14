/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** pause_menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "accessibility_menu.hpp"
#include "audio_manager.hpp"
#include "options_menu.hpp"
#include <memory>

enum class PauseResult { None, Continue, Options, Accessibility, Quit };

class PauseMenu {
  public:
    PauseMenu(render::IRenderWindow &win, AudioManager &audioMgr);

    PauseResult run();
    void render();
    void setVisible(bool visible) { _visible = visible; }
    bool isVisible() const { return _visible; }
    const render::IShape &getContinueButton() const { return *_continueButton; }
    const render::IShape &getOptionsButton() const { return *_optionsButton; }
    const render::IShape &getAccessibilityButton() const {
        return *_accessibilityButton;
    }
    const render::IShape &getQuitButton() const { return *_quitButton; }

  private:
    void createButtons();
    void updateButtonScale();

  private:
    render::IRenderWindow &_window;
    AudioManager &_audioManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;

    std::unique_ptr<render::IShape> _continueButton;
    std::unique_ptr<render::IShape> _optionsButton;
    std::unique_ptr<render::IShape> _accessibilityButton;
    std::unique_ptr<render::IShape> _quitButton;

    std::unique_ptr<render::IText> _continueButtonText;
    std::unique_ptr<render::IText> _optionsButtonText;
    std::unique_ptr<render::IText> _accessibilityButtonText;
    std::unique_ptr<render::IText> _quitButtonText;
    std::unique_ptr<render::IShape> _overlay;

    render::Vector2u _windowSize;
    bool _visible;
};