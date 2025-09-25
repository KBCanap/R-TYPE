#pragma once
#include <SFML/Graphics.hpp>
#include "registery.hpp"
#include "components.hpp"
#include "audio_manager.hpp"
#include "options_menu.hpp"

enum class MenuResult { None, Play, Options, Quit };

class Menu {
public:
    Menu(registry& reg, sf::RenderWindow& win, AudioManager& audioMgr);

    MenuResult run();                  // Boucle du menu
    void update(float dt);             // Update background et ennemis
    void render();                     // Dessin de tout

private:
    void createButtons();              // Création boutons et textes
    void createEnemies();              // Création ennemis
    void resetEnemies();               // Reset positions ennemis
    void updateButtonScale();          // Reposition et resize boutons/texte
    void updateEnemyPositions();       // Met à jour les positions des ennemis lors du resize

private:
    registry& _registry;
    sf::RenderWindow& _window;
    AudioManager& _audioManager;

    sf::Texture _bgTexture;
    sf::Sprite _bgSprite1;
    sf::Sprite _bgSprite2;
    sf::Vector2u _windowSize;
    sf::Vector2u _baseWindowSize;

    float _bgScrollSpeed = 100.f;

    sf::Font _font;
    std::vector<sf::RectangleShape> _buttons;
    std::vector<sf::Text> _buttonTexts;

    std::vector<sf::Vector2f> _enemyStartPositions;
    std::vector<entity> _enemies;
};