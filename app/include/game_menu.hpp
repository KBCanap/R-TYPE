#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include <memory>

enum class MenuAction { NONE, REPLAY, QUIT };

class GameOverMenu {
  public:
    GameOverMenu(render::IRenderWindow &window, AudioManager &audioMgr);

    MenuAction handleEvents(const render::Event &event);
    void render();
    void setVisible(bool visible);
    bool isVisible() const;
    void onWindowResize();

  private:
    void initializeMenu();
    void updateButtonColors();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    bool _visible;
    int _selectedButton;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _gameOverText;
    std::unique_ptr<render::IText> _replayText;
    std::unique_ptr<render::IText> _quitText;

    std::unique_ptr<render::IShape> _replayButton;
    std::unique_ptr<render::IShape> _quitButton;
    std::unique_ptr<render::IShape> _overlay;

    const render::Color NORMAL_BUTTON_COLOR = render::Color(100, 100, 100, 200);
    const render::Color SELECTED_BUTTON_COLOR =
        render::Color(150, 150, 150, 200);
    const render::Color TEXT_COLOR = render::Color::White();
    const render::Color SELECTED_TEXT_COLOR = render::Color::Yellow();
};

class VictoryMenu {
  public:
    VictoryMenu(render::IRenderWindow &window, AudioManager &audioMgr);

    MenuAction handleEvents(const render::Event &event);
    void render();
    void setVisible(bool visible);
    bool isVisible() const;
    void onWindowResize();

  private:
    void initializeMenu();
    void updateButtonColors();

    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    bool _visible;
    int _selectedButton;

    std::unique_ptr<render::IFont> _font;
    std::unique_ptr<render::IText> _victoryText;
    std::unique_ptr<render::IText> _replayText;
    std::unique_ptr<render::IText> _quitText;

    std::unique_ptr<render::IShape> _replayButton;
    std::unique_ptr<render::IShape> _quitButton;
    std::unique_ptr<render::IShape> _overlay;

    const render::Color NORMAL_BUTTON_COLOR = render::Color(100, 100, 100, 200);
    const render::Color SELECTED_BUTTON_COLOR =
        render::Color(150, 150, 150, 200);
    const render::Color TEXT_COLOR = render::Color::White();
    const render::Color SELECTED_TEXT_COLOR = render::Color::Yellow();
};