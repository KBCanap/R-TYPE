/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** accessibility_menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include "settings.hpp"
#include <memory>

enum class AccessibilityResult { None, Back, Resumed };

class AccessibilityMenu {
  public:
    AccessibilityMenu(render::IRenderWindow &win, AudioManager &audioMgr);

    AccessibilityResult run();
    void render();

  private:
    void createButtons();
    void updateButtonScale();
    void cycleColorblindMode(
        int direction); // direction: -1 for previous, +1 for next

  private:
    render::IRenderWindow &_window;
    AudioManager &_audioManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;

    // Colorblind mode settings
    ColorblindMode _currentMode;
    std::unique_ptr<render::IText> _colorblindLabel;
    std::unique_ptr<render::IText> _colorblindValue;
    std::unique_ptr<render::IShape> _colorblindLeftButton;
    std::unique_ptr<render::IText> _colorblindLeftText;
    std::unique_ptr<render::IShape> _colorblindRightButton;
    std::unique_ptr<render::IText> _colorblindRightText;

    // Reference squares for colorblind testing (with different colors)
    std::unique_ptr<render::IShape> _referenceSquare1; // Red
    std::unique_ptr<render::IShape> _referenceSquare2; // Green
    std::unique_ptr<render::IShape> _referenceSquare3; // Blue
    std::unique_ptr<render::IText> _referenceLabel;

    // Back button
    std::unique_ptr<render::IShape> _backButton;
    std::unique_ptr<render::IText> _backButtonText;

    render::Vector2u _windowSize;

    // Background (scrolling)
    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;
    float _bgScrollSpeed;
};