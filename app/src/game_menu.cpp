#include "../include/game_menu.hpp"
#include <SFML/Window/Keyboard.hpp>

GameOverMenu::GameOverMenu(sf::RenderWindow& window, AudioManager& audioMgr)
    : _window(window), _audioManager(audioMgr), _visible(false), _selectedButton(0)
{
    initializeMenu();
}

void GameOverMenu::initializeMenu() {

    if (!_font.loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");
    sf::Vector2u windowSize = _window.getSize();
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;

    _overlay.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    _overlay.setFillColor(sf::Color(0, 0, 0, 128));

    _gameOverText.setFont(_font);
    _gameOverText.setString("GAME OVER");
    _gameOverText.setCharacterSize(72);
    _gameOverText.setFillColor(sf::Color::Red);
    sf::FloatRect gameOverBounds = _gameOverText.getLocalBounds();
    _gameOverText.setPosition(centerX - gameOverBounds.width / 2, centerY - 150);

    _replayButton.setSize(sf::Vector2f(200, 60));
    _replayButton.setPosition(centerX - 100, centerY - 30);
    _replayButton.setFillColor(NORMAL_BUTTON_COLOR);

    _replayText.setFont(_font);
    _replayText.setString("REPLAY");
    _replayText.setCharacterSize(32);
    _replayText.setFillColor(TEXT_COLOR);
    sf::FloatRect replayBounds = _replayText.getLocalBounds();
    _replayText.setPosition(centerX - replayBounds.width / 2, centerY - 20);

    _quitButton.setSize(sf::Vector2f(200, 60));
    _quitButton.setPosition(centerX - 100, centerY + 50);
    _quitButton.setFillColor(NORMAL_BUTTON_COLOR);

    _quitText.setFont(_font);
    _quitText.setString("QUIT");
    _quitText.setCharacterSize(32);
    _quitText.setFillColor(TEXT_COLOR);
    sf::FloatRect quitBounds = _quitText.getLocalBounds();
    _quitText.setPosition(centerX - quitBounds.width / 2, centerY + 60);

    updateButtonColors();
}

MenuAction GameOverMenu::handleEvents(const sf::Event& event) {
    if (!_visible) return MenuAction::NONE;

    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
                _selectedButton = (_selectedButton - 1 + 2) % 2;
                updateButtonColors();
                break;
            case sf::Keyboard::Down:
                _selectedButton = (_selectedButton + 1) % 2;
                updateButtonColors();
                break;
            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                return (_selectedButton == 0) ? MenuAction::REPLAY : MenuAction::QUIT;
            case sf::Keyboard::Escape:
                return MenuAction::QUIT;
            default:
                break;
        }
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);

        if (_replayButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            return MenuAction::REPLAY;
        } else if (_quitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            return MenuAction::QUIT;
        }
    } else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i mousePos(event.mouseMove.x, event.mouseMove.y);

        if (_replayButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (_selectedButton != 0) {
                _selectedButton = 0;
                updateButtonColors();
            }
        } else if (_quitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (_selectedButton != 1) {
                _selectedButton = 1;
                updateButtonColors();
            }
        }
    }

    return MenuAction::NONE;
}

void GameOverMenu::render() {
    if (!_visible) return;

    _window.draw(_overlay);
    _window.draw(_gameOverText);
    _window.draw(_replayButton);
    _window.draw(_replayText);
    _window.draw(_quitButton);
    _window.draw(_quitText);
}

void GameOverMenu::setVisible(bool visible) {
    _visible = visible;
    if (visible) {
        _selectedButton = 0;
        updateButtonColors();
    }
}

bool GameOverMenu::isVisible() const {
    return _visible;
}

void GameOverMenu::updateButtonColors() {
    if (_selectedButton == 0) {
        _replayButton.setFillColor(SELECTED_BUTTON_COLOR);
        _replayText.setFillColor(SELECTED_TEXT_COLOR);
        _quitButton.setFillColor(NORMAL_BUTTON_COLOR);
        _quitText.setFillColor(TEXT_COLOR);
    } else {
        _replayButton.setFillColor(NORMAL_BUTTON_COLOR);
        _replayText.setFillColor(TEXT_COLOR);
        _quitButton.setFillColor(SELECTED_BUTTON_COLOR);
        _quitText.setFillColor(SELECTED_TEXT_COLOR);
    }
}

