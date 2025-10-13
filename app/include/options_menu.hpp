/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** options_menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include "key_bindings.hpp"
#include <memory>
#include <vector>

enum class OptionsResult {
    None,
    Back,
    ResolutionChanged,
    SoundToggled,
    Accessibility,
    KeyBindings
};

struct Resolution {
    unsigned int width;
    unsigned int height;
    std::string name;
};

class OptionsMenu {
  public:
    OptionsMenu(render::IRenderWindow &win, AudioManager &audioMgr,
                KeyBindings &keyBindings);

    OptionsResult run();
    void render();

    // Getters for settings
    Resolution getCurrentResolution() const {
        return _resolutions[_currentResolution];
    }
    bool isSoundEnabled() const { return _soundEnabled; }

  private:
    void createButtons();
    void updateButtonScale();
    void handleResolutionChange();
    void toggleSound();

  private:
    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    KeyBindings &_keyBindings;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;

    // Resolution settings
    std::vector<Resolution> _resolutions;
    size_t _currentResolution;
    std::unique_ptr<render::IText> _resolutionLabel;
    std::unique_ptr<render::IText> _resolutionValue;
    std::unique_ptr<render::IShape> _resolutionLeftButton;
    std::unique_ptr<render::IShape> _resolutionRightButton;
    std::unique_ptr<render::IText> _resolutionLeftText;
    std::unique_ptr<render::IText> _resolutionRightText;

    // Sound settings
    bool _soundEnabled;
    std::unique_ptr<render::IText> _soundLabel;
    std::unique_ptr<render::IText> _soundValue;
    std::unique_ptr<render::IShape> _soundButton;
    std::unique_ptr<render::IText> _soundButtonText;

    // Accessibility button
    std::unique_ptr<render::IShape> _accessibilityButton;
    std::unique_ptr<render::IText> _accessibilityButtonText;

    // Key Bindings button
    std::unique_ptr<render::IShape> _keyBindingsButton;
    std::unique_ptr<render::IText> _keyBindingsButtonText;

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