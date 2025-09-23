#include "../include/menu.hpp"
#include "../include/game.hpp"
#include "registery.hpp"
#include "components.hpp"
#include "../include/audio_manager.hpp"
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "BS_R-TYPE");
    registry reg;
    AudioManager audioManager;

    // Enregistre les composants
    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();
    reg.register_component<component::controllable>();

    // Lance le menu
    Menu menu(reg, window, audioManager);
    MenuResult result = menu.run();

    if (result == MenuResult::Play) {
        // Lance le jeu
        Game game(reg, window, audioManager);
        game.run();
    }
    else if (result == MenuResult::Quit) {
        window.close();
    }

    return 0;
}
