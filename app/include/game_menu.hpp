#pragma once
#include <SFML/Graphics.hpp>
#include "audio_manager.hpp"

enum class MenuAction {
    NONE,
    REPLAY,
    QUIT
};

class GameOverMenu {
public:
    GameOverMenu(sf::RenderWindow& window, AudioManager& audioMgr);

    MenuAction handleEvents(const sf::Event& event);
    void render();
    void setVisible(bool visible);
    bool isVisible() const;
    void onWindowResize();

private:
    void initializeMenu();
    void updateButtonColors();

    sf::RenderWindow& _window;
    AudioManager& _audioManager;
    bool _visible;
    int _selectedButton;

    sf::Font _font;
    sf::Text _gameOverText;
    sf::Text _replayText;
    sf::Text _quitText;

    sf::RectangleShape _replayButton;
    sf::RectangleShape _quitButton;
    sf::RectangleShape _overlay;

    const sf::Color NORMAL_BUTTON_COLOR = sf::Color(100, 100, 100, 200);
    const sf::Color SELECTED_BUTTON_COLOR = sf::Color(150, 150, 150, 200);
    const sf::Color TEXT_COLOR = sf::Color::White;
    const sf::Color SELECTED_TEXT_COLOR = sf::Color::Yellow;
};