void GameOverMenu::onWindowResize() {
    initializeMenu();
}

VictoryMenu::VictoryMenu(sf::RenderWindow& window, AudioManager& audioMgr)
    : _window(window), _audioManager(audioMgr), _visible(false), _selectedButton(0)
{
    initializeMenu();
}

void VictoryMenu::initializeMenu() {

    if (!_font.loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");
    sf::Vector2u windowSize = _window.getSize();
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;

    _overlay.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    _overlay.setFillColor(sf::Color(0, 0, 0, 128));

    _victoryText.setFont(_font);
    _victoryText.setString("VICTORY");
    _victoryText.setCharacterSize(72);
    _victoryText.setFillColor(sf::Color::Green);
    sf::FloatRect victoryBounds = _victoryText.getLocalBounds();
    _victoryText.setPosition(centerX - victoryBounds.width / 2, centerY - 150);

    _replayButton.setSize(sf::Vector2f(200, 60));
    _replayButton.setPosition(centerX - 100, centerY - 30);
    _replayButton.setFillColor(NORMAL_BUTTON_COLOR);

    _replayText.setFont(_font);
    _replayText.setString("REPLAY");
    _replayText.setCharacterSize(32);
    _replayText.setFillColor(TEXT_COLOR);
    sf::FloatRect replayBounds = _replayText.getLocalBounds();
    _replayText.setPosition(centerX - replayBounds.width / 2, centerY - 20);

    _quitButton.setSize(sf::Vector2f(200, 60));
    _quitButton.setPosition(centerX - 100, centerY + 50);
    _quitButton.setFillColor(NORMAL_BUTTON_COLOR);

    _quitText.setFont(_font);
    _quitText.setString("QUIT");
    _quitText.setCharacterSize(32);
    _quitText.setFillColor(TEXT_COLOR);
    sf::FloatRect quitBounds = _quitText.getLocalBounds();
    _quitText.setPosition(centerX - quitBounds.width / 2, centerY + 60);

    updateButtonColors();
}

MenuAction VictoryMenu::handleEvents(const sf::Event& event) {
    if (!_visible) return MenuAction::NONE;

    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
                _selectedButton = (_selectedButton - 1 + 2) % 2;
                updateButtonColors();
                break;
            case sf::Keyboard::Down:
                _selectedButton = (_selectedButton + 1) % 2;
                updateButtonColors();
                break;
            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                return (_selectedButton == 0) ? MenuAction::REPLAY : MenuAction::QUIT;
            case sf::Keyboard::Escape:
                return MenuAction::QUIT;
            default:
                break;
        }
    } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);

        if (_replayButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            return MenuAction::REPLAY;
        } else if (_quitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            return MenuAction::QUIT;
        }
    } else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i mousePos(event.mouseMove.x, event.mouseMove.y);

        if (_replayButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (_selectedButton != 0) {
                _selectedButton = 0;
                updateButtonColors();
            }
        } else if (_quitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (_selectedButton != 1) {
                _selectedButton = 1;
                updateButtonColors();
            }
        }
    }

    return MenuAction::NONE;
}

void VictoryMenu::render() {
    if (!_visible) return;

    _window.draw(_overlay);
    _window.draw(_victoryText);
    _window.draw(_replayButton);
    _window.draw(_replayText);
    _window.draw(_quitButton);
    _window.draw(_quitText);
}

void VictoryMenu::setVisible(bool visible) {
    _visible = visible;
    if (visible) {
        _selectedButton = 0;
        updateButtonColors();
    }
}

bool VictoryMenu::isVisible() const {
    return _visible;
}

void VictoryMenu::updateButtonColors() {
    if (_selectedButton == 0) {
        _replayButton.setFillColor(SELECTED_BUTTON_COLOR);
        _replayText.setFillColor(SELECTED_TEXT_COLOR);
        _quitButton.setFillColor(NORMAL_BUTTON_COLOR);
        _quitText.setFillColor(TEXT_COLOR);
    } else {
        _replayButton.setFillColor(NORMAL_BUTTON_COLOR);
        _replayText.setFillColor(TEXT_COLOR);
        _quitButton.setFillColor(SELECTED_BUTTON_COLOR);
        _quitText.setFillColor(SELECTED_TEXT_COLOR);
    }
}

void VictoryMenu::onWindowResize() {
    initializeMenu();
}