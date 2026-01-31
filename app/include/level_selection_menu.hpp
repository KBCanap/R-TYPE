/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** level_selection_menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <memory>
#include <vector>

enum class LevelSelectionResult { BACK, LEVEL_1, LEVEL_2, ENDLESS };

class LevelSelectionMenu {
  public:
    LevelSelectionMenu(render::IRenderWindow &window, AudioManager &audioMgr);

    LevelSelectionResult run();

  private:
    void initializeMenu();
    void handleEvents(bool &running);
    void update(float dt);
    void render();
    void updateButtonColors();
    bool isMouseOverButton(int buttonIndex,
                           const render::Vector2f &mousePos) const;

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    int _selectedButton;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _titleText;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;
    std::unique_ptr<render::ITexture> _bgTexture;

    // Buttons and texts
    std::vector<std::unique_ptr<render::IShape>> _buttons;
    std::vector<std::unique_ptr<render::IText>> _buttonTexts;
    std::vector<std::string> _buttonLabels;

    // Background scrolling
    float _bgScrollSpeed;

    const render::Color NORMAL_BUTTON_COLOR = render::Color(100, 100, 100, 200);
    const render::Color SELECTED_BUTTON_COLOR =
        render::Color(150, 150, 150, 200);
    const render::Color TEXT_COLOR = render::Color::White();
    const render::Color SELECTED_TEXT_COLOR = render::Color::Yellow();
};
