#include "../include/game_menu.hpp"
#include <stdexcept>

// Helper function to check if a FloatRect contains a point
static bool containsPoint(const render::FloatRect& rect, const render::Vector2f& point) {
    return point.x >= rect.left && point.x <= rect.left + rect.width &&
           point.y >= rect.top && point.y <= rect.top + rect.height;
}

GameOverMenu::GameOverMenu(render::IRenderWindow& window, AudioManager& audioMgr)
    : _window(window), _audioManager(audioMgr), _visible(false), _selectedButton(0)
{
    initializeMenu();
}

void GameOverMenu::initializeMenu() {

    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");
    render::Vector2u windowSize = _window.getSize();
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;

    _overlay = _window.createRectangleShape(render::Vector2f(windowSize.x, windowSize.y));
    _overlay->setFillColor(render::Color(0, 0, 0, 128));

    _gameOverText = _window.createText();
    _gameOverText->setFont(*_font);
    _gameOverText->setString("GAME OVER");
    _gameOverText->setCharacterSize(72);
    _gameOverText->setFillColor(render::Color::Red());
    render::FloatRect gameOverBounds = _gameOverText->getLocalBounds();
    _gameOverText->setPosition(centerX - gameOverBounds.width / 2, centerY - 150);

    _replayButton = _window.createRectangleShape(render::Vector2f(200, 60));
    _replayButton->setPosition(centerX - 100, centerY - 30);
    _replayButton->setFillColor(NORMAL_BUTTON_COLOR);

    _replayText = _window.createText();
    _replayText->setFont(*_font);
    _replayText->setString("REPLAY");
    _replayText->setCharacterSize(32);
    _replayText->setFillColor(TEXT_COLOR);
    render::FloatRect replayBounds = _replayText->getLocalBounds();
    _replayText->setPosition(centerX - replayBounds.width / 2, centerY - 20);

    _quitButton = _window.createRectangleShape(render::Vector2f(200, 60));
    _quitButton->setPosition(centerX - 100, centerY + 50);
    _quitButton->setFillColor(NORMAL_BUTTON_COLOR);

    _quitText = _window.createText();
    _quitText->setFont(*_font);
    _quitText->setString("QUIT");
    _quitText->setCharacterSize(32);
    _quitText->setFillColor(TEXT_COLOR);
    render::FloatRect quitBounds = _quitText->getLocalBounds();
    _quitText->setPosition(centerX - quitBounds.width / 2, centerY + 60);

    updateButtonColors();
}

