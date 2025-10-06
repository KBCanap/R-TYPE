#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <memory>

enum class AccessibilityResult { None, Back, Resumed };

class AccessibilityMenu {
public:
    AccessibilityMenu(render::IRenderWindow& win, AudioManager& audioMgr);

    AccessibilityResult run();
    void render();

private:
    void createButtons();
    void updateButtonScale();
    void updateContrastSlider(float mouseX);
    void updateContrastSliderPosition();

private:
    render::IRenderWindow& _window;
    AudioManager& _audioManager;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;

    // Contrast settings
    float _currentContrast;
    std::unique_ptr<render::IText> _contrastLabel;
    std::unique_ptr<render::IText> _contrastValue;
    std::unique_ptr<render::IShape> _contrastSliderTrack;
    std::unique_ptr<render::IShape> _contrastSliderHandle;
    bool _isDraggingContrastSlider;
    float _contrastSliderMin;
    float _contrastSliderMax;

    // Reference square for contrast testing
    std::unique_ptr<render::IShape> _referenceSquare;
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