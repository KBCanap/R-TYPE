/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_main - Mario Bros prototype test
*/

#include "audio_manager.hpp"
#include "mario_game.hpp"
#include "registery.hpp"
#include "render/RenderFactory.hpp"
#include <iostream>

int main() {
    std::cout << "[Mario] Starting Mario Bros prototype..." << std::endl;

    // Create render and audio systems
    auto window = render::RenderFactory::createWindow(
        render::RenderBackend::SFML, 800, 600, "Mario Bros Prototype");

    auto audioSystem =
        render::RenderFactory::createAudio(render::RenderBackend::SFML);

    registry reg;
    AudioManager audioManager(*audioSystem);

    // Create and run the Mario game
    MarioGame marioGame(reg, *window, audioManager);
    marioGame.run();

    window->close();
    std::cout << "[Mario] Game ended" << std::endl;

    return 0;
}