MenuAction GameOverMenu::handleEvents(const render::Event& event) {
    if (!_visible) return MenuAction::NONE;

    if (event.type == render::EventType::KeyPressed) {
        switch (event.key.code) {
            case render::Key::Up:
                _selectedButton = (_selectedButton - 1 + 2) % 2;
                updateButtonColors();
                break;
            case render::Key::Down:
                _selectedButton = (_selectedButton + 1) % 2;
                updateButtonColors();
                break;
            case render::Key::Enter:
            case render::Key::Space:
                return (_selectedButton == 0) ? MenuAction::REPLAY : MenuAction::QUIT;
            case render::Key::Escape:
                return MenuAction::QUIT;
            default:
                break;
        }
    } else if (event.type == render::EventType::MouseButtonPressed && event.mouseButton.button == render::Mouse::Left) {
        render::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        if (containsPoint(_replayButton->getGlobalBounds(), mousePos)) {
            return MenuAction::REPLAY;
        } else if (containsPoint(_quitButton->getGlobalBounds(), mousePos)) {
            return MenuAction::QUIT;
        }
    } else if (event.type == render::EventType::MouseMoved) {
        render::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        if (containsPoint(_replayButton->getGlobalBounds(), mousePos)) {
            if (_selectedButton != 0) {
                _selectedButton = 0;
                updateButtonColors();
            }
        } else if (containsPoint(_quitButton->getGlobalBounds(), mousePos)) {
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

    _window.draw(*_overlay);
    _window.draw(*_gameOverText);
    _window.draw(*_replayButton);
    _window.draw(*_replayText);
    _window.draw(*_quitButton);
    _window.draw(*_quitText);
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
        _replayButton->setFillColor(SELECTED_BUTTON_COLOR);
        _replayText->setFillColor(SELECTED_TEXT_COLOR);
        _quitButton->setFillColor(NORMAL_BUTTON_COLOR);
        _quitText->setFillColor(TEXT_COLOR);
    } else {
        _replayButton->setFillColor(NORMAL_BUTTON_COLOR);
        _replayText->setFillColor(TEXT_COLOR);
        _quitButton->setFillColor(SELECTED_BUTTON_COLOR);
        _quitText->setFillColor(SELECTED_TEXT_COLOR);
    }
}

void GameOverMenu::onWindowResize() {
    initializeMenu();
}

VictoryMenu::VictoryMenu(render::IRenderWindow& window, AudioManager& audioMgr)
    : _window(window), _audioManager(audioMgr), _visible(false), _selectedButton(0)
{
    initializeMenu();
}

void VictoryMenu::initializeMenu() {

    _font = _window.createFont();
    if (!_font->loadFromFile("assets/r-type.otf"))
        throw std::runtime_error("Failed to load r-type.otf");
    render::Vector2u windowSize = _window.getSize();
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;

    _overlay = _window.createRectangleShape(render::Vector2f(windowSize.x, windowSize.y));
    _overlay->setFillColor(render::Color(0, 0, 0, 128));

    _victoryText = _window.createText();
    _victoryText->setFont(*_font);
    _victoryText->setString("VICTORY");
    _victoryText->setCharacterSize(72);
    _victoryText->setFillColor(render::Color::Green());
    render::FloatRect victoryBounds = _victoryText->getLocalBounds();
    _victoryText->setPosition(centerX - victoryBounds.width / 2, centerY - 150);

    _replayButton = _window.createRectangleShape(render::Vector2f(200, 60));
    _replayButton->setPosition(centerX - 100, centerY - 30);
    _replayButton->setFillColor(NORMAL_BUTTON_COLOR);

    _replayText = _window.createText();
    _replayText->setFont(*_font);
    _replayText->setString("REPLAY");
    _replayText->setCharacterSize(32);
    _replayText->setFillColor(TEXT_COLOR);
    render::FloatRect replayBounds = _replayText->getLocalBounds();
    _replayText->setPosition(centerX - replayBounds.width / 2, centerY - 20);

    _quitButton = _window.createRectangleShape(render::Vector2f(200, 60));
    _quitButton->setPosition(centerX - 100, centerY + 50);
    _quitButton->setFillColor(NORMAL_BUTTON_COLOR);

    _quitText = _window.createText();
    _quitText->setFont(*_font);
    _quitText->setString("QUIT");
    _quitText->setCharacterSize(32);
    _quitText->setFillColor(TEXT_COLOR);
    render::FloatRect quitBounds = _quitText->getLocalBounds();
    _quitText->setPosition(centerX - quitBounds.width / 2, centerY + 60);

    updateButtonColors();
}

MenuAction VictoryMenu::handleEvents(const render::Event& event) {
    if (!_visible) return MenuAction::NONE;

    if (event.type == render::EventType::KeyPressed) {
        switch (event.key.code) {
            case render::Key::Up:
                _selectedButton = (_selectedButton - 1 + 2) % 2;
                updateButtonColors();
                break;
            case render::Key::Down:
                _selectedButton = (_selectedButton + 1) % 2;
                updateButtonColors();
                break;
            case render::Key::Enter:
            case render::Key::Space:
                return (_selectedButton == 0) ? MenuAction::REPLAY : MenuAction::QUIT;
            case render::Key::Escape:
                return MenuAction::QUIT;
            default:
                break;
        }
    } else if (event.type == render::EventType::MouseButtonPressed && event.mouseButton.button == render::Mouse::Left) {
        render::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        if (containsPoint(_replayButton->getGlobalBounds(), mousePos)) {
            return MenuAction::REPLAY;
        } else if (containsPoint(_quitButton->getGlobalBounds(), mousePos)) {
            return MenuAction::QUIT;
        }
    } else if (event.type == render::EventType::MouseMoved) {
        render::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        if (containsPoint(_replayButton->getGlobalBounds(), mousePos)) {
            if (_selectedButton != 0) {
                _selectedButton = 0;
                updateButtonColors();
            }
        } else if (containsPoint(_quitButton->getGlobalBounds(), mousePos)) {
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

    _window.draw(*_overlay);
    _window.draw(*_victoryText);
    _window.draw(*_replayButton);
    _window.draw(*_replayText);
    _window.draw(*_quitButton);
    _window.draw(*_quitText);
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
        _replayButton->setFillColor(SELECTED_BUTTON_COLOR);
        _replayText->setFillColor(SELECTED_TEXT_COLOR);
        _quitButton->setFillColor(NORMAL_BUTTON_COLOR);
        _quitText->setFillColor(TEXT_COLOR);
    } else {
        _replayButton->setFillColor(NORMAL_BUTTON_COLOR);
        _replayText->setFillColor(TEXT_COLOR);
        _quitButton->setFillColor(SELECTED_BUTTON_COLOR);
        _quitText->setFillColor(SELECTED_TEXT_COLOR);
    }
}

void VictoryMenu::onWindowResize() {
    initializeMenu();
}