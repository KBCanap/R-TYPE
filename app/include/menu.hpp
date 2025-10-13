/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** menu
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include "audio_manager.hpp"
#include "components.hpp"
#include "key_bindings.hpp"
#include "options_menu.hpp"
#include "registery.hpp"
#include <memory>
#include <vector>

enum class MenuResult { None, Play, Options, Quit };

class Menu {
  public:
    Menu(registry &reg, render::IRenderWindow &win, AudioManager &audioMgr,
         KeyBindings &keyBindings);

    MenuResult run();      // Boucle du menu
    void update(float dt); // Update background et ennemis
    void render();         // Dessin de tout

  private:
    void createButtons();        // Création boutons et textes
    void createEnemies();        // Création ennemis
    void resetEnemies();         // Reset positions ennemis
    void updateButtonScale();    // Reposition et resize boutons/texte
    void updateEnemyPositions(); // Met à jour les positions des ennemis lors du
                                 // resize

  private:
    registry &_registry;
    render::IRenderWindow &_window;
    AudioManager &_audioManager;
    KeyBindings &_keyBindings;

    std::unique_ptr<render::ITexture> _bgTexture;
    std::unique_ptr<render::ISprite> _bgSprite1;
    std::unique_ptr<render::ISprite> _bgSprite2;
    render::Vector2u _windowSize;
    render::Vector2u _baseWindowSize;

    float _bgScrollSpeed = 100.f;

    std::unique_ptr<render::IFont> _font;
    std::vector<std::unique_ptr<render::IShape>> _buttons;
    std::vector<std::unique_ptr<render::IText>> _buttonTexts;

    std::vector<render::Vector2f> _enemyStartPositions;
    std::vector<entity> _enemies;
